/** @file mutex_type.h
 *  @brief This file defines the type for mutexes.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_MUTEX_TYPE_H
#define _KERN_INC_MUTEX_TYPE_H

#include <st_queue.h>

/* declare ktcb type */
struct ktcb;

typedef struct mutex {  
    /* indicate if a mutex is initialized */
    char init;     
    
    /* indicate if the mutex is available */     
    int available;   
    
    /* the waiting queue */   
    st_queue queue;

    /* the current holder */
    struct ktcb *holder;
} mutex_t;

#endif /* _MUTEX_TYPE_H */
