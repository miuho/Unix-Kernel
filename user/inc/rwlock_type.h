/** @file rwlock_type.h
 *  @brief This file defines the type for reader/writer locks.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _RWLOCK_TYPE_H
#define _RWLOCK_TYPE_H

#include <sem.h>
#include <mutex.h>
#include <splay_tree.h>

typedef struct rwlock {
    char init;          /* indicate if a rwlock is initialized */
    int read_count;     /* indicate the number of readers */

    splay_tree rw_tids; /* stores the thr_key of writers in a splay tree */

    sem_t enter_mutex;  /* sem for entering the reading area */
    sem_t rw_mutex;     /* sem for writers */

    sem_t mutex;        /* sem for enter and exit the reader session */
    mutex_t st_mp;      /* mutex for accessing splay tree */
} rwlock_t;

#endif /* _RWLOCK_TYPE_H */
