/* user/libsyscall/set_status.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global set_status
set_status:
    PUSH    %esi
    MOV     8(%esp), %esi           /* prepare argument */
    INT     $SET_STATUS_INT         /* make system call */
    POP     %esi
    RET                             /* return */
