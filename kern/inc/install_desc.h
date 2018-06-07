/** @file kern/install_desc.h
 *
 *  @brief idt descriptor manipulation
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_INSTALL_DESC_H
#define _KERN_INC_INSTALL_DESC_H

/* defines the gate type (trap_gate and interrupt_gate) */
enum GATE_TYPE {
    trap_gate,
    interrupt_gate
};

/** @brief install a gate descriptor (hard-coded to trap gate)
 *
 *  @param base the idt_base pointer
 *  @param index the index we want to install the descriptor
 *  @param handler the address of the handler function
 */
void install_desc(void *base, int index, void (*handler)(), 
                    enum GATE_TYPE gate_type, int dpl);

#endif /* !_INSTALL_DESC_H */
