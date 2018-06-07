/** @file kern/inc/mutex.h
 *
 *  @brief This file defines the interface for mutexes.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_MUTEX_H_
#define _KERN_INC_MUTEX_H_

#include <mutex_type.h>

/** @brief init a mutex
 *
 *  @param mp the pointer to the mutex struct
 *  @return 0 on success, -1 on error
 */
int mutex_init( mutex_t *mp );

/** @brief destroy a mutex
 *
 *  @param mp the pointer to the mutext struct
 *  @return Void
 */
void mutex_destroy( mutex_t *mp );

/** @brief lock a mutex 
 *
 *  @param mp the mutex we want to lock
 *  @return Void
 */
void mutex_lock( mutex_t *mp );

/** @brief unlock a mutex atomically
 *
 *  @param mp the mutex we want to unlock
 *  @return Void
 */
void mutex_cond_unlock( mutex_t *mp );

/** @brief unlock a mutex
 *
 *  @param mp the mutex we want to unlock
 *  @return Void
 */
void mutex_unlock( mutex_t *mp );

#endif /* MUTEX_H */
