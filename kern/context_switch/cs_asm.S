/* kern/context_switch/cs_asm.S */
/* the wrapper (save and restore registers for context switch) */
/* Author: An Wu (anwu), Hingon Miu (hmiu) */

.global cs_save_and_switch
cs_save_and_switch:
    call if_disable /* disable interrupts and put prev interrupt state in eax*/
    pusha
    call cs_switch
    popa
    push %eax       /* restore the prev interrupt state */
    call if_recover
    pop %eax
    ret



