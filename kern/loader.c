/**
 * The 15-410 kernel project.
 * @name loader.c
 *
 * Functions for the loading
 * of user programs from binary 
 * files should be written in
 * this file. The function 
 * elf_load_helper() is provided
 * for your use.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

/* --- Includes --- */
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <exec2obj.h>
#include <loader.h>
#include <elf_410.h>
#include <vm.h>
#include <pgtable.h>
#include <cr.h>
#include <eflags.h>
#include <seg.h>
#include <pcb.h>
#include <mode_switch.h>
#include <x86/asm.h>
#include <reporter.h>

static char *tag = "loader";

/* --- Local function prototypes --- */ 


/**
 * Copies data from a file into a buffer.
 *
 * @param filename   the name of the file to copy data from
 * @param offset     the location in the file to begin copying from
 * @param size       the number of bytes to be copied
 * @param buf        the buffer to copy the data into
 *
 * @return returns the number of bytes copied on succes; -1 on failure
 */
int getbytes( const char *filename, int offset, int size, char *buf )
{
    int i;
    for (i = 0; i < exec2obj_userapp_count; i++) {
        if (strcmp(filename, exec2obj_userapp_TOC[i].execname) == 0)
            break;
    }

    if (i == exec2obj_userapp_count)
        /* can't find filename in table of contents */
        return -1;

    int execlen = exec2obj_userapp_TOC[i].execlen;
    int length = execlen - offset;

    char *ret = memcpy(buf, exec2obj_userapp_TOC[i].execbytes + offset,
                       length > size ? size : length);

    if (ret != buf)
        /* memcpy failed */
        return -1;

    return length > size ? size : length;
}

ktcb_t *register_ktcb(tcb_t *tcb) {
    /* allocate a new ktcb */
    ktcb_t *ktcb = kthr_alloc();

    if (ktcb == NULL) {
        report_error(tag, "allocate kernel thread failed");
        return NULL;
    }

    ktcb->tcb = tcb;
    tcb->ktcb = ktcb;
    return ktcb;
}

int count_argv(char **argvec)
{
    if (argvec == NULL) { 
        /* the first progrma we loaded */
        return 1;
    }

    if (*argvec == NULL) { 
        /* the first string should be the filename, by convention */
        report_error(tag, "argven length is 0");
        return -1;
    }

    /* start counting from the second string */
    int count = 1;
    while (argvec[count] != NULL) {
        count++;
    }

    return count;
}

int copy_argv(char **array_addr_start, char **argvec, int argc, char *filename)
{
    /* the start of argvec string contents */
    char *array_content_start = (char *)(array_addr_start + (argc + 1));

    /* copy file name */
    strcpy(array_content_start, filename);
    *array_addr_start = array_content_start;
    array_addr_start++;
    array_content_start += strlen(filename) + 1;

    /* copy the rest argvec */
    int i;
    for (i = 1; i < argc; i++) {
        /* copy the ith string */
        strcpy(array_content_start, argvec[i]);
        *array_addr_start = array_content_start;;
        array_addr_start++;
        array_content_start += strlen(argvec[i]) + 4;
    }
    *array_addr_start = NULL;

    return 0;
}

char **save_argvec(char *filename, char **argvec)
{
    int count = count_argv(argvec);

    char **array = calloc(count + 1, sizeof(char *));
    if (array == NULL) {
        report_error(tag, "save_argvec: array calloc gets NULL");
        return NULL;
    }
    
    char *tmp;
    if (argvec == NULL || *argvec == NULL) {
        /* first execution, only copy filename */
        tmp = calloc(strlen(filename) + 1, sizeof(char));
        if (tmp == NULL) {
            report_error(tag, "save_argvec: tmp calloc failed");
            free(array);
            return NULL;
        }

        strcpy(tmp, filename);
        array[0] = tmp;
    }
    else {
        int i;
        for (i = 0; i < count; i++) {
            tmp = calloc(strlen(argvec[i]) + 1, sizeof(char));
            if (tmp == NULL) {
                report_error(tag, "save_argvec: tmp calloc gets NULL");

                /* roll back */
                for (i = i - 1; i >= 0; i--) {
                    free(array[i]);
                }
                free(array);

                return NULL;
            }
            strcpy(tmp, argvec[i]);
            array[i] = tmp;
        }
    }
    array[count] = NULL;
    
    return array;
}

void free_argvec(char **argvec) {

    if (argvec == NULL) {
        report_error(tag, "free_argvec: trying to free NULL argvec");
        return;
    }

    char *arg = *argvec;
    int i = 0;
    while (arg != NULL) {
        free(arg);
        arg = argvec[++i];
    }

    free(argvec);
}

