/* user/libsyscall/yield.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global yield
yield:
    PUSH    %esi
    MOV     8(%esp), %esi       /* prepare arg */
    INT     $YIELD_INT          /* make system call */
    POP     %esi
    RET                         /* return */
