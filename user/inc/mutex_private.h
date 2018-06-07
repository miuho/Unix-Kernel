/** @file mutex_private.h
 *
 *  @brief private function for mutex
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */
#ifndef _MUTEX_PRIVATE_H
#define _MUTEX_PRIVATE_H

#include <mutex.h>

/** @brief check if a mutex is idle
 *
 *  @param mp the mutex we want to check
 *  @return 1 if idle, 0 if not
 */
int mutex_idle(mutex_t *mp);

#endif /* !_MUTEX_PRIVATE_H */
