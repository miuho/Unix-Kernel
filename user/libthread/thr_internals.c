/** @file thr_internals.c
 *
 *  @brief This file may be used to define things
 *         internal to the thread library.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <thr_internals.h>

int before_thr_create = 1;

void print_debug() {
    void *esp = get_esp();
    void *stack_base = current_stack_base(esp);
    int thr_key = *((int *)stack_base);
    lprintf("thr_key: %d\n", thr_key);

    mutex_lock(&st_mutex);
    thread_info thr_info = st_lookup(thr_splay_tree, thr_key);
    mutex_unlock(&st_mutex);

    if (thr_info == NULL) {
        return;
    }

    lprintf("tid: %d\n", thr_info->tid);
    lprintf("stack_base: %p\n", thr_info->stack_base);
    lprintf("exit_status: %p\n", thr_info->exit_status);
    if (thr_info->status == NORMAL) {
        lprintf("status: NORMAL\n");
    }
    else {
        lprintf("status: EXITED\n");
    }
    
    return;
}

int compare_tid(int key_1, int key_2)
{
    if (key_1 == key_2)
        return 0;
    else if (key_1 > key_2)
        return 1;
    else
        return -1;
}

void *current_stack_base(void *esp)
{
    unsigned long block_size = FAULT_SIZE + EXN_STACK_SIZE + thr_stack_size;
    int num_blocks = 
        (unsigned long)(root_stack_lo - FAULT_SIZE - esp) / block_size;
    return (void *)((unsigned long)root_stack_lo - FAULT_SIZE -  
            num_blocks * block_size - EXN_STACK_SIZE - 4); /* esp 4 aligned */
}

void exception_handler(void *arg, ureg_t *ureg)
{
    int thr_key;
    void *esp = (void *)ureg->esp;
    if ((unsigned long)esp >= ((unsigned long)root_stack_lo - FAULT_SIZE)) {
        thr_key = 1;
    }
    else {
        thr_key = *((int *)current_stack_base(esp));
    }

    /* remove the thread info structure from splay tree */
    mutex_lock(&st_mutex);
    thread_info thr_info = st_lookup(thr_splay_tree, thr_key);
    mutex_unlock(&st_mutex);
    if (thr_info == NULL) {
        return;
    }
    void *stack_base = thr_info->stack_base;
    /* mark the calling thread as exited */
    mutex_lock(&(thr_info->status_mp));
    thr_info->status = EXITED;
    thr_info->exit_status = NULL;
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

void *alloc_child_stack()
{
    /* previously terminated thread stack was enqueued, and can be reused */
    mutex_lock(&queue_mutex);
    void *child_stack_base = dequeue(exited_thr_queue);
    mutex_unlock(&queue_mutex);

    if (child_stack_base != NULL) {
        return child_stack_base;
    }
    else {
        /* allocate new page for exception stack, skip page fault region */
        void *exn_stack_lo = lowest_stack_lo - FAULT_SIZE - EXN_STACK_SIZE;
        if (new_pages(exn_stack_lo, EXN_STACK_SIZE) < 0) {
            /* no available page left */
            return NULL;
        }

        void *child_stack_lo = exn_stack_lo - thr_stack_size;
        if (new_pages(child_stack_lo, thr_stack_size) < 0) {
            /* no available page left */
            return NULL;
        }

        /* updates the lowest thread stack_lo */
        lowest_stack_lo = child_stack_lo;

        return child_stack_lo + thr_stack_size - 1;
    }
}

int generate_thr_key()
{
    int new_key;
    /* ensure the key is unique */
    do {
        new_key = xadd(&key_count, 1);
    } while (st_lookup(thr_splay_tree, new_key) != NULL || new_key == 0);

    return new_key;
}

void setup_and_run_child(int thr_key) {
    mutex_lock(&st_mutex);
    thread_info child_thr_info = st_lookup(thr_splay_tree, thr_key);
    mutex_unlock(&st_mutex);

    if (child_thr_info == NULL)
        thr_exit(NULL);    /* can't find thr info */
    
    /* register handler for new thread in multi-threading mode */
    swexn(child_thr_info->stack_base + EXN_STACK_SIZE, &exception_handler, 
            NULL, NULL);
    
    void *exit_status = child_thr_info->func(child_thr_info->args);
    thr_exit(exit_status);
}

