/* user/libsyscall/sleep.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global sleep
sleep:
    PUSH    %esi
    MOV     8(%esp), %esi       /* prepare arg */
    INT     $SLEEP_INT          /* make system call */
    POP     %esi
    RET                         /* return */
