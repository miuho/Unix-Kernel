/** @file sem_private.c
 *
 *  @brief private function for the semaphore lib
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <sem_private.h>
#include <cond_private.h>
#include <mutex_private.h>

int sem_idle(sem_t *sem) {
    return (sem->init == 1 && cond_idle(&(sem->cv)) && mutex_idle(&(sem->mp)));
}
