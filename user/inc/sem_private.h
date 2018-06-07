/** @file sem_private.h
 *
 *  @brief private function for semaphore
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _SEM_PRIVATE_H
#define _SEM_PRIVATE_H

#include <sem.h>

/** @brief checks if a semaphore is in idle state
 *
 *  @param sem the semaphore we want to check
 *  @return 1 if idle, 0 if not
 */
int sem_idle(sem_t *sem);

#endif /* !_SEM_PRIVATE_H */
