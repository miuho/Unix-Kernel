/* user/libsyscall/readfile.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global readfile
readfile:
    PUSH    %esi
    LEA     8(%esp), %esi       /* prepare arg */
    INT     $READFILE_INT       /* make system call */
    POP     %esi
    RET                         /* return */
