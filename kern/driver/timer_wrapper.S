/* kern/timer_wrapper.S */
/* Author: An Wu (anwu) Hingon Miu (hmiu) */

.global timer_wrapper
timer_wrapper:
    pusha               /* save all registers */
    call timer_handler  /* call the timer interrupt handler */
    popa                /* restore all registers */
    iret            /* return with info saved on the stack by the interrupt */

