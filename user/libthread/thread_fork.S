/* user/libsyscall/thread_fork.S */
/* Author: Hingon Miu (hmiu), An Wu (anwu) */

#include <syscall_int.h>

.global thread_fork
thread_fork:
    MOV     4(%esp), %ecx       /* save child_stack_base in %ecx, since  
                                 * all regs except %eax are reserved */
    MOV     8(%esp), %edx       /* save child thr_key in %edx */
    MOV     %edx, (%ecx)        /* put thr_key at child_stack_base */
    INT     $THREAD_FORK_INT    /* make system call */
    TEST    %eax, %eax          /* test %eax to see parent or child */
    JZ      child_branch        /* if %eax = 0, jump to child branch */
    RET                         /* parent return */

child_branch:
    MOV     %ecx, %esp      /* esp = child_stack_base - 4 */
    CALL    setup_and_run_child /* find child info in splay tree and run */
    /* this should not return */
