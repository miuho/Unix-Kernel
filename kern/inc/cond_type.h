/** @file cond_type.h
 *
 *  @brief This file defines the type for condition variables.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_COND_TYPE_H_
#define _KERN_INC_COND_TYPE_H_

#include <st_queue.h>

/* defines the conditional variable struct */
typedef struct cond {
    char init;      /* indicate initialization */

    st_queue wait_ktcbs;    /* the queue of threads waiting */
} cond_t;

#endif /* _COND_TYPE_H */
