/** @file kern/inc/common_handler.h
 *
 *  @brief the wrapper to install all the syscalls
 *
 *  @author Hingon Miu (hmiu)
 *  @author An Wu (anwu)
 */

#ifndef _KERN_INC_COMMON_HANDLER_H_
#define  _KERN_INC_COMMON_HANDLER_H_


/** @brief init the syscalls
 *
 *  @param idt_base_p the current idt base
 *  @return Void
 */
void syscall_install(void *idt_base_p);


#endif
