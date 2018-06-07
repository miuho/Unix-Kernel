/* user/libthread/xchg.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */
/* bug:  Check if the passed in mutex pointer is writable or not. */
/* bug:  Cant make sure that destroyed mutex from un-initialized ones. */


.global xchg
xchg:
    MOV     8(%esp), %eax       /* prepare src */
    MOV     4(%esp), %edx       /* prepare dest */
    XCHG    %eax, (%edx)        /* add src to dest */
    RET
