/** @file kern/deschedule.c
 *
 *  @brief deschedule syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "deschedule";

int deschedule_handler(int *reject) {
    report_progress(tag, "entry");

    pcb_t *pcb = running_ktcb->tcb->pcb;
    void *pgd = (void *)pcb->pgd;

    if (vm_mem_region_check(pcb, pgd, reject, 4) < 0) {
        lprintf("deschedule_handler: reject not accessible, exit");
        return -1;
    }

    int if_was_set = if_disable();

    if (*reject != 0) {
        if_recover(if_was_set);

        report_progress(tag, "reject != 0, exit");
        return 0;
    }

    else {
        report_progress(tag, "deschedule switch to another thread, exit");
        /* calling thread is suspended until make runnable is called */
        if (sched_running_to_waiting(running_ktcb) == -1) {
            if_recover(if_was_set);

            /* running ktcb is in waiting ht */
            report_error(tag, "cannot put running ktcb to waiting, exit");
            return -1;
        }

        /* context switch to sched ktcb */
        cs_save_and_switch(running_ktcb, sched_next());
        if_recover(if_was_set);
        report_progress(tag, "deschedule returns, exit");

        return 0;
    }
}
