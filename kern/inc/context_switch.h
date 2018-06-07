/** @file kern/inc/context_switch.h
 *
 *  @brief The context switch interface.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_CONTEXT_SWITCH_H_
#define _KERN_INC_CONTEXT_SWITCH_H_

#include <kthread_pool.h>

/** @brief called by cs_save_and_switch.
 *         this save the current ebp, and atomically change running_ktcb
 *         as well as switch to the target kernel thread.
 *         Return directly if the target is same as source.
 *
 *  @param edi padding
 *  @param esi padding
 *  @param ebp the ebp we want to save for "to"
 *  @param esp padding
 *  @param ebx padding
 *  @param edx padding
 *  @param ecx padding
 *  @param eax padding
 *  @param eip padding
 *  @param from the pointer to the source ktcb_t that we switch from
 *  @param to the pointer to the source ktcb_t that we switch to
 *  @return Void
 */
void cs_switch(unsigned long edi, unsigned long esi, unsigned long ebp,
                unsigned long esp, unsigned long ebx, unsigned long edx,
                unsigned long ecx, unsigned long eax,
                unsigned long eip, ktcb_t *from, ktcb_t *to);

/** @brief save the context and switch to another kernel thread.
 *         This is the wrapper for cs_switch. It disables interrupts and
 *         save the register info on the stack
 *
 *  @param from the pointer to the source ktcb_t that we switch from
 *  @param to the pointer to the source ktcb_t that we switch to
 *  @return Void
 */
void cs_save_and_switch(ktcb_t *from, ktcb_t *to);

#endif
