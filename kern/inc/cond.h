/** @file cond.h
 *  @brief This file defines the interface for condition variables.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_COND_H_
#define _KERN_INC_COND_H_

#include <mutex.h>
#include <cond_type.h>

/** @brief init a cond variable
 *
 *  @param cv the pointer to the cond struct
 *  @return 0 on success, -1 on error
 */
int cond_init( cond_t *cv );

/** @brief destroy a cond variable
 *
 *  @param cv the pointer to the cond struct
 *  @return Void
 */
void cond_destroy( cond_t *cv );

/** @brief wait on a condition and atomically unlock the mutex
 *
 *  @param cv the pointer to the cond struct
 *  @param mp the pointer to the mutex (want to atomically unlock)
 *  @return Void
 */
void cond_wait( cond_t *cv, mutex_t *mp );

/** @brief signal a conditional variable
 *
 *  @param cv the pointer to the cond struct
 *  @return Void
 */
void cond_signal( cond_t *cv );

/** @brief signal a conditional variable and terminate the current 
 *         kernel thread
 *
 *  @param cv the pointer to the cond struct
 *  @return Void
 */
void cond_signal_terminate( cond_t *cv );

/** @brief broadcast on a conditional variable
 *
 *  @param cv the pointer to the cond struct
 *  @return Void
 */
void cond_broadcast( cond_t *cv );

#endif /* COND_H */
