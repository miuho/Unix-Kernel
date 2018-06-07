/* user/libsyscall/make_runnable.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global make_runnable
make_runnable:
    PUSH    %esi
    MOV     8(%esp), %esi       /* prepare arg */
    INT     $MAKE_RUNNABLE_INT  /* make system call */
    POP     %esi
    RET                         /* return */
