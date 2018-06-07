/** @file sem_type.h
 *  @brief This file defines the type for semaphores.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _SEM_TYPE_H
#define _SEM_TYPE_H

#include <mutex.h>
#include <cond.h>

typedef struct sem {
    char init;      /* indicate if a sem is initialized */ 
    int count;      /* the available count of semaphore */
    cond_t cv;      /* the conditional variable for blocking threads */
    mutex_t mp;     /* the mutex for updating count */
} sem_t;

#endif /* _SEM_TYPE_H */
