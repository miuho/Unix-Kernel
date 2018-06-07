/* user/libsyscall/wait.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global wait
wait:
    PUSH    %esi
    MOV     8(%esp), %esi       /* prepare arg */
    INT     $WAIT_INT           /* make system call */
    POP     %esi
    RET                         /* return */
