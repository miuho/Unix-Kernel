/** @file kern/common_handler.c
 *
 *  @brief the trap gate handlers
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_int.h>
#include <common_wrapper.h>
#include <install_desc.h>
#include <reporter.h>

static char *tag = "common_handler";

/** @brief install the fork syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void fork_install(void *idt_base_p) {
    install_desc(idt_base_p, FORK_INT, fork_wrapper, 
                    trap_gate, 3);
}

/** @brief install the exec syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void exec_install(void *idt_base_p) {
    install_desc(idt_base_p, EXEC_INT, exec_wrapper, 
                    trap_gate, 3);
}

/** @brief install the wait syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void wait_install(void *idt_base_p) {
    install_desc(idt_base_p, WAIT_INT, wait_wrapper, 
                    trap_gate, 3);
}

/** @brief install the yield syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void yield_install(void *idt_base_p) {
    install_desc(idt_base_p, YIELD_INT, yield_wrapper, 
                    trap_gate, 3);
}

/** @brief install the deschedule syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void deschedule_install(void *idt_base_p) {
    install_desc(idt_base_p, DESCHEDULE_INT, deschedule_wrapper, 
                    trap_gate, 3);
}

/** @brief install the make_runnable syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void make_runnable_install(void *idt_base_p) {
    install_desc(idt_base_p, MAKE_RUNNABLE_INT, make_runnable_wrapper, 
                    trap_gate, 3);
}

/** @brief install the gettid syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void gettid_install(void *idt_base_p) {
    install_desc(idt_base_p, GETTID_INT, gettid_wrapper, 
                    trap_gate, 3);
}

/** @brief install the new_pages syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void new_pages_install(void *idt_base_p) {
    install_desc(idt_base_p, NEW_PAGES_INT, new_pages_wrapper, 
                    trap_gate, 3);
}

/** @brief install the remove_pages syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void remove_pages_install(void *idt_base_p) {
    install_desc(idt_base_p, REMOVE_PAGES_INT, remove_pages_wrapper, 
                    trap_gate, 3);
}

/** @brief install the sleep syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void sleep_install(void *idt_base_p) {
    install_desc(idt_base_p, SLEEP_INT, sleep_wrapper, 
                    trap_gate, 3);
}

/** @brief install the getchar syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void getchar_install(void *idt_base_p) {
    install_desc(idt_base_p, GETCHAR_INT, getchar_wrapper, 
                    trap_gate, 3);
}

/** @brief install the readline syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void readline_install(void *idt_base_p) {
    install_desc(idt_base_p, READLINE_INT, readline_wrapper, 
                    trap_gate, 3);
}

/** @brief install the print syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void print_install(void *idt_base_p) {
    install_desc(idt_base_p, PRINT_INT, print_wrapper, 
                    trap_gate, 3);
}

/** @brief install the set_term_color syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void set_term_color_install(void *idt_base_p) {
    install_desc(idt_base_p, SET_TERM_COLOR_INT, set_term_color_wrapper, 
                    trap_gate, 3);
}

/** @brief install the set_cursor_pos syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void set_cursor_pos_install(void *idt_base_p) {
    install_desc(idt_base_p, SET_CURSOR_POS_INT, set_cursor_pos_wrapper, 
                    trap_gate, 3);
}

/** @brief install the get_cursor_pos syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void get_cursor_pos_install(void *idt_base_p) {
    install_desc(idt_base_p, GET_CURSOR_POS_INT, get_cursor_pos_wrapper, 
                    trap_gate, 3);
}

/** @brief install the thread_fork syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void thread_fork_install(void *idt_base_p) {
    install_desc(idt_base_p, THREAD_FORK_INT, thread_fork_wrapper, 
                    trap_gate, 3);
}

/** @brief install the get_ticks syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void get_ticks_install(void *idt_base_p) {
    install_desc(idt_base_p, GET_TICKS_INT, get_ticks_wrapper, 
                    trap_gate, 3);
}

/** @brief install the misbehave syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void misbehave_install(void *idt_base_p) {
    install_desc(idt_base_p, MISBEHAVE_INT, misbehave_wrapper, 
                    trap_gate, 3);
}

/** @brief install the halt syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void halt_install(void *idt_base_p) {
    install_desc(idt_base_p, HALT_INT, halt_wrapper, 
                    trap_gate, 3);
}

/** @brief install the task_vanish syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void task_vanish_install(void *idt_base_p) {
    install_desc(idt_base_p, TASK_VANISH_INT, task_vanish_wrapper, 
                    trap_gate, 3);
}

/** @brief install the set_status syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void set_status_install(void *idt_base_p) {
    install_desc(idt_base_p, SET_STATUS_INT, set_status_wrapper, 
                    trap_gate, 3);
}

/** @brief install the vanish syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void vanish_install(void *idt_base_p) {
    install_desc(idt_base_p, VANISH_INT, vanish_wrapper, 
                    trap_gate, 3);
}

/** @brief install the readfile syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void readfile_install(void *idt_base_p) {
    install_desc(idt_base_p, READFILE_INT, readfile_wrapper, 
                    trap_gate, 3);
}

/** @brief install the swexn syscall
 *  
 *  @param idt_base_p the idt base pointer
 *  @return Void
 */
void swexn_install(void *idt_base_p) {
    install_desc(idt_base_p, SWEXN_INT, swexn_wrapper, 
                    trap_gate, 3);
}

void syscall_install(void *idt_base_p) {
    report_progress(tag, "installing syscall to idt");

    /* install all syscalls */
    fork_install(idt_base_p);
    exec_install(idt_base_p);
    wait_install(idt_base_p);
    yield_install(idt_base_p);
    deschedule_install(idt_base_p);
    make_runnable_install(idt_base_p);
    gettid_install(idt_base_p);
    new_pages_install(idt_base_p);
    remove_pages_install(idt_base_p);
    sleep_install(idt_base_p);
    getchar_install(idt_base_p);
    readline_install(idt_base_p);
    print_install(idt_base_p);
    set_term_color_install(idt_base_p);
    set_cursor_pos_install(idt_base_p);
    get_cursor_pos_install(idt_base_p);
    thread_fork_install(idt_base_p);
    get_ticks_install(idt_base_p);
    misbehave_install(idt_base_p);
    halt_install(idt_base_p);
    task_vanish_install(idt_base_p);
    set_status_install(idt_base_p);
    vanish_install(idt_base_p);
    readfile_install(idt_base_p);
    swexn_install(idt_base_p);

    report_progress(tag, "installing syscall done!");
}
