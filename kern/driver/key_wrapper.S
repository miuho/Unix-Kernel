/* kern/key_wrapper.S */
/* Author: An Wu (anwu), Hingon Miu (hmiu) */

.global keyboard_wrapper
keyboard_wrapper:
    pusha               /* save all registers */
    call keyboard_handler  /* call the timer interrupt handler */
    popa                /* restore all registers */
    iret            /* return with info saved on the stack by the interrupt */
