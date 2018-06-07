/* user/libsyscall/gettid.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global gettid
gettid:
    INT     $GETTID_INT         /* make system call */
    RET                         /* return */
