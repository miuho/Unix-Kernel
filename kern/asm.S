/* kern/asm.S */
/* asm functions */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

.global xchg
xchg:
    MOV     8(%esp), %eax       /* prepare src */
    MOV     4(%esp), %edx       /* prepare dest */
    XCHG    %eax, (%edx)        /* add src to dest */
    RET

.global xadd
xadd:
    MOV     8(%esp), %eax       /* prepare src */
    MOV     4(%esp), %edx       /* prepare dest */
    XADD    %eax, (%edx)        /* add src to dest */
    RET

.global get_ebp
get_ebp:
    MOVL    %ebp, %eax
    RET

.global set_ebp_and_switch
set_ebp_and_switch:
    MOVL    4(%esp), %ebp
    MOVL    %ebp, %esp
    POP     %ebp
    RET

.global halt
halt:
    HLT
