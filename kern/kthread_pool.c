/** @file kern/kthread_pool.c
 *
 *  @brief kernel thread implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */
#include <if_flag.h>
#include <kthread_pool.h>
#include <syscall.h>
#include <loader.h>
#include <reporter.h>
#include <asm.h>
#include <sched.h>

st_queue available_kthreads;

static char *tag = "kthread_pool";

int kthr_init(void)
{
    available_kthreads = st_queue_new();
    if (available_kthreads == NULL) {
        report_error(tag, "kthr_init: allocating ktcb queue failed");
        return -1;
    }

    ktcb_t *ktcb;
    reg_t *regs;
    void *esp0_lo;

    /* initialize 10 kernel threads into the available pool */
    int i;
    for (i = 0; i < KTHREAD_POOL_SIZE; i++) {
        ktcb = calloc(1, sizeof(ktcb_t));
        regs = calloc(1, sizeof(reg_t));

        esp0_lo = smemalign(PAGE_SIZE, PAGE_SIZE);
        if (ktcb == NULL || regs == NULL || esp0_lo == NULL) {
            return -1;
        }

        report_progress(tag, "kthr_init: ktcb = %p", ktcb);
        report_progress(tag, "esp0: %p", esp0_lo + PAGE_SIZE);
        
        regs->esp0 = (unsigned long)esp0_lo + PAGE_SIZE;
        ktcb->regs = regs;
        ktcb->blocked_mutex = NULL;
        
        int set = if_disable();
        if (st_enqueue(&(ktcb->k_n),ktcb, available_kthreads) < 0) {
            if_recover(set);
            report_error(tag, "fail to enqueue ktcb");
            return -1;
        }
        if_recover(set);
    }

    return 0;
}

ktcb_t *kthr_alloc(void)
{
    int set = if_disable();
    ktcb_t *ktcb = (ktcb_t *)st_dequeue(available_kthreads);
    if_recover(set);
    
    /* no available kernel threads, create a new one */
    if (ktcb == NULL) {
        ktcb = calloc(1, sizeof(ktcb_t));
        reg_t *regs = calloc(1, sizeof(reg_t));
        void *esp0_lo = smemalign(PAGE_SIZE, PAGE_SIZE);

        if (ktcb == NULL || regs == NULL || esp0_lo == 0) {
            report_error(tag, "ktcb_alloc: allocate new ktcb failed");
            return NULL;
        }

        regs->esp0 = (unsigned long)esp0_lo + PAGE_SIZE;
        ktcb->regs = regs;
    }
    
    ktcb->blocked_mutex = NULL;
    return ktcb;
}

void kthr_free(ktcb_t *ktcb)
{
    if (ktcb == NULL)  {
        report_error(tag, "kthr_free: ktcb is NULL");
        return;
    }

    else {
        int set = if_disable();
        st_enqueue(&(ktcb->k_n),(void *)ktcb, available_kthreads);
        if_recover(set);

        return;
    }
}

void kthr_build_relation(ktcb_t *ktcb, tcb_t *tcb) {
    
    if ((ktcb == NULL) || (tcb == NULL)) {
        report_error(tag, "kthr_build_relation: arg NULL");
        return;
    }
    
    ktcb->tcb = tcb;
    tcb->ktcb = ktcb;
}
