/** @file cond_type.h
 *
 *  @brief This file defines the type for condition variables.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _COND_TYPE_H
#define _COND_TYPE_H

#include <queue.h>

typedef struct cond {
    char init;      /* indicate initialization */
    queue wait_tids;    /* the queue of threads waiting */
    mutex_t lock;   /* used to protect the data of cond struct */
} cond_t;

#endif /* _COND_TYPE_H */
