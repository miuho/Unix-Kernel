/** @file mutex.c
 *  @brief This file implements the mutex lock/unlock to ensure concurrent
 *         threads run without race conditions.
 *
 *  @author Hingon Miu (hmiu) 
 *  @author An Wu (anwu) 
 */

#include <mutex_type.h>
#include <stddef.h>
#include <stdio.h>
#include "asm.h"
#include <assert.h>
#include <reporter.h>
#include <loader.h>
#include <x86/asm.h>
#include <sched.h>
#include <context_switch.h>
#include <if_flag.h>

static char *tag = "mutex";

/*
 * @brief This should initialize the mutex pointed to by mp.
 *
 *
 * @param mp The mutex to be initialized.
 * @return 0 on success, -1 on error.
 *
 */
int mutex_init(mutex_t *mp)
{
    if (mp == NULL) {
        /* the given mutex struct pointer is invalid */
        report_progress(tag, "the given mutex struct pointer is invalid");
        return -1;
    }

    else {
        mp->available = 1;
        mp->holder = NULL;
         
        mp->queue = st_queue_new();
        if (mp->queue == NULL) {
            report_progress(tag, "mutex_init fail to allocate mutex_queue");
            return -1;
        }
        
        mp->init = 1;
        return 0;
    }
}

/*
 * @brief This should deactivate the mutex pointed to by mp.
 *
 *
 * @param mp The mutex to be deactivated.
 * @return Void.
 *
 */
void mutex_destroy(mutex_t *mp)
{
    if (mp == NULL) {
        /* the given mutex struct pointer is invalid */
        report_error(tag, "mutex_destroy: get NULL mutex");
        return;
    }

    else if (!mp->init) {
        /* using a mutex before its initialized is undefined */
        report_error(tag, "mutex_destroy: Attempt to destroy a uninit mp");
        return;
    }

    else if (!mp->available) {
        /* illegal to destroyed a mutex in use */
        report_error(tag, "mutex_destroy: Attempt to destroy a locked mutex.");
        return;
    }

    else if (!st_queue_empty(mp->queue)) {
        /* other threads are trying to acquire this mutex */
        report_error(tag, 
            "mutex_destroy: Destroy mutex whn others are waiting for it");
        return;
    }

    else {
        mp->init = 0;
        mp->holder = NULL;
        st_queue_destroy(mp->queue);

        return;
    }
}

/*
 * @brief This should ensures mutual exclusion in the region 
 *        between itself and a call to mutex_unlock().
 *
 * @param mp The mutex to be locked.
 * @return Void.
 */
void mutex_lock(mutex_t *mp)
{
    if (mp == NULL) {
        report_error(tag, "mutex_lock: input mutex is NULL");
        /* the given mutex struct pointer is invalid, do nothing */
        return;
    }

    else if (!mp->init) {
        report_error(tag, "mutex_lock: mutex hasn't been init yet");
        /* using a mutex before its initialized is undefined */
        return;
    }

    else {
        int if_was_set = if_disable();
        
        if (!(mp->available)) {
            
            running_ktcb->blocked_mutex = mp;
            
            st_enqueue(&(running_ktcb->m_n),(void *)running_ktcb, mp->queue);
            cs_save_and_switch(running_ktcb, sched_next());

            running_ktcb->blocked_mutex = NULL;
            mp->holder = running_ktcb;
        }
        else {
            mp->available = 0;
            mp->holder = running_ktcb;
        }

        if_recover(if_was_set);
        return;
    }
}

/** @brief Guarentee to switch to another thread, that 
 *         performs atomic unlock when called by cond_wait().
 *
 *  @param mp The mutex to be unlocked.
 *  @return Void.
 */
void mutex_cond_unlock(mutex_t *mp)
{
    if (mp == NULL) {
        report_error(tag, "mutex_cond_unlock: mutex is NULL");
        /* the given mutex struct pointer is invalid, do nothing */
        return;
    }

    else if (!mp->init) {
        report_error(tag, "mutex_cond_unlock: not init");
        /* using a mutex before its initialized is undefined */
        return;
    }

    else {
        int if_was_set = if_disable();
        
        ktcb_t *to_run = st_dequeue(mp->queue);

        if (to_run != NULL) {
            
            to_run->blocked_mutex = NULL;
            mp->holder = to_run;
            cs_save_and_switch(running_ktcb, to_run);
        }
        
        else {
            mp->available = 1;
            mp->holder = NULL;

            cs_save_and_switch(running_ktcb, sched_next());
        }

        if_recover(if_was_set);
        return;
    }
}

/** @brief This should signals the end of a region of mutual exclusion.
 *
 *  @param mp The mutex to be unlocked.
 *  @return Void.
 */
void mutex_unlock(mutex_t *mp)
{
    if (mp == NULL) {
        report_error(tag, "mutex_unlock: mutex is NULL");
        /* the given mutex struct pointer is invalid, do nothing */
        return;
    }

    else if (!mp->init) {
        report_error(tag, "mutex_unlock: not init");
        /* using a mutex before its initialized is undefined */
        return;
    }

    else {
        int if_was_set = if_disable();
        
        ktcb_t *to_run = st_dequeue(mp->queue);

        if (to_run != NULL) {
            
            if (running_ktcb != sched_ktcb) {
                if (sched_running_to_runnable(running_ktcb) == -1) {
                    if_recover(if_was_set);
                    report_error(tag, 
                        "running ktcb is in runnable queue");
                    return;
                }
            }
            
            to_run->blocked_mutex = NULL;
            mp->holder = to_run;
            cs_save_and_switch(running_ktcb, to_run);
        }
        else {
            mp->available = 1;
            mp->holder = NULL;
        }

        if_recover(if_was_set);
        return;
    }
}
