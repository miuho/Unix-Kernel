/** @file kern/vanish.c
 *
 *  @brief vanish syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "vanish";

void vanish_handler() {
    report_progress(tag, "entry, going to vanish %p", running_ktcb);

    tcb_t *tcb = running_ktcb->tcb;
    pcb_t *pcb = tcb->pcb;
    void *pgd = (void *)pcb->pgd;

    int process_exited;
    
    /* clean up memory if all threads exited */
    if ((process_exited = 
            (pcb->exited_thread_count == (ht_size(pcb->tcb_ht) - 1)))) {

        /* use kern pgd since going to free its own pgd */
        pcb->pgd = (unsigned long)kern_pgd;
        set_cr3((unsigned long)kern_pgd);

        report_progress(tag, "going to cleanup pgd");
        pgd_process_cleanup(pgd);
   
        mutex_lock(&(pcb->children->mp));
        ht_traverse_all(pcb->children, announce_parent_death);
        mutex_unlock(&(pcb->children->mp));
    }

    int if_was_set = if_disable();
    
    /* change status to exited */
    tcb->state = TCB_EXITED;

    xadd(&(pcb->exited_thread_count), 1);
    
    /* free the ktcb so other threads can use it */
    kthr_free(running_ktcb);

   if (process_exited) { 
        if (pcb->parent != NULL) {
            report_progress(tag, "pcb %p signaling parent %p, exit", 
                            pcb, pcb->parent);
            report_progress(tag, "signal parent %d", pcb->parent->pid);
            cond_signal_terminate(&(pcb->parent->wait_cond));
        }
   }

    /* context switch to kernel and never switch back again */
    report_progress(tag, "pcb no signal parent, going to switch, exit");

    cs_save_and_switch(NULL, sched_next());

    /* never reach here */
    if_recover(if_was_set);
}
