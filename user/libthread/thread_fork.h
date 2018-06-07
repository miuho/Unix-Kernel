/** @file thread_fork.h
 *
 *  @brief This file defines thread_fork.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu  (anwu@andrew.cmu.edu)
 */

/**
 * @brief fork a new thread.
 *
 * @param child_thr The child thread's thr_key.
 *
 * @return 0 to child thread, tid to parent thread, negative number if failed.
 */
int thread_fork(void *child_stack_base, int thr_key);
