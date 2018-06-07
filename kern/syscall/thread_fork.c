/** @file kern/thread_fork.c
 *
 *  @brief thread_fork syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "thread_fork";

void thread_fork_handler() {

    report_progress(tag, "entry");

    /* get the running process */
    ktcb_t *ktcb = running_ktcb;
    tcb_t *tcb = ktcb->tcb;
    pcb_t *pcb = tcb->pcb;
    unsigned long ebp = get_ebp();

    reg_t *new_regs = reg_copy(tcb->regs);
    if (new_regs == NULL) {
        report_error(tag, "cannot make new reg, exit");
        *(int *)(ebp + 8) = -1;
        return;
    }

    ktcb_t *new_ktcb = kthr_alloc();
    if (new_ktcb == NULL) {
        report_error(tag, "cannot alloc new ktcb, exit");

        free(new_regs);
        *(int *)(ebp + 8) = -1;
        return;
    }

    /* generate new tcb */
    tid_t new_tid = generate_tid();
    tcb_t *new_tcb = tcb_create(pcb, pcb->tcb_ht, new_regs, new_tid, new_ktcb);
    if (new_tcb == NULL) {
        report_error(tag, "cannot create new tcb, exit");

        kthr_free(new_ktcb);
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
    
    int set = if_disable();

    /* child return 0 */
    *(int *)(new_ktcb->regs->ebp + 8) = 0;

    /* schedule child */
    if (sched_running_to_runnable(running_ktcb) != 0)
        report_error(tag, "cannot add to runnable queue");

    report_progress(tag, "child %p saved in runnable", new_ktcb);

    /* parent return child's pid */
    *(int *)(ebp + 8) = new_tid;
    
    cs_save_and_switch(running_ktcb, new_ktcb);

    if_recover(set);
    report_progress(tag, "exit");
}
