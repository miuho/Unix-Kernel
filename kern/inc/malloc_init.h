/** @file kern/inc/malloc_init.h
 *
 *  @brief This file defines interface for initing malloc package.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_MALLOC_INIT_
#define _KERN_INC_MALLOC_INIT_

/** @brief init the malloc wrapper
  *
  * @return 0 on success, -1 on fail
  */
int malloc_init(void);

#endif


