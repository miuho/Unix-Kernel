/** @file malloc_internal.c
 *  
 *  @brief internal functions for malloc package
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include "malloc_internal.h"
#include <mutex.h>

mutex_t _malloc_mp;

void malloc_init_mp() {
    mutex_init(&_malloc_mp);
}

void malloc_mutex_lock() {
    mutex_lock(&_malloc_mp);
}

void malloc_mutex_unlock() {
    mutex_unlock(&_malloc_mp);
}

