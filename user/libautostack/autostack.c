/** @file user/libautostack/autostack.c
 *
 *  @brief register exn handler (for autostack growth) for root thread
 *
 *  @author HingOn Miu (hmiu@andrew.cmu.edu) 
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall.h>
#include <ureg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <simics.h>

#define PAGE_ALIGN_MASK 0xFFFFF000
#define SINGLE_THREAD_EXN_STACK_SIZE 0x100        /* 0.25 KB */
#define ESP_ALIGN_MASK  0xFFFFFFFC

void *exn_stack_base;       /* indicate the start of exception stack */
void *root_stack_hi;        /* indicate the high of root stack */
void *root_stack_lo;        /* indicate the low of root stack */

/*
 * @brief Allocate pages to the root thread stack up to the addr given.
 *
 */
int alloc_aligned_pages(void *addr)
{
    /* allocate new page until the page fault address is covered */
    while ((unsigned long)root_stack_lo >= (unsigned long)addr) {
        root_stack_lo -= PAGE_SIZE;
        if (new_pages(root_stack_lo, PAGE_SIZE) < 0) {
            /* no available page left */
            return -1;
        }
    }

    /* success */
    return 0;
}

/*
 * @brief Handler to catch page faults to allow auto-stack growth for
 *        single-threading mode.
 *
 */
void autostack_handler(void *arg, ureg_t *ureg)
{
    if (ureg == NULL) {
        /* should not dereference a NULL ureg. Retry */
        swexn(exn_stack_base, &autostack_handler, NULL, ureg);
        return;
    }

    else if (ureg->cause != SWEXN_CAUSE_PAGEFAULT ||
             (void *)ureg->esp > root_stack_lo) {
        /* only handles autostack growth */
        return;
    }

    else {
        /* stores new stack_lo for root thread */
        if ((alloc_aligned_pages((void *)ureg->esp)) != 0) {
            /* can't allocate more space */
            return;
        }

        /* re-register the page fault exception handler */
        swexn(exn_stack_base, &autostack_handler, NULL, ureg);
        return;
    }
}

/*
 * @brief Install the exception handler for the root thread, and store the
 *        root thread information globally.
 *
 */
void install_autostack(void *stack_high, void *stack_low)
{
    /* makes the root_stack_lo page aligned */
    root_stack_lo = (void *)((unsigned int)stack_low & PAGE_ALIGN_MASK);

    if (stack_high == NULL || stack_low == NULL ||
        stack_high <= stack_low || root_stack_lo != stack_low) {
        /* invalid inputs */
        return;
    }


    else {
        /* it must be single thread to begin with */
        root_stack_hi = stack_high;
        
        exn_stack_base = _malloc(SINGLE_THREAD_EXN_STACK_SIZE) + 
                        SINGLE_THREAD_EXN_STACK_SIZE - 1;

        /* make sure stack base is 4-aligned */
        exn_stack_base = 
            (void*)((unsigned int)exn_stack_base & ESP_ALIGN_MASK);

        /* if exn_stack_base is NULL, exception_handler wouldn't be */
        /* registered anyway */
        swexn(exn_stack_base, &autostack_handler, NULL, NULL);
        return;
    }
}
