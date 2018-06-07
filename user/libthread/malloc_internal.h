/** @file malloc_internal.h
 *
 *  @brief internal functions for malloc package
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _MALLOC_PRIVATE_H
#define _MALLOC_PRIVATE_H

/** @brief init the mutex in malloc wrapper
 *  
 *  @return Void
 */
void malloc_init_mp();

/** @brief lock the mutex for malloc package
 *
 *  @return Void
 */
void malloc_mutex_lock();

/** @brief unlock the mutex for malloc package
 *
 *  @return Void
 */
void malloc_mutex_unlock();

#endif /* !_MALLOC_PRIVATE_H */
