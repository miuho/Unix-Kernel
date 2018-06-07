/** @file cond_private.c
 *
 *  @brief the private function for conditional variable
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <cond_private.h>
#include <mutex_private.h>
#include <st_queue.h>

int cond_idle(cond_t *cv) {
    return (cv->init == 1) && (st_queue_empty(cv->wait_ktcbs));
}
