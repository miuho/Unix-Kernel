/** @file kern/fork.c
 *
 *  @brief fork syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall.h>

#include <common_include.h>

static char *tag = "fork";

void fork_handler() {
    report_progress(tag, "entry");

    /* get the running process */
    ktcb_t *ktcb = running_ktcb;
    tcb_t *tcb = ktcb->tcb;
    pcb_t *pcb = tcb->pcb;
    unsigned long ebp = get_ebp();

    /* check that the process only has one thread */
    mutex_lock(&(pcb->tcb_ht->mp));
    if (ht_size(pcb->tcb_ht) > 1) {
        mutex_unlock(&(pcb->tcb_ht->mp));
        report_error(tag, "can't fork from process with >1 threads, exit");

        *(int *)(ebp + 8) = -1;
        return;
    }
    mutex_unlock(&(pcb->tcb_ht->mp));

    /* allocate structures */
    reg_t *new_regs = reg_copy(tcb->regs);
    if (new_regs == NULL) {
        report_error(tag, "fork: can't copy new reg, exit");

        *(int *)(ebp + 8) = -1;
        return;
    }

    /* allocate new pgd */
    unsigned long pgd = pcb->pgd;
    unsigned long new_pgd = (unsigned long)pgd_alloc();
    if (new_pgd == 0) {
        report_error(tag, "can't alloc new pgd, exit");

        free(new_regs);
        *(int *)(ebp + 8) = -1;
        return;
    }

    /* COW pages */
    if (vm_ref_copy((void *)pgd, (void *)new_pgd, 1) != 0) {
        report_error(tag, "can't copy new pgd, exit");

        vm_ref_copy_rollback((void *)new_pgd);
        pgd_free((void *)new_pgd);
        free(new_regs);
        *(int *)(ebp + 8) = -1;
        return;
    }

    /* allocate new ktcb */
    ktcb_t *new_ktcb;
    if ((new_ktcb = kthr_alloc()) == NULL) {
        report_error(tag, "can't alloc new ktcb, exit");

        pgd_cleanup((void *)new_pgd);
        pgd_free((void *)new_pgd);
        free(new_regs);
        *(int *)(ebp + 8) = -1;
        return;
    }

    /* allocate new pcb */
    pcb_t *new_pcb = pcb_create(new_pgd, new_regs, pcb, new_ktcb,
                                pcb->txt_base,
                                pcb->txt_len, pcb->rodat_base, 
                                pcb->rodat_len);
    if (new_pcb == NULL) {
        report_error(tag, "fork: can't create new pcb, exit");

        kthr_free(new_ktcb);
        pgd_cleanup((void *)new_pgd);
        pgd_free((void *)new_pgd);
        free(new_regs);
        *(int *)(ebp + 8) = -1;
        return;
    }


    report_progress(tag, "setting up child...");
    /* set up new ktcb stack */
    unsigned long copy_len = ktcb->regs->esp0 - ebp + 4;
    memcpy((void *)(new_ktcb->regs->esp0 - copy_len),
           (void *)(ktcb->regs->esp0 - copy_len), copy_len);

    /* set up new ktcb registers */
    new_ktcb->regs->ebp = new_ktcb->regs->esp0 - copy_len + 4;

    /* child return 0 */
    *(int *)(new_ktcb->regs->ebp + 8) = 0;

    /* make child relation and schedule child */
    report_progress(tag, "make new process %d child of process %d", 
                    new_pcb->pid,  pcb->pid);

    mutex_lock(&(pcb->children->mp));
    ht_insert(pcb->children, new_pcb->pid, (void *)new_pcb);
    mutex_unlock(&(pcb->children->mp));

    int set = if_disable();
    
    if (sched_running_to_runnable(running_ktcb) != 0)
        report_error(tag, "cannot add to runnable queue");

    /* parent return child's pid */
    *(int *)(ebp + 8) = new_pcb->pid;

    cs_save_and_switch(running_ktcb, new_ktcb);

    report_progress(tag, "child ktcb is %p, exit", new_ktcb);

    if_recover(set);
}

