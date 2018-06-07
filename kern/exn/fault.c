/** @file kern/exn/fault.c
 *
 *  @brief handle the faults that specific fault handlers can't handle
 *         possibly kill the problem
 *
 * @author An Wu (anwu)
 * @author Hingon Miu (hmiu)
 */

#include <fault.h>
#include <mutex.h>

#include <common_include.h>

static char *tag = "fault";

void fault_handler(ureg_t *ureg) {
    report_progress(tag, "fault_handler: entry point");
    tcb_t *tcb = running_ktcb->tcb;
    pcb_t *pcb = tcb->pcb;
    void *pgd = (void *)pcb->pgd;

    if (tcb->swexn_eip != NULL) {
        report_progress(tag, "swexn registered");
        
        /* swexn registered for this thread. try to handle */
        void *eip = tcb->swexn_eip;
        void *esp3 = tcb->swexn_esp3;
        void *arg = tcb->swexn_arg;

        /* de-register the swexn */
        tcb->swexn_eip = NULL;
        tcb->swexn_esp3 = NULL;
        tcb->swexn_arg = NULL;

        /* put ureg_t at the start, then align it, then put
         * ureg_t pointer, arg and return address 
         */
        int len = sizeof(ureg_t) + 12;
        len += (4 - len % 4);

        report_progress(tag, "check swexn eip and esp3");
        if (vm_mem_check(pcb, (void *)pcb->pgd, eip) >= 0 &&
            vm_mem_region_check(pcb, (void *)pcb->pgd, esp3-len, len) == 1) {

            /* copy ureg */ 
            memcpy(esp3-len+12, ureg, sizeof(ureg_t));

            /* copy args */
            *(void **)(esp3-len+4) = arg;
            *(void **)(esp3-len+8) = esp3-len+12;

            /* return address */
            *(void **)(esp3-len) = NULL;
            
            report_progress(tag, 
                            "going to mode switch to swexn handler, exit");
            mode_switch(ureg->ds, ureg->fs, ureg->gs, ureg->es, 
                        (unsigned long)eip, ureg->cs, ureg->eflags, 
                        (unsigned long)esp3-len, ureg->ss);
        }
    }

    /* print fault info to console */
    mutex_lock(&print_mp);
    print_ureg(ureg);
    mutex_unlock(&print_mp);

    report_warning(tag, "going to kill");

    /* set exit status to FAULT_KILL_EXIT_STATUS */
    pcb->exit_status = FAULT_KILL_EXIT_STATUS;

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

    report_progress(tag, "going to switch, exit");

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
            report_error(tag, "signal parent %d", pcb->parent->pid);
            cond_signal_terminate(&(pcb->parent->wait_cond));
        }
    }

    /* context switch to kernel and never switch back again */
    cs_save_and_switch(NULL, sched_next());

    /* never reach here */
    if_recover(if_was_set);
}
