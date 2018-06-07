/* user/libsyscall/readline.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global readline
readline:
    PUSH    %esi
    LEA     8(%esp), %esi       /* prepare arg */
    INT     $READLINE_INT       /* make system call */
    POP     %esi
    RET                         /* return */
