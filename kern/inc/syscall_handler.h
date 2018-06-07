/** @file kern/inc/syscall_handler.h
 *
 *  @brief the syscall handlers
 *  
 *  @author HingOn Miu (hmiu)
 *  @author An Wu (anwu)
 */

#ifndef _KERN_INC_SYSCALL_HANDLER_H
#define _KERN_INC_SYSCALL_HANDLER_H

#include <ureg.h>
#include <syscall.h>
#include <mutex.h>

/* lock the console print screen */
extern mutex_t print_mp;

/**
 * @brief check cs, ss and eflags of a register
 *
 * @param ureg the input register.
 * @return 1 if successful, 0 otherwise.
 *
 */
int check_newureg(ureg_t *ureg);

/**
 * @brief initialize the syscall handlers.
 *
 * @return 0 if successful, -1 otherwise.
 *
 */
int syscall_init(); 

/**
 * @brief initialize the mutex lock for print
 *
 * @return 0 if successful, -1 otherwise.
 *
 */
int print_init();

/**
 * @brief fill the console with keyboard without context switch.
 *
 * @return Void.
 *
 */
void fill_cons();


#endif
