/* user/libsyscall/get_esp.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global get_esp
get_esp:
    MOV     %esp, %eax          /* set child esp */
    RET                         /* return */
