/** @file sem.c
 *
 *  @brief functions for semaphore
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <stddef.h>
#include <mutex_private.h>
#include <cond_private.h>

#include <sem.h>

/*
 * @brief This should initialize the semaphore pointed to
 *        by sem to the value count.
 *
 * @param sem The semaphore to be initialized.
 * @param count The number of times to wait.
 *
 * @return 0 on success, -1 on error.
 */
int sem_init(sem_t *sem, int count)
{
    if (sem == NULL) {
        return -1;
    }

    sem->count = count;
    cond_init(&(sem->cv));
    mutex_init(&(sem->mp));
    sem->init = 1;
    return 0;
}

/*
 * @brief This should allows a thread to decrement a
 *        semaphore value and may cause it to block
 *        indefinitely until it is legal to perform the
 *        decrement.
 *
 * @param sem The semaphore to block till the decrement.
 *
 * @return Void.
 */
void sem_wait(sem_t *sem)
{
    mutex_lock(&(sem->mp));
    sem->count--;
    if (sem->count < 0) {
        cond_wait(&(sem->cv), &(sem->mp));
    }
    mutex_unlock(&(sem->mp));
}

/*
 * @brief This should wake up a thread waiting on the semaphore
 *        pointed to by sem.
 *
 * @param sem The semaphore to wake up.
 *
 * @return Void.
 */
void sem_signal(sem_t *sem) 
{
    mutex_lock(&(sem->mp));
    sem->count++;
    if (sem->count <= 0) {
        cond_signal(&(sem->cv));
    }
    mutex_unlock(&(sem->mp));

}

/*
 * @brief This should deactivate the semaphore pointed to
 *        by sem.
 *
 * @param sem The semaphore to be deactivated.
 *
 * @return Void.
 */
void sem_destroy(sem_t *sem) 
{
    if (mutex_idle(&(sem->mp)) && (cond_idle(&(sem->cv)))) {
        sem->init = 0;
        cond_destroy(&(sem->cv));
        mutex_destroy(&(sem->mp));
    }
}
