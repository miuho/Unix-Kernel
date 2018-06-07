/* kern/pgfault_wrapper.S */
/* Author: Hingon Miu (hmiu@andrew.cmu.edu), An Wu (anwu@andrew.cmu.edu) */

.global pgfault_wrapper
pgfault_wrapper:
    pusha                   /* save all registers */

    call pgfault_handler    /* call the timer interrupt handler */
    
    popa                    /* restore all registers */
    add $4, %esp            /* skip error code */
    iret        /* return with info saved on the stack by the interrupt */
