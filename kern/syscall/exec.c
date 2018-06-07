/** @file kern/exec.c
 *
 *  @brief exec syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "exec";

int exec_handler(void *args) {

    report_progress(tag, "entry");

    ktcb_t *ktcb = running_ktcb;
    pcb_t *pcb = ktcb->tcb->pcb;
    void *pgd = (void *)pcb->pgd;

    /* check that the process only has one thread */
    mutex_lock(&(pcb->tcb_ht->mp));
    if (ht_size(pcb->tcb_ht) > 1) {
        mutex_unlock(&(pcb->tcb_ht->mp));
        report_error(tag, "can't exec from process with >1 threads, exit");

        return -1;
    }
    mutex_unlock(&(pcb->tcb_ht->mp));

    /* check args */
    if (vm_mem_region_check(pcb, pgd, args, 8) < 0) {
        report_error(tag, "exec_handler: arguments not accessible, exit");
        return -1;
    }

    char *execname = *(char **)args;
    char **argvec = *(char ***)(args + 4);

    /* check execname array */
    if (vm_mem_array_check(pcb, pgd, 1, execname) < 0) {
        report_error(tag, "exec_handler: execname not accessible, exit");
        return -1;
    }

    /* check argvec array */
    if (vm_mem_array_check(pcb, pgd, 4, argvec) < 0) {
        report_error(tag, "exec_handler: argvec not accessible, exit");
        return -1;
    }

    /* check inside argvec */
    int idx = 0;
    char *arg = *argvec;
    while (arg != NULL) {
        if (vm_mem_array_check(pcb, pgd, 1, arg) < 0) {
            report_error(tag, 
                        "exec_handler: argvec elem not accessible, exit");

            return -1;
        }
        idx++;
        arg = *(argvec + idx);
    }

    /* check that argvec[0] is filename */
    if (strcmp(execname, argvec[0]) != 0) {
        report_error(tag, "wrong input, exit");

        return -1;
    }

    ktcb_t *new_ktcb;
    if ((new_ktcb = kthr_alloc()) == NULL) {
        report_error(tag, "kthr_alloc failed, exit");
        return -1;
    }

    /* do exec */
    if ((pcb = load_prog(execname, argvec, pcb, ktcb->tcb->tid, 
                        new_ktcb)) == NULL) {
        report_error(tag, "exec failed, file not loadable, exit");

        kthr_free(new_ktcb);
        return -1;
    }

    setup_exec_stack(new_ktcb);

    report_progress(tag, "going to switch to esp0 0x%x, exit", 
                    new_ktcb->regs->esp0);
    int if_was_set = if_disable();

    kthr_free(ktcb);
    cs_save_and_switch(NULL, new_ktcb);

    if_recover(if_was_set);

    return 0;
}
