/* user/libsyscall/halt.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global halt
halt:
    INT     $HALT_INT           /* make system call */
    RET                         /* return */
