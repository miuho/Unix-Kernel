/* user/libsyscall/task_vanish.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global task_vanish
task_vanish:
    PUSH    %esi
    MOV     8(%esp), %esi       /* prepare arg */
    INT     $TASK_VANISH_INT    /* make system call */
    POP     %esi
    RET                         /* return */
