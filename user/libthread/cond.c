/** @file cond.c
 *  @brief This file implements the condition variable.
 *
 *  @bug:  Fix the queue struct's data type to pointer address to data.
 *  @bug:  Fix atomically unlock mutex in wait
 */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall.h>
#include <cond.h>
#include <stddef.h>
#include <queue.h>
#include <simics.h>

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
        return -1;
    }
    /* init the fields */
    cv->init = 1;
    cv->wait_tids = queue_new();
    if (cv->wait_tids == NULL) {
        return -1;
    }
    /* init the lock */
    mutex_init(&(cv->lock));
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
    if (cv->init == 0) {
        /* the condition variable was destroyed */
        return;
    }

    else if (!queue_empty(cv->wait_tids)) {
        /* there are blocked thread waiting */
        return;
    }

    mutex_destroy(&(cv->lock));
    queue_destroy(cv->wait_tids);

    cv->init = 0;
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
    /* lock on cond queue and enqueue current thread */
    mutex_lock(&(cv->lock));
    int tid = gettid();
    enqueue((void *)tid, cv->wait_tids);
    mutex_unlock(&(cv->lock));

    /* unlock the mutex mp */
    mutex_unlock(mp);                       
    
    int zero = 0;
    deschedule(&(zero));

    /* when deschedule returns, lock the mutex again */
    mutex_lock(mp);
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
    /* lock on cond queue and dequeue the top waiting thread */
    mutex_lock(&(cv->lock));
    void *tid_data = dequeue(cv->wait_tids);
    mutex_unlock(&(cv->lock));
    
    if (tid_data == NULL) {
        /* empty queue */
        return;
    }

    int tid = (int)tid_data;
    while (make_runnable(tid) < 0) {
        // let it deschdule first
        yield(tid);
    }
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
    mutex_lock(&(cv->lock));
    queue wait_q = cv->wait_tids;
    
    while (!queue_empty(wait_q)) {
        int tid = (int) dequeue(wait_q);
        while (make_runnable(tid) < 0) {
            // let it deschedule first
            yield(tid);
        }
    }
    mutex_unlock(&(cv->lock)); /* unlock the cond queue */
}
