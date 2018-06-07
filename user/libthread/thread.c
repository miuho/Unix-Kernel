/** @file thread.c
 *
 *  @brief the P2 thread library
 *
 *  @author Hingon Miu (hmiu)
 *  @author An Wu (anwu) 
 * */

#include "thr_internals.h"

/*
 * @brief Intialize the thread library routine.
 *
 * @param size The size of each thread stack space.
 *
 * @return 0 if succeed, -1 for error.
 */
int thr_init(unsigned int size)
{
    /* initialize malloc mutex */
    malloc_init_mp();

    if((thr_splay_tree = st_new((st_compare_fn) compare_tid)) == NULL) {
        /* fail to initialize splay tree to store running thread info */
        return -1;
    }

    else if ((exited_thr_queue = queue_new()) == NULL) {
        /* fail to initialize queue to store dead thread info */
        return -1;
    }

    else {
        /* initialize splay tree mutex and queue mutex */
        mutex_init(&st_mutex);
        mutex_init(&queue_mutex);

        /* page align each thread stack size */
        thr_stack_size = (size + PAGE_SIZE - 1) & PAGE_ALIGN_MASK;

        /* store root thread info to splay tree as well */
        key_count = 1;
        thread_info thr_info = calloc(1, sizeof(struct thread_info));
        if (thr_info == NULL) {
            /* no heap memory left */
            return -1;
        }

        thr_info->tid = gettid();
        thr_info->stack_base = root_stack_hi;
        thr_info->status = NORMAL;
        mutex_init(&(thr_info->status_mp));
        cond_init(&(thr_info->status_cv));
        thr_info->exit_status = NULL;
        thr_info->func = NULL;
        thr_info->args = NULL;

        st_insert(thr_splay_tree, key_count, thr_info);
        
        return 0;
    }
}

/*
 * @brief Create a new thread under the multi-threading mode.
 *
 * @param func The function to be called in the new child thread.
 * @param args The arguments for the function.
 *
 * @return 0 if succeed, -1 for error.
 */
int thr_create(void *(*func)(void *), void *args)
{
    if (xchg(&before_thr_create, 0)) {
        /* root stack stops auto-growth */
        /* free the heap allocated exception stack */
        free(exn_stack_base);
        exn_stack_base = NULL;
        root_stack_lo -= FAULT_SIZE + EXN_STACK_SIZE;
        
        if (new_pages(root_stack_lo, EXN_STACK_SIZE) < 0) {
            return -1;
        }
     
        /* register new handler for root thread in multi-threading mode */
        swexn(root_stack_lo + EXN_STACK_SIZE, &exception_handler, NULL, NULL);
        lowest_stack_lo = root_stack_lo;
    }

    /* allocate a thread info structure for the child thread */
    thread_info thr_info = calloc(1, sizeof(struct thread_info));
    if (thr_info == NULL) {
        /* no heap memory left */
        return -1;
    }

    /* need to align child stack pointer to 4 */
    void *child_stack_base = alloc_child_stack(thr_info);
    if (child_stack_base == NULL) {
        /* no page memory left */
        return -1;
    }
    child_stack_base =
        (void *)((unsigned long)child_stack_base & ESP_ALIGN_MASK);

    /* generate next thread key */
    int thr_key = generate_thr_key();

    /* initialize struct fields */
    mutex_init(&(thr_info->status_mp));
    cond_init(&(thr_info->status_cv));
    thr_info->stack_base = child_stack_base;
    thr_info->status = NORMAL;
    thr_info->func = func;
    thr_info->args = args;

    /* insert child thread info */
    mutex_lock(&st_mutex);
    st_insert(thr_splay_tree, thr_key, thr_info);
    mutex_unlock(&st_mutex);

    int tid;
    if ((tid = thread_fork(child_stack_base, thr_key)) > 0) {
        /* parent thread */
        /* store the child thread info */
        thr_info->tid = tid;
        return thr_key;
    }

    else if (tid == 0) {
        /* child thread should return to setup_and_run_child */
        return -1;
    }

    else {
        /* thread_fork failed */
        return -1;
    }
}

/*
 * @brief The calling thread join on the target thread.
 *
 * @param tid The target thread key in splay tree.
 * @param statusp The memory address to store the exited thread's exit status.
 *
 * @return 0 if succeed, -1 for error.
 */
int thr_join(int tid, void **statusp)
{
    int target_thr_key = tid;
    mutex_lock(&st_mutex);
    thread_info target_thr_info = st_lookup(thr_splay_tree, target_thr_key);
    mutex_unlock(&st_mutex);

    if (target_thr_info == NULL) {
        /* the target thread does not exist */
        return -1;
    }
    
    mutex_lock(&(target_thr_info->status_mp));
    
    /* the target thread was not exited */
    if (target_thr_info->status == NORMAL) {
        /* suspend the calling thread until the target thread is exited */
        cond_wait(&(target_thr_info->status_cv),&(target_thr_info->status_mp));
    }
    
    mutex_unlock(&(target_thr_info->status_mp));

    if (statusp != NULL) {
        *statusp = target_thr_info->exit_status;
    }  
    
    st_delete(thr_splay_tree, target_thr_key);
    
    /* destroy the mutex and cv */
    mutex_destroy(&(target_thr_info->status_mp));
    cond_destroy(&(target_thr_info->status_cv));
    
    free(target_thr_info);

    return 0;
}

/*
 * @brief Exit the calling thread with given exit status.
 *
 * @param status The exit status.
 */
void thr_exit(void *status)
{
    /* remove the thread info structure from splay tree */
    mutex_lock(&st_mutex);
    thread_info thr_info = st_lookup(thr_splay_tree, thr_getid());
    mutex_unlock(&st_mutex);
    if (thr_info == NULL) {
        return;
    }
    void *stack_base = thr_info->stack_base;
    /* mark the calling thread as exited */
    mutex_lock(&(thr_info->status_mp));
    thr_info->status = EXITED;
    thr_info->exit_status = status;
    /* signal the waiting thread */
    cond_signal(&(thr_info->status_cv));
    mutex_unlock(&(thr_info->status_mp));
    
    /* store the stack_base for later reuse */
    mutex_lock(&queue_mutex);
    enqueue(stack_base, exited_thr_queue);
    mutex_unlock(&queue_mutex);
    /* terminate this thread */
    vanish();
}



/*
 * @brief Get the splay tree key for the calling thread.
 *
 * @return The thread's splay tree key.
 */
int thr_getid(void)
{
    /* find the thr_key from stack */
    void *esp = get_esp();
    if ((unsigned long)esp >= ((unsigned long)root_stack_lo - FAULT_SIZE)) {
        return 1;
    }
    else {
        return *((int *)current_stack_base(esp));
    }
}

/*
 * @brief Defers execution of the invoking thread to a later time in favor
 *        of the thread with the given splay tree key.
 *
 * @param tid The thread's splay tree key which the invoking thread yields to.
 *
 * @return 0 if succeed, -1 if the given tid's corresponding thread is not
 *         runnable or doesnt exist.
 */
int thr_yield(int tid)
{
    if (tid == -1) {
        return yield(-1);
    }

    int thr_key = tid;
    /* ensure the thread exists */
    mutex_lock(&st_mutex);
    thread_info thr_info = st_lookup(thr_splay_tree, thr_key);
    mutex_unlock(&st_mutex);

    if (thr_info == NULL) {
        return -1;
    }
    return yield(thr_info->tid);
}
