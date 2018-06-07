/** @file cond_private.h
 *
 *  @brief private function for conditional variables
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_COND_PRIVATE_H_
#define _KERN_INC_COND_PRIVATE_H_

#include <cond.h>

/** @brief check if a conditional variable is idle
 *
 *  @param cv the conditional variable we want to check
 *  @return 1 if idle, 0 if not
 */
int cond_idle(cond_t *cv);

#endif /* !_COND_PRIVATE_H */
