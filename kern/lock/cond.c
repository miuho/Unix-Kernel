/** @file cond.c
 *  @brief This file implements the condition variable.
 *
 *  @author Hingon Miu (hmiu)
 *  @author An Wu (anwu)
 */

#include <syscall.h>
#include <cond.h>
#include <stddef.h>
#include <st_queue.h>
#include <simics.h>
#include <kthread_pool.h>
#include <sched.h>
#include <loader.h>
#include <context_switch.h>
#include <reporter.h>
#include <x86/asm.h>
#include <if_flag.h>

static char *tag = "cond";

/*
 * @brief This should initialize the condition variable pointed to
 *        by cv.
 *
 * @param cv The condition variable to be initialized.
 *
 * @return 0 on success, -1 on error.
 */
int cond_init(cond_t *cv)
{
    if (cv == NULL) {
        report_error(tag, "cond_init: trying to init a NULL cond");
        return -1;
    }
    /* init the fields */
    cv->wait_ktcbs = st_queue_new();
    if (cv->wait_ktcbs == NULL) {
        report_error(tag, "cond_init: can't alloc new st_queue");
        return -1;
    }
    
    cv->init = 1;
    return 0;
}

/*
 * @brief This should deactivate the condition variable pointed to
 *        by cv.
 *
 * @param cv The condition variable to be deactivated.
 *
 * @return Void.
 */
void cond_destroy(cond_t *cv)
{
    if (cv == NULL) {
        report_error(tag, "trying to destroy a NULL cond");
        return;
    }

    if (cv->init == 0) {
        /* the condition variable was already destroyed */
        report_error(tag, "cond already destroyed");
        return;
    }
    
    cv->init = 0;
    
    int set = if_disable();
    if (!st_queue_empty(cv->wait_ktcbs)) {
        if_recover(set);
        /* there are blocked thread waiting */
        report_error(tag, 
            "trying to destroy cond when there's thr waiting");
        return;
    }
    

    st_queue_destroy(cv->wait_ktcbs);
    if_recover(set);

    return;
}

/*
 * @brief This should allows a thread to wait for a condition
 *        and release the associated mutex that it needs to 
 *        hold to check that condition.
 *
 * @param cv The condition variable to wait on.
 * @param mp The mutex to be released.
 *
 * @return Void.
 */
void cond_wait(cond_t *cv, mutex_t *mp)
{
    /* check arg */
    if (cv == NULL || mp == NULL) {
        report_error(tag, "cond_wait: arg NULL");
        return;
    }

    /* atomically unlock and switch */
    int if_was_set = if_disable();

    if (running_ktcb == sched_ktcb) {
        if (st_queue_empty(cv->wait_ktcbs)) {
            st_enqueue(&(sched_ktcb->c_n), (void *)sched_ktcb, cv->wait_ktcbs);
        }
    }
    else {
        st_enqueue(&(running_ktcb->c_n), (void *)running_ktcb, cv->wait_ktcbs);
    }

    mutex_cond_unlock(mp);

    if_recover(if_was_set);

    /* when deschedule returns, lock the mutex again */
    mutex_lock(mp);

    return;
}

/*
 * @brief This should wake up a thread waiting on the condition
 *        variable.
 *
 * @param cv The condition variable to wake up.
 *
 * @return Void.
 */
void cond_signal(cond_t *cv)
{
    if (cv == NULL) {
        report_error(tag, "cond_signal: arg NULL");
        return;
    }
    
    int if_was_set = if_disable();
    
    ktcb_t *to_run = (ktcb_t *)st_dequeue(cv->wait_ktcbs);
    
    if (to_run == NULL) {
        /* empty st_queue */
        if_recover(if_was_set);
        report_warning(tag, 
            "cond_signal has no effect since its queue is empty");
        return;
    }
    
    if (running_ktcb != sched_ktcb) {
        if (sched_running_to_runnable(running_ktcb) == -1) {
            if_recover(if_was_set);
            report_error(tag, "running ktcb is in runnable queue");
            return;
        }
    }
    else {
        if (st_queue_empty(sched_ktcb->tcb->pcb->wait_cond.wait_ktcbs)) {
            st_enqueue(&(sched_ktcb->c_n), sched_ktcb, 
                        sched_ktcb->tcb->pcb->wait_cond.wait_ktcbs);
        }
    }
    
    cs_save_and_switch(running_ktcb, to_run);
    
    if_recover(if_was_set);
}

/*
 * @brief This should wake up a thread waiting on the condition
 *        variable (and never goes back)
 *
 * @param cv The condition variable to wake up.
 *
 * @return Void.
 */
void cond_signal_terminate(cond_t *cv)
{
    if (cv == NULL) {
        report_error(tag, "cond_signal: arg NULL");
        return;
    }

    int if_was_set = if_disable();

    ktcb_t *to_run = (ktcb_t *)st_dequeue(cv->wait_ktcbs);
    
    if (to_run == NULL) {
        /* empty st_queue */
        report_warning(tag, 
            "cond_signal has no effect since its queue is empty");
        
        if_recover(if_was_set);
        return;
    }
    
    if (running_ktcb == sched_ktcb) {
        report_error(tag, "trying to signal_terminate sched_ktcb, exit");

        if_recover(if_was_set);
        return;
    }
    
    cs_save_and_switch(NULL, to_run);

    /* never reach here */
    if_recover(if_was_set);
}

/*
 * @brief This should wake up all threads waiting on the condition
 *        variable.
 *
 * @param cv The condition variable to wake up.
 *
 * @return Void.
 */
void cond_broadcast(cond_t *cv)
{
    /* lock the cond queue and make every waiting tid runnable */
    st_queue wait_q = cv->wait_ktcbs;
    
    while (!st_queue_empty(wait_q)) {
        int if_was_set = if_disable();
        ktcb_t *ktcb = (ktcb_t *)st_dequeue(wait_q);
        if_recover(if_was_set);
        
        if (sched_running_to_runnable(ktcb) == -1) {
            report_error(tag, "ktcb is already in runnable queue");
            return;
        }
    }
}
