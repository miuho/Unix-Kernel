/** @file mutex_private.c
 *
 *  @brief private function for the mutex lib
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <mutex_private.h>
#include <st_queue.h>

int mutex_idle(mutex_t *mp) {
    return (mp->init == 1) && (mp->available == 1) && (st_queue_empty(mp->queue));
}
