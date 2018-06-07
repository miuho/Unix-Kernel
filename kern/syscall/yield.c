/**
 * @file kern/syscall/yield.c
 * @brief implements the yield handler.
 *
 * @author HingOn Miu (hmiu)
 * @author An Wu (anwu)
 *
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "yield";

int yield_handler(int tid) {

    report_progress(tag, "entry");

    ktcb_t *to_run;
    int if_was_set = if_disable();

    if ((to_run = sched_runnable_to_running(tid)) == NULL) {

        /* tid not runnable */
        ktcb_t *target;
        if ((target = sched_is_waiting(tid)) != NULL) {
            /* tid is waiting already, add it back and return */
            if_recover(if_was_set);

            report_warning(tag, "thread %d already waiting, exit", tid);
            return -1;
        }

        /* check if tid is holding a lock */
        /* tid is blocking on a lock */
        tcb_t *target_tcb = get_root_thr(sched_find_pcb(tid));

        if (target_tcb == NULL) {
            /* passed tid is not a pid, return */
            if_recover(if_was_set);

            report_error(tag, "passed tid is not pid (can't find pcb)");
            return -1;
        }

        mutex_t *blocked_mutex = target_tcb->ktcb->blocked_mutex;
        if (blocked_mutex == NULL) {
            /* it's not blocking. Really? */
            if_recover(if_was_set);
            report_warning(tag, "mutex is not blocking while not in sched");
            return -1;
        }

        /* get the current mutex that blocks the target ktcb */
        mutex_t *cur_blocked_mutex = blocked_mutex;
        /* the holder of current mutex by which target_tcb is blocked */
        ktcb_t *cur_holder = cur_blocked_mutex->holder;
        while (cur_holder != NULL) {
            /* find the mutex that blocks the holder */
            cur_blocked_mutex = cur_holder->blocked_mutex;
            
            if (cur_blocked_mutex == NULL) {
                /* found a holder of mutex that is not blocked by 
                 * another mutex */
                break;
            }
            
            /* the holder is found blocked by another mutex, then find */
            /* the holder of that mutex */
            cur_holder = cur_blocked_mutex->holder;
        }
        
        /* found a mutex that blocks a ktcb but not having a holder */
        /* get the holder of blocked mutex and switch to it */
        if ((to_run = cur_holder) == NULL) {
            /* can't find the holder of the lock that
             * thread with tid is waiting on. Really? */
            if_recover(if_was_set);
            
            report_error(tag, 
                "found a wrong mutex that blocks without a holder");
            return -1;
        }
        
        if (sched_runnable_to_running(to_run->tcb->tid) == NULL) {
            if_recover(if_was_set);

            report_error(tag, 
            "cannot retrieve ultimate mutex holder tid=%d from runnable queue",
                to_run->tcb->tid);
            return -1;
        }

        report_progress(tag, "going to switch to holder");
    }

    /* put the running ktcb in runnable queue */
    if (sched_running_to_runnable(running_ktcb) == -1) {
        /* running ktcb is already in runnable queue */
        if_recover(if_was_set);

        report_error(tag, "cannot put running_ktcb in runnable queue, exit");
        return -1;
    }

    report_progress(tag, "going to switch, exit");
    cs_save_and_switch(running_ktcb, to_run);
    
    if_recover(if_was_set);
    
    return 0;
}
