/* kern/common_wrapper.S */
/* assembly for syscall wrappers */
/* Author: An Wu (anwu@andrew.cmu.edu) Hingon Miu (hmiu@andrew.cmu.edu) */

.global fork_wrapper
fork_wrapper:
    push %ecx
    push %edx
    push %ebp
    push %ebx
    push %esi
    push %edi
    push %eax
    call fork_handler
    pop %eax
    pop %edi
    pop %esi
    pop %ebx
    pop %ebp
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global exec_wrapper
exec_wrapper:
    push %ecx
    push %edx
    push %esi
    call exec_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global yield_wrapper
yield_wrapper:
    push %ecx
    push %edx
    push %esi
    call yield_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global deschedule_wrapper
deschedule_wrapper:
    push %ecx
    push %edx
    push %esi
    call deschedule_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global make_runnable_wrapper
make_runnable_wrapper:
    push %ecx
    push %edx
    push %esi
    call make_runnable_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global wait_wrapper
wait_wrapper:
    push %ecx
    push %edx
    push %esi
    call wait_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global gettid_wrapper
gettid_wrapper:
    push %ecx
    push %edx
    call gettid_handler  /* call the syscall handler handler */
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global new_pages_wrapper
new_pages_wrapper:
    push %ecx
    push %edx
    push %esi
    call new_pages_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global remove_pages_wrapper
remove_pages_wrapper:
    push %ecx
    push %edx
    push %esi
    call remove_pages_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global sleep_wrapper
sleep_wrapper:
    push %ecx
    push %edx
    push %esi
    call sleep_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global getchar_wrapper
getchar_wrapper:
    push %ecx
    push %edx
    call getchar_handler  /* call the syscall handler handler */
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global readline_wrapper
readline_wrapper:
    push %ecx
    push %edx
    push %esi
    call readline_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global print_wrapper
print_wrapper:
    push %ecx
    push %edx
    push %esi
    call print_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global set_term_color_wrapper
set_term_color_wrapper:
    push %ecx
    push %edx
    push %esi
    call set_term_color_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global set_cursor_pos_wrapper
set_cursor_pos_wrapper:
    push %ecx
    push %edx
    push %esi
    call set_cursor_pos_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global get_cursor_pos_wrapper
get_cursor_pos_wrapper:
    push %ecx
    push %edx
    push %esi
    call get_cursor_pos_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global thread_fork_wrapper
thread_fork_wrapper:
    push %ecx
    push %edx
    push %ebx
    push %ebp
    push %edi
    push %eax
    call thread_fork_handler  /* call the syscall handler handler */
    pop %eax
    pop %edi
    pop %ebp
    pop %ebx
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global get_ticks_wrapper
get_ticks_wrapper:
    push %ecx
    push %edx
    call get_ticks_handler  /* call the syscall handler handler */
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global misbehave_wrapper
misbehave_wrapper:
    push %ecx
    push %edx
    push %esi
    call misbehave_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global task_vanish_wrapper
task_vanish_wrapper:
    push %ecx
    push %edx
    push %esi
    call task_vanish_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global halt_wrapper
halt_wrapper:
    push %ecx
    push %edx
    call halt_handler  /* call the syscall handler handler */
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global set_status_wrapper
set_status_wrapper:
    push %eax
    push %ecx
    push %edx
    push %esi
    call set_status_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    pop %eax
    iret              /* return with info saved on the stack by the trap */

.global vanish_wrapper
vanish_wrapper:
    push %ecx
    push %edx
    call vanish_handler  /* call the syscall handler handler */
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global readfile_wrapper
readfile_wrapper:
    push %ecx
    push %edx
    push %esi
    call readfile_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    iret              /* return with info saved on the stack by the trap */

.global swexn_wrapper
swexn_wrapper:
    push %ecx
    push %edx
    push %esi
    call swexn_handler  /* call the syscall handler handler */
    pop %esi
    pop %edx
    pop %ecx
    iret
