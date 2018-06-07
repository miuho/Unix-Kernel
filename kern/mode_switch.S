/* @file kern/mode_switch.S */
/* mode switch to user space */
/* Author: Hingon Miu (hmiu) An Wu (anwu) */

.global mode_switch
mode_switch:
    addl $4, %esp  /* pad out return address of caller */
    pop %ds
    pop %fs
    pop %gs
    pop %es
    iret


/* for swexn */
.global move_regs_and_mode_switch
move_regs_and_mode_switch:
    addl $4, %esp /* pad out return address of caller */
    pop %edi
    pop %esi
    pop %ebp
    pop %ebx
    pop %edx
    pop %ecx
    pop %eax
    pop %ds
    pop %fs
    pop %gs
    pop %es
    iret
