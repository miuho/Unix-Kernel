/** @file kern/common_wrapper.h
 *  @brief the trap handler wrappers
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_COMMON_WRAPPER_H_
#define _KERN_INC_COMMON_WRAPPER_H_

/** @brief the fork trap handler wrapper 
 *
 *  @return Void
 */
void fork_wrapper();

/** @brief the exec trap handler wrapper 
 *
 *  @return Void
 */
void exec_wrapper();

/** @brief the wait trap handler wrapper 
 *
 *  @return Void
 */
void wait_wrapper();

/** @brief the yield trap handler wrapper 
 *
 *  @return Void
 */
void yield_wrapper();

/** @brief the deshecule trap handler wrapper 
 *
 *  @return Void
 */
void deschedule_wrapper();

/** @brief the make_runnable trap handler wrapper 
 *
 *  @return Void
 */
void make_runnable_wrapper();

/** @brief the gettid trap handler wrapper 
 *
 *  @return Void
 */
void gettid_wrapper();

/** @brief the new_pages trap handler wrapper 
 *
 *  @return Void
 */
void new_pages_wrapper();

/** @brief the remove_pages trap handler wrapper 
 *
 *  @return Void
 */
void remove_pages_wrapper();

/** @brief the sleep trap handler wrapper 
 *
 *  @return Void
 */
void sleep_wrapper();

/** @brief the getchar trap handler wrapper 
 *
 *  @return Void
 */
void getchar_wrapper();

/** @brief the readline trap handler wrapper 
 *
 *  @return Void
 */
void readline_wrapper();

/** @brief the print trap handler wrapper 
 *
 *  @return Void
 */
void print_wrapper();

/** @brief the set_term_color trap handler wrapper 
 *
 *  @return Void
 */
void set_term_color_wrapper();

/** @brief the set_cursor_pos trap handler wrapper 
 *
 *  @return Void
 */
void set_cursor_pos_wrapper();

/** @brief the get_cursor_pos trap handler wrapper 
 *
 *  @return Void
 */
void get_cursor_pos_wrapper();

/** @brief the thread_fork trap handler wrapper 
 *
 *  @return Void
 */
void thread_fork_wrapper();

/** @brief the get_ticks trap handler wrapper 
 *
 *  @return Void
 */
void get_ticks_wrapper();

/** @brief the misbehave trap handler wrapper 
 *
 *  @return Void
 */
void misbehave_wrapper();

/** @brief the halt trap handler wrapper 
 *
 *  @return Void
 */
void halt_wrapper();

/** @brief the task_vanish trap handler wrapper 
 *
 *  @return Void
 */
void task_vanish_wrapper();

/** @brief the set_status trap handler wrapper 
 *
 *  @return Void
 */
void set_status_wrapper();

/** @brief the vanish trap handler wrapper 
 *
 *  @return Void
 */
void vanish_wrapper();

/** @brief the readfile trap handler wrapper 
 *
 *  @return Void
 */
void readfile_wrapper();

/** @brief the swexn trap handler wrapper 
 *
 *  @return Void
 */
void swexn_wrapper();

#endif /* !_COMMON_WRAPPER_H */