pcb_t *load_prog(char *filename, char **argvec, pcb_t *pcb, tid_t tid, 
                ktcb_t *ktcb) 
{
    report_progress(tag, "load_entry: entry");

    void *pgd = pgd_alloc();
    if (pgd == NULL) {
        report_error(tag, "load_prog: pgd_alloc failed");
        return NULL;
    }

    /* copy kernel pgd into prog pgd */
    memcpy(pgd, kern_pgd, PAGE_SIZE);

    void *orig_pgd = (pcb == NULL) ? kern_pgd : (void *)(pcb->pgd);

    simple_elf_t elf_header;
    /* fill in elf_header */
    if (elf_load_helper(&elf_header, filename) != ELF_SUCCESS) {
        report_warning(tag, "file is not in elf format.");
        return NULL;
    }
    
    /* save user argvec to kernel */
    report_progress(tag, "load_prog: saving argvec to kernel");
    char **saved_argvec = save_argvec(filename, argvec);
    if (saved_argvec == NULL) {
        report_error(tag, "load_prog: save_argvec failed");
        
        pgd_free(pgd);
        return NULL;
    }

    /* copy inputs to user stack */
    /* alloc three pages for stack and put esp in the middle */
    report_progress(tag, "load_prog: trying to allocate stack space...");
    int temp = pgd_alloc_pages(pgd, (void *)USER_STACK_BASE - PAGE_SIZE, 
                                2 * PAGE_SIZE,
                                PG_PRESENT | PG_WRITABLE | PG_USER,
                                PG_PRESENT | PG_WRITABLE | PG_USER);
    if (temp != 0) {
        report_error(tag, "can't alloc stack space for program");

        free_argvec(saved_argvec);
        pgd_free(pgd);
        return NULL;
    }

    int argc = count_argv(saved_argvec);

    /* !!! Set cr3 to new pgd and start loading program !!! */
    if (pcb != NULL) {
        pcb->pgd = (unsigned long)pgd;
    }
    set_cr3((unsigned long)pgd);

    /* save argvec into user stack */
    /* string pointer array starts at USER_STACK_BASE + 4 + 4 * 4 */
    report_progress(tag, "load_prog: saving argvec into user stack...");
    char **argvec_start = (char **)(USER_STACK_BASE + 20);
    copy_argv(argvec_start, saved_argvec, argc, saved_argvec[0]);

    /* prepare inputs for main wrapper */
    *((int *)(USER_STACK_BASE + 4)) = argc;
    *((char ***)(USER_STACK_BASE + 8)) = argvec_start;
    *((void **)(USER_STACK_BASE + 12)) = (void *)USER_STACK_BASE + PAGE_SIZE;
    *((void **)(USER_STACK_BASE + 16)) = (void *)USER_STACK_BASE - PAGE_SIZE;

    report_progress(tag, "load_prog: loading program binary...");

    /* load bss */
    if (elf_header.e_bsslen > 0) {

        temp = pgd_alloc_pages(pgd, (void *)elf_header.e_bssstart, 
                            (int)elf_header.e_bsslen,
                            PG_PRESENT | PG_WRITABLE | PG_USER,
                            PG_PRESENT | PG_WRITABLE | PG_USER);
        if (temp != 0) {
            report_error(tag, "load_prog: load bss section failed");

            if (pcb != NULL) {
                pcb->pgd = (unsigned long)orig_pgd;
            }
            set_cr3((unsigned long)orig_pgd);
            free_argvec(saved_argvec);
            pgd_free(pgd);
            return NULL;
        }

        memset((void *)elf_header.e_bssstart, 0, elf_header.e_bsslen);
    }

    /* load data */ 
    if (elf_header.e_datlen > 0) {

        temp = pgd_alloc_pages(pgd, (void *)elf_header.e_datstart, 
                            (int)elf_header.e_datlen, 
                            PG_PRESENT | PG_WRITABLE | PG_USER,
                            PG_PRESENT | PG_WRITABLE | PG_USER);
        if (temp != 0) {
            report_error(tag, "load_prog: load data section failed");

            if (pcb != NULL) {
                pcb->pgd = (unsigned long)orig_pgd;
            }
            set_cr3((unsigned long)orig_pgd);
            free_argvec(saved_argvec);
            pgd_free(pgd);
            return NULL;
        }

        getbytes(saved_argvec[0], elf_header.e_datoff, elf_header.e_datlen, 
                    (void *)elf_header.e_datstart);
    }

    /* load text */
    if (elf_header.e_txtlen > 0) {

        temp = pgd_alloc_pages(pgd, (void *)elf_header.e_txtstart, 
                            (int)elf_header.e_txtlen, 
                            PG_PRESENT | PG_USER,
                            PG_PRESENT | PG_USER);
        if (temp != 0) {
            report_error(tag, "load_prog: load text section failed");

            if (pcb != NULL) {
                pcb->pgd = (unsigned long)orig_pgd;
            }
            set_cr3((unsigned long)orig_pgd);
            free_argvec(saved_argvec);
            pgd_free(pgd);
            return NULL;
        }

        getbytes(saved_argvec[0], elf_header.e_txtoff, elf_header.e_txtlen, 
                    (void *)elf_header.e_txtstart);
    }
   
    /* load read-only data */
    if (elf_header.e_rodatlen > 0) {

        temp = pgd_alloc_pages(pgd, (void *)elf_header.e_rodatstart, 
                            (int)elf_header.e_rodatlen,
                            PG_PRESENT | PG_USER,
                            PG_PRESENT | PG_USER);
        if (temp != 0) {
            report_error(tag, "load_prog: load rodata section failed");

            if (pcb != NULL) {
                pcb->pgd = (unsigned long)orig_pgd;
            }
            set_cr3((unsigned long)orig_pgd);
            free_argvec(saved_argvec);
            pgd_free(pgd);
            return NULL;
        }

        getbytes(saved_argvec[0], elf_header.e_rodatoff, elf_header.e_rodatlen, 
                    (void *)elf_header.e_rodatstart);
    }

    free_argvec(saved_argvec);

    report_progress(tag, "load_prog: setting up program structures");

    /* set up regs */
    reg_t *regs = calloc(1, sizeof(reg_t));
    if (regs == NULL) {
        report_error(tag, "load_prog: regs calloc failed");

        if (pcb != NULL) {
            pcb->pgd = (unsigned long)orig_pgd;
        }
        set_cr3((unsigned long)orig_pgd);
        pgd_free(pgd);
        return NULL;
    }

    regs->esp = USER_STACK_BASE;
    regs->eip = elf_header.e_entry;

    /* set up eflags for user process */
    uint32_t eflags = get_eflags();
    eflags |= EFL_RESV1;
    eflags &= ~EFL_AC;
    eflags |= EFL_IOPL_RING0;
    eflags |= EFL_IF;
    regs->eflags = eflags;

    pcb_t *new_pcb;
    if (pcb == NULL) {
        new_pcb = pcb_create((unsigned long)pgd, regs, NULL, ktcb,
                         (void *)elf_header.e_txtstart, elf_header.e_txtlen,
                         (void *)elf_header.e_rodatstart, 
                         elf_header.e_rodatlen);

        if (new_pcb == NULL) {
            report_error(tag, "load_prog: pcb_create failed");

            set_cr3((unsigned long)orig_pgd);
            free(regs);
            pgd_free(pgd);
            return NULL;
        }
    }
    else {
        new_pcb = pcb_update(pcb, (unsigned long)pgd, regs, tid, ktcb,
                            (void *)elf_header.e_txtstart, elf_header.e_txtlen,
                            (void *)elf_header.e_rodatstart, 
                            elf_header.e_rodatlen);
        if (new_pcb == NULL) {
            report_error(tag, "load_prog: pcb_update failed");            

            pcb->pgd = (unsigned long)orig_pgd;
            set_cr3((unsigned long)orig_pgd);
            free(regs);
            pgd_free(pgd);
            return NULL;
        }
    }

    if (orig_pgd != kern_pgd) {
        /* remove its COW refs first */
        pgd_cleanup(orig_pgd);
        pgd_free(orig_pgd);
    }

    report_progress(tag, "load_prog: loaded pcb %p with pgd %p, exit", new_pcb,
                    pgd);

    return new_pcb;
}

void setup_exec_stack(ktcb_t *ktcb) {

    tcb_t *tcb = ktcb->tcb;

    unsigned long esp0 = ktcb->regs->esp0;

    /* copy mode switch args */
    *(unsigned long *)(esp0 - 4) = SEGSEL_USER_DS;
    *(unsigned long *)(esp0 - 8) = tcb->regs->esp;
    *(unsigned long *)(esp0 - 12) = tcb->regs->eflags;
    *(unsigned long *)(esp0 - 16) = SEGSEL_USER_CS;
    *(unsigned long *)(esp0 - 20) = tcb->regs->eip;
    *(unsigned long *)(esp0 - 24) = SEGSEL_USER_DS;
    *(unsigned long *)(esp0 - 28) = SEGSEL_USER_DS;
    *(unsigned long *)(esp0 - 32) = SEGSEL_USER_DS;
    *(unsigned long *)(esp0 - 36) = SEGSEL_USER_DS;

    /* load "mode_switch" 's eip */
    *(unsigned long *)(esp0 - 44) = (unsigned long)mode_switch;

    /* put ebp under */
    ktcb->regs->ebp = esp0 - 48;
}
