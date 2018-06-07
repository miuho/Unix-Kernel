/** @file tcb.c
 *
 *  @brief thread control block implementation
 *
 *  @author HingOn Miu (hmiu)
 *  @author An Wu (anwu)
 */

#include <tcb.h>
#include <common_kern.h>
#include <malloc.h>
#include <stdio.h>
#include <loader.h>
#include <kthread_pool.h>
#include <sched.h>
#include <x86/asm.h>
#include <if_flag.h>
#include <reporter.h>

static char *tag = "tcb";

int key_compare_tid(ht_entry_t *e1, ht_entry_t *e2)
{
    return (tid_t)(e1->key) == (tid_t)(e2->key);
}

void *tcb_create(pcb_t *pcb, ht_t *tcb_ht, reg_t *regs, int tcb_count,
                ktcb_t *ktcb) 
{
    if (regs == NULL) {
        report_error(tag, "tcb_create: get NULL regs");
        return NULL;
    }

    tcb_t *tcb;
    if ((tcb = calloc(1, sizeof(tcb_t))) == NULL) {
        report_error(tag, "tcb_create: can't allocate tcb");
        return NULL;
    }

    tcb->regs = regs;
    tcb->pcb = pcb;
    tcb->tid = tcb_count;
    tcb->state = TCB_RUNNING;

    /* insert tcb into pcb's tcb splay tree */
    mutex_lock(&(tcb_ht->mp));
    ht_insert(tcb_ht, tcb->tid, tcb);
    kthr_build_relation(ktcb, tcb);
    mutex_unlock(&(tcb_ht->mp));

    return tcb;
}

tcb_t *get_root_thr(pcb_t *pcb) {

    if (pcb == NULL) {
        report_error(tag, "get_root_thr: get NULL pcb");
        return NULL;
    }

    return ht_lookup(pcb->tcb_ht, (hash_key)pcb->pid);
}

int tcb_free(ht_entry_t *e)
{
    if (e == NULL) {
        report_error(tag, "tcb_free: arg NULL");
        return 0;
    }
    
    tcb_t *tcb = (void *)e->value;
    if (tcb == NULL) {
        report_error(tag, "tcb_free: tcb NULL");
        return 0;
    }
    
    free(tcb->regs);
    free(tcb);
    return 1;
}

/* make tcb status EXITED and free its ktcb */
int kill_tcb(ht_entry_t *e) {

    report_progress(tag, "kill_tcb: entry");

    if (e == NULL) {
        report_error(tag, "kill_tcb: e is NULL");
        return -1;
    }

    tcb_t *child = (void *)e->value;
    if (e == NULL) {
        report_error(tag, "kill_tcb: child is NULL");
        return -1;
    }

    if (child == running_ktcb->tcb) {
        /* don't kill itself yet */
        return -1;
    }

    report_progress(tag, "kill_tcb: checking %p", child->ktcb);
    int if_was_set = if_disable();

    if (child->state != TCB_EXITED) {
        /* kill the thread */
        ktcb_t *ktcb = child->ktcb;

        sched_delete(ktcb);
        child->state = TCB_EXITED;
        kthr_free(ktcb);

        report_progress(tag, "kill_tcb: killed %p", ktcb);
    }

    if_recover(if_was_set);
    return 0;
}
