/** @file kern/inc/reg.h
 *
 *  @brief the registers struct
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_REG_H_
#define _KERN_INC_REG_H_

#include <ureg.h>

/* the register strct definition */
typedef struct reg {
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;

    unsigned int eip;
    unsigned int eflags;
    unsigned int esp;

    unsigned int esp0;
} reg_t;

/** @brief create and copy a new reg struct
 *
 *  @param reg the reg struct we want to copy from
 *  @return the copied reg struct pointer on success, NULL on error
 */
reg_t *reg_copy(reg_t *reg);

/** @brief create a ureg using the passed parameters
 *
 *  @param ureg the pointer to the ureg struct
 *  @param cause 
 *  @param cr2
 *  @param ds
 *  @param es
 *  @param fs
 *  @param gs
 *  @param edi
 *  @param esi
 *  @param ebp
 *  @param ebx
 *  @param edx
 *  @param ecx
 *  @param eax
 *  @param error_code
 *  @param eip
 *  @param cs
 *  @param eflags
 *  @param esp
 *  @param ss
 *  @return 0 on success, -1 on error
 */
int ureg_create(ureg_t *ureg,
                unsigned int cause, unsigned cr2, unsigned ds,
                unsigned int es, unsigned int fs, unsigned int gs,
                unsigned int edi, unsigned int esi, unsigned int ebp,
                unsigned int ebx, unsigned int edx, unsigned int ecx,
                unsigned int eax, unsigned int error_code,
                unsigned int eip, unsigned int cs, unsigned int eflags,
                unsigned int esp, unsigned int ss);

/** @brief print the info of a ureg to console
 *
 *  @param ureg the ureg we want to print
 *  @return Void
 */
void print_ureg(ureg_t *ureg);

#endif
