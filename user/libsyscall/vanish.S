/* user/libsyscall/vanish.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global vanish
vanish:
    INT     $VANISH_INT         /* make system call */
    RET
