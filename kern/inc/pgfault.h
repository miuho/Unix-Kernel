/** @file kern/inc/pgfault.h
 *  @brief This file defines the page fault handler interface.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */ 


#ifndef _KERN_INC_PGFAULT_H_
#define _KERN_INC_PGFAULT_H_

/** @brief install the page fault
 *
 *  @return Void
 */
void pgfault_handler_install();

/** @brief the page fault handler
 *
 *  @param edi register edi
 *  @param esi register esi
 *  @param ebp register ebp
 *  @param esp register esp
 *  @param ebx register ebx
 *  @param edx register edx
 *  @param ecx register ecx
 *  @param eax register eax
 *  @param error_code the error code
 *  @param fault_eip the fault eip
 *  @param fault_cs the fault cs
 *  @param fault_eflags the fault eflags
 *  @param fault_esp the fault esp
 *  @param fault_ss the fault ss
 *  @return Void
 */
void pgfault_handler(unsigned long edi, unsigned long esi, unsigned long ebp,
                    unsigned long esp, unsigned long ebx, unsigned long edx,
                    unsigned long ecx, unsigned long eax,
                    unsigned long error_code, unsigned long fault_eip,
                    unsigned long fault_cs, unsigned long fault_eflags,
                    unsigned long fault_esp, unsigned long fault_ss); 

#endif
