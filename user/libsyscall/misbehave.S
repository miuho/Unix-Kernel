/* user/libsyscall/misbehave.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global misbehave
misbehave:
    PUSH    %esi
    MOV     8(%esp), %esi       /* prepare arg */
    INT     $MISBEHAVE_INT      /* make system call */
    POP     %esi
    RET                         /* return */
