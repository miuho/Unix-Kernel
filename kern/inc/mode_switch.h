/** @file kern/inc/mode_switch.h 
 * 
 *  @brief This file defines mode switch interface
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_MODE_SWITCH_H_
#define _KERN_INC_MODE_SWITCH_H_

/** @brief mode switch using the parameters to a user task
 *
 *  @param ds segment ds
 *  @param fs segment fs
 *  @param gs segment gs
 *  @param es segment es
 *  @param eip the eip we want to point to after mode switch
 *  @param cs segment cs
 *  @param eflags the eflags
 *  @param esp the esp we want to use after mode switch
 *  @param ss segment ss
 *  @return Void
 */
void mode_switch(unsigned long ds, unsigned long fs, unsigned long gs,
                unsigned long es, unsigned long eip, unsigned long cs,
                unsigned long eflags, unsigned long esp, unsigned long ss);

/** @brief restores the registers and mode switch to user task
 *
 *  @param edi register edi
 *  @param esi register esi
 *  @param ebp register ebp
 *  @param ebx register ebx
 *  @param ecx register ecx
 *  @param eax register eax
 *  @param ds segment ds
 *  @param fs segment fs
 *  @param gs segment gs
 *  @param es segment es
 *  @param eip the eip we want to point to after mode switch
 *  @param cs segment cs
 *  @param eflags the eflags
 *  @param esp the esp we want to use after mode switch
 *  @param ss segment ss
 *  @return Void
 */

void move_regs_and_mode_switch(unsigned long edi, unsigned long esi, 
                    unsigned long ebp, unsigned long ebx, unsigned long edx, 
                    unsigned long ecx, unsigned long eax,
                    unsigned long ds, unsigned long fs, unsigned long gs,
                    unsigned long es, unsigned long eip, unsigned long cs,
                    unsigned long eflags, unsigned long esp, unsigned long ss);
#endif
