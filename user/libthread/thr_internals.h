/** @file thr_internals.h
 *
 *  @brief This file may be used to define things
 *         internal to the thread library.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

#include <thread.h>
#include <stddef.h>
#include <stdio.h>
#include <syscall.h>
#include <mutex.h>
#include <thread.h>
#include <cond.h>
#include <malloc.h>
#include <simics.h>

#include <queue.h>
#include <autostack_private.h>
#include <splay_tree.h>
#include "thread_fork.h"
#include "get_esp.h"
#include "malloc_internal.h"
#include "asm.h"

#define EXN_STACK_SIZE PAGE_SIZE
#define FAULT_SIZE PAGE_SIZE
#define PAGE_ALIGN_MASK 0xFFFFF000
#define ESP_ALIGN_MASK 0xFFFFFFFC

splay_tree thr_splay_tree;  /* the splay tree to store thread data */
mutex_t st_mutex;           /* the mutex for splay tree */
queue exited_thr_queue;     /* a queue of not-in-use stack bases */
mutex_t queue_mutex;        /* the mutex for queue */
unsigned int thr_stack_size;/* each thread (except root thread) stack size */
void *lowest_stack_lo;      /* records the lowest thread stack_lo address */
int key_count;
int before_thr_create;  /* indicate in multithreading or not */

typedef enum {
    NORMAL,     /* after creation & before exit */
    EXITED     /* after exit and before joined */
} thread_status;

typedef struct thread_info {
    int tid;                /* the real tid */
    thread_status status;   /* the status of the thread */
    void *stack_base;       /* esp of the new thread's stack */
    void *exit_status;      /* the exit status of the thread */
    void *(*func)(void *);  /* the function that thread should run */
    void *args;             /* the arguments to the function */
    mutex_t status_mp;      /* the status mutex */
    cond_t status_cv;       /* the conditional var for thr_join */
} *thread_info;

/* forward declaration for current_stack_base */
void *current_stack_base(void *esp);


/** @brief an lprintf debug function
 *
 *  @return Void
 */
void print_debug();

/*
 * @brief Compare the keys given for the splay tree.
 *
 * @param key_1 The first key
 * @param key_2 The second key
 *
 * @return 1 if key_1 > key_2, -1 if key_2 > key_1, 0 if equal.
 */
int compare_tid(int key_1, int key_2);

/*
 * @brief Find the current thread's stack hi, aka thread base.
 *
 * @param esp The current thread stack's esp.
 *
 * @return The thread's stack base.
 */
void *current_stack_base(void *esp);

/** @brief exception handler for threads
 *
 *  @param arg the argument of the exception handler
 *  @param ureg the ureg_t address of the exception handler
 */
void exception_handler(void *arg, ureg_t *ureg);

/*
 * @brief Allocate memory for child thread stack space.
 *
 * @return The child thread stack base pointer.
 */
void *alloc_child_stack();

/*
 * @brief Generate a unqie thread key for the splay tree.
 *
 * @return The new key for child thread.
 */
int generate_thr_key();

/** @brief Setup the child after stack change and run its task
  *
  * @param thr_key the child's thr_key
  * @return Void. This function shouldn't return
  */
void setup_and_run_child(int thr_key);

#endif /* THR_INTERNALS_H */
