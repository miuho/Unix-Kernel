/* user/libsyscall/remove_pages.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global remove_pages
remove_pages:
    PUSH    %esi
    MOV     8(%esp), %esi       /* prepare arg */
    INT     $REMOVE_PAGES_INT       /* make system call */
    POP     %esi
    RET                         /* return */
