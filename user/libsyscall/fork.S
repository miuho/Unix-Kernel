/* user/libsyscall/fork.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global fork
fork:
    INT     $FORK_INT           /* make system call */
    RET                         /* return */
