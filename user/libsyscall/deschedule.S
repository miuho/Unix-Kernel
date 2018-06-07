/* user/libsyscall/deschedule.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global deschedule
deschedule:
    PUSH    %esi
    MOV     8(%esp), %esi       /* prepare arg */
    INT     $DESCHEDULE_INT     /* make system call */
    POP    %esi
    RET                         /* return */
