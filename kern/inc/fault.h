/** @file kern/inc/fault.h
 *  @brief This file defines the "ultimate" fault handler interface. 
 *         It is used when other fault handlers fail to handle a fault
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_FAULT_H_
#define _KERN_INC_FAULT_H_

#include <ureg.h>

/* the exit status if a user thread is killed by this handler */
#define FAULT_KILL_EXIT_STATUS -2

/** @brief the "ultimate" fault handler for faults from user
 *         This handler first check if user have registered swexn,
 *         and use it if so. Else just kill the thread and free its
 *         frames. (if it sees that the whole process is exited, then it will
 *         signal its parent.
 *
 *  @param ureg the fault info registers
 *  @return Void
 */
void fault_handler(ureg_t *ureg);

#endif
