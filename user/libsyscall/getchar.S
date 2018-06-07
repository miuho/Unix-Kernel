/* user/libsyscall/getchar.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global getchar
getchar:
    INT     $GETCHAR_INT        /* make system call */
    RET                         /* return */
