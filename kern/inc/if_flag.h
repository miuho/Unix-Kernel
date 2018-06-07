/** @file kern/inc/if_flag.h
 *  @brief This file defines the advanced disable interrupt interface.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_IF_FLAG_
#define _KERN_INC_IF_FLAG_

/** @brief enable interrupts and return if interrupts were enabled just now
 *
 *  @return 0 if interrupts were not enabled just now. not 0 otherwise
 */
int if_enable(void);

/** @brief disable interrupts and return if interrupts were enabled just now
 *
 *  @return 0 if interrupts were not enabled just now. not 0 otherwise
 */
int if_disable(void);

/** @brief set interrupts enable/disable based on a parameter
 *
 *  @param if_was_set 1 if we want to enable interrupts, 0 otherwise
 *  @return Void
 */
void if_recover(int if_was_set);

#endif
