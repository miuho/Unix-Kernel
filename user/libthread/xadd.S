/* user/libthread/xadd.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */
/* bug:  Check if the passed in mutex pointer is writable or not. */
/* bug:  Cant make sure that destroyed mutex from un-initialized ones. */


.global xadd
xadd:
    MOV     8(%esp), %eax       /* prepare src */
    MOV     4(%esp), %edx       /* prepare dest */
    XADD    %eax, (%edx)        /* add src to dest */
    RET
