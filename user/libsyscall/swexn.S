/* user/libsyscall/swexn.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global swexn
swexn:
    PUSH    %esi
    LEA     8(%esp), %esi       /* prepare arg */
    INT     $SWEXN_INT          /* make system call */
    POP     %esi
    RET                         /* return */
