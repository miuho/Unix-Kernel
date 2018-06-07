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
        return -1;
    }

    else {
        mp->init = 1;
        mp->available = 1;

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
        return;
    }

    else if (!mp->init) {
        /* using a mutex before its initialized is undefined */
        panic("Attempt to destroy a unintialized mutex.");
    }

    else if (!mp->available) {
        /* illegal to destroyed a mutex in use */
        panic("Attempt to destroy a locked mutex.");
    }

    else if (mp->threads_count) {
        /* other threads are trying to acquire this mutex */
        panic("Attempt to destroy a mutex while others are acquiring it.");
    }

    else {
        mp->init = 0;

        return;
    }
}

/*
 * @brief This should ensures mutual exclusion in the region 
 *        between itself and a call to mutex_unlock().
 *
 *
 * @param mp The mutex to be locked.
 * @return Void.
 *
 */
void mutex_lock(mutex_t *mp)
{
    if (mp == NULL) {
        /* the given mutex struct pointer is invalid, do nothing */
        return;
    }

    else if (!mp->init) {
        /* using a mutex before its initialized is undefined */
        return;
    }

    else {
        xadd(&mp->threads_count, 1);

        while (!xchg(&mp->available, 0)) {
            /* busy waiting for other threads to unlock */
            continue;
        }

        return;
    }
}

/*
 * @brief This should signals the end of a region of mutual exclusion.
 *
 *
 * @param mp The mutex to be unlocked.
 * @return Void.
 *
 */
void mutex_unlock(mutex_t *mp)
{
    if (mp == NULL) {
        /* the given mutex struct pointer is invalid, do nothing */
        return;
    }

    else if (!mp->init) {
        /* using a mutex before its initialized is undefined */
        return;
    }

    else {
        xchg(&mp->available, 1);
        xadd(&mp->threads_count, -1);

        return;
    }
}
