/* user/libsyscall/get_ticks.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global get_ticks
get_ticks:
    INT     $GET_TICKS_INT      /* make system call */
    RET                         /* return */
