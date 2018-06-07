/** @file rwlock.c
 *
 *  @brief functions for the read-write lock
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <rwlock.h>

#include <stddef.h>
#include <thread.h>
#include <mutex_private.h>
#include <sem_private.h>

/*
 * @brief This should compare the keys for splay tree.
 *
 * @param t1 The first key.
 * @param t2 The second key.
 *
 * @return 1 if t1 > t2, -1 if t1 < t2, 0 if t1 == t2.
 */
int thr_id_comp(int t1, int t2) {
    if (t1 > t2) return 1;
    else if (t1 < t2) return -1;
    else return 0;
}

/*
 * @brief This should initialize the lock pointed to
 *        by rwlock.
 *
 * @param rwlock The rwlock to be initialized.
 *
 * @return 0 on success, -1 on error.
 */
int rwlock_init( rwlock_t *rwlock ) 
{
    if (rwlock == NULL)
        return -1;

    rwlock->read_count = 0;
    sem_init(&(rwlock->rw_mutex), 1);
    sem_init(&(rwlock->enter_mutex), 1);
    sem_init(&(rwlock->mutex), 1);
    rwlock->rw_tids = st_new(thr_id_comp);
    mutex_init(&(rwlock->st_mp));
    rwlock->init = 1;
    return 0;
}


/*
 * @brief This should block the calling thread until it
 *        has been granted the requested form of access.
 *
 * @param rwlock The rwlock to block on.
 * @param type The type of lock.
 *
 * @return Void.
 */
void rwlock_lock( rwlock_t *rwlock, int type ) 
{
    if (type == RWLOCK_READ) {
        /* make sure writer is not waiting */
        int empty;
        do {
            sem_wait(&(rwlock->enter_mutex));
            sem_signal(&(rwlock->enter_mutex));

            mutex_lock(&(rwlock->st_mp));
            empty = st_empty(rwlock->rw_tids);
            mutex_unlock(&(rwlock->st_mp));

        } while (!empty);

        sem_wait(&(rwlock->mutex));
        rwlock->read_count++;
        if (rwlock->read_count == 1)
            sem_wait(&(rwlock->rw_mutex));
        sem_signal(&(rwlock->mutex));

        /* reading */
    }
    else {

        mutex_lock(&(rwlock->st_mp));
        st_insert(rwlock->rw_tids, thr_getid(), (void *)1);
        mutex_unlock(&(rwlock->st_mp));
        
        sem_wait(&(rwlock->enter_mutex));
        sem_wait(&(rwlock->rw_mutex));

        /* writing */
    }
}

/*
 * @brief This should indicate the calling thread is done using
 *        the locked state in whichever mode it was granted 
 *        access for.
 *
 * @param rwlock The rwlock to indicate the done signal.
 *
 * @return Void.
 */
void rwlock_unlock( rwlock_t *rwlock )
{

        /* reader */
        mutex_lock(&(rwlock->st_mp));
        int is_writer = (int) st_delete(rwlock->rw_tids, thr_getid());
        mutex_unlock(&(rwlock->st_mp));

        if (is_writer) {
            sem_signal(&(rwlock->rw_mutex));
            sem_signal(&(rwlock->enter_mutex));
        }
        else {
            sem_wait(&(rwlock->mutex));
            rwlock->read_count--;
            if (rwlock->read_count == 0)
                sem_signal(&(rwlock->rw_mutex));
            sem_signal(&(rwlock->mutex));
        }

}

/*
 * @brief This should deactivate the lock pointed to
 *        by rwlock.
 *
 * @param rwlock The rwlock to be deactivated.
 *
 * @return Void.
 */
void rwlock_destroy( rwlock_t *rwlock )
{
    if (rwlock->init && sem_idle(&(rwlock->enter_mutex)) && 
        sem_idle(&(rwlock->rw_mutex)) &&
        sem_idle(&(rwlock->mutex)) && mutex_idle(&(rwlock->st_mp))) {
        sem_destroy(&(rwlock->enter_mutex));
        sem_destroy(&(rwlock->rw_mutex));
        sem_destroy(&(rwlock->mutex));
        mutex_destroy(&(rwlock->st_mp));
        st_destroy(rwlock->rw_tids);
        rwlock->init = 0;
    }

}


/*
 * @brief This should ensure no threads hold the lock in writer mode
 *        such that turns lock from writer mode to reader mode.
 *
 * @param rwlock The reader/writer lock.
 *
 * @return Void.
 */
void rwlock_downgrade( rwlock_t *rwlock) {
    mutex_lock(&(rwlock->st_mp));
    st_delete(rwlock->rw_tids, thr_getid());
    mutex_unlock(&(rwlock->st_mp));

    sem_signal(&(rwlock->enter_mutex));
    sem_signal(&(rwlock->rw_mutex));

    sem_wait(&(rwlock->mutex));
    rwlock->read_count++;
    if (rwlock->read_count == 1)
        sem_wait(&(rwlock->rw_mutex));
    sem_signal(&(rwlock->mutex));

}
