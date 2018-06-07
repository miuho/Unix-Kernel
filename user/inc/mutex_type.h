/** @file mutex_type.h
 *  @brief This file defines the type for mutexes.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H


typedef struct mutex {  
    char init;          /* indicate if a mutex is initialized */
    int threads_count;  /* indicate the # threads that's waiting on mutex */
    int available;      /* indicate if the mutex is available */
} mutex_t;

#endif /* _MUTEX_TYPE_H */
