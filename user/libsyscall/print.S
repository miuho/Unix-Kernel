/* user/libsyscall/print.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global print
print:
    PUSH    %esi
    LEA     8(%esp), %esi       /* prepare arg */
    INT     $PRINT_INT          /* make system call */
    POP     %esi
    RET                         /* return */
