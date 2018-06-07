/** @file asm.h
 *  @brief This file defines the assembly atomic functions.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_ASM_H_
#define _KERN_INC_ASM_H_

/**
 * @brief Atomic exchange
 *
 * @param source The source value.
 * @param destination The address of the destination.
 * @return Old destination value.
 */
int xchg(int *destination, int source);

/**
 * @brief Atomic addition
 *
 * @param source The source value.
 * @param destination The address of the destination.
 * @return Old destination value.
 */
int xadd(int *destination, int source);

/**
 * @brief get current ebp
 *
 * @return the current ebp
 */
unsigned long get_ebp();

/**
 * @brief this function set the ebp to the destination kernel thread's
 *        saved ebp and do the context switch
 *
 * @param ebp the target's ebp
 * @return Void
 */
void set_ebp_and_switch(unsigned long ebp);

/**
 * @brief assembly HLT
 *
 * @return Void
 */
void halt();

#endif
