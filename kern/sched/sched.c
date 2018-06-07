/*
 * @file kern/sched/sched.c 
 * @brief the implementation a round-robin scheduler.
 *
 * @author HingOn Miu (hmiu)
 * @author An Wu (anwu)
 *
 */

#include <sched.h>
#include <queue.h>
#include <simics.h>
#include <x86/asm.h>
#include <x86/eflags.h>
#include <hash.h>
#include <vm.h>
#include <loader.h>
#include <context_switch.h>
#include <timed_queue.h>
#include <reporter.h>
#include <if_flag.h>

/* the runnable queue of KTCB */
st_queue runnable_ktcbs;
/* the descheduled hash table of KTCB */
sht_t *waiting_ktcbs;
/* all processes' PCB */
sht_t *pcbs_sht;
/* the scheduler's KTCB */
ktcb_t *sched_ktcb;

static char *tag = "sched";

/**
 * @brief compare two hash table entries for their key's
 *        pointer value
 *
 * @param e1 the first entry
 * @param e2 the second entry
 * @return 1 if identical,0 otherwise.
 */
int key_compare_ptr(sht_entry_t *e1, sht_entry_t *e2) {
    return (void *)(e1->key) == (void *)(e2->key);
}

/**
 * @brief compare two KTCB
 *
 * @param ktcb the first KTCB
 * @param data the second KTCB
 * @return 1 if identical,0 otherwise.
 */
int is_equal_ktcb(void *ktcb, void *data) {
    return (((ktcb_t *)ktcb) == ((ktcb_t *)data));
}

/**
 * @brief compare two tid
 *
 * @param tid the first tid
 * @param data the second tid
 * @return 1 if identical,0 otherwise.
 */
int is_equal_tid(void *tid, void *data) {
    return ((int)tid == ((ktcb_t *)data)->tcb->tid);
}

int sched_pcbs_sht_init(void)
{
    pcbs_sht = sht_new((skey_compare_fn)key_compare_pid);
    if (pcbs_sht == NULL) {
        report_error(tag, "sched_locks_init: can't allocate pcbs_sht");
        return -1;
    }   
    return 0;
}

int sched_init(void)
{
    runnable_ktcbs = st_queue_new();
    if (runnable_ktcbs == NULL) {
        report_error(tag, "sched_init: can't allocate runnable_ktcb");
        return -1;
    }

    waiting_ktcbs = sht_new((skey_compare_fn)key_compare_tid);
    if (waiting_ktcbs == NULL) {
        report_error(tag, "sched_init: can't allocate waiting_ktcb");
        st_queue_destroy(runnable_ktcbs);
        return -1;
    }
    
    if (tq_init() != 0) {
        report_error(tag, "sched_init: cant tq_init");
        sht_destroy(waiting_ktcbs);
        st_queue_destroy(runnable_ktcbs);
        return -1;
    }

    reg_t *sched_regs = calloc(1, sizeof(reg_t));
    if (sched_regs == NULL) {
        report_error(tag, "sched_init: reg calloc failed");
        sht_destroy(waiting_ktcbs);
        st_queue_destroy(runnable_ktcbs);
        return -1;
    }

    sched_ktcb = kthr_alloc();
    if (sched_ktcb == NULL) {
        report_error(tag, "sched_init: kthr_alloc failed");

        free(sched_regs);
        sht_destroy(waiting_ktcbs);
        st_queue_destroy(runnable_ktcbs);
        return -1;
    }

    pcb_t *sched_pcb = pcb_create((unsigned long)kern_pgd,
                                  sched_regs, NULL, sched_ktcb,
                                  NULL, 0, NULL, 0);

    if (sched_pcb == NULL) {
        report_error(tag, "sched_init: pcb create failed");

        kthr_free(sched_ktcb);
        free(sched_regs);
        sht_destroy(waiting_ktcbs);
        st_queue_destroy(runnable_ktcbs);
        return -1;
    }

    running_ktcb = sched_ktcb;

    report_progress(tag, 
            "\n#####\nsched_ktcb = %p, tcb = %p, pcb = %p\n#####", sched_ktcb,
            sched_ktcb->tcb, sched_ktcb->tcb->pcb);

    return 0;
}

void sched_add_pcb(pcb_t* pcb) {

    int set = if_disable();
     
    sht_insert(&(pcb->e), &(pcb->n), pcbs_sht, 
                (st_hash_key)(pcb->pid), (st_hash_value)pcb);

    if_recover(set); 
    return;
}

int sched_remove_pcb(pcb_t *pcb) {
    
    int set = if_disable();

    if (sht_delete(pcbs_sht, (st_hash_key)(pcb->pid)) == NULL) {
        
        if_recover(set);
        report_error(tag, "sched_remove_pcb: input lock cannot be found");
        return -1;
    }
    
    if_recover(set);
    return 0;
}

pcb_t *sched_find_pcb(int pid) {
    
    int set = if_disable();
     
    pcb_t *pcb = (pcb_t *)sht_lookup(pcbs_sht, (st_hash_key)pid);
    
    if_recover(set);
    return pcb;
}

int sched_delete(ktcb_t *ktcb) {
    int if_was_set = if_disable();

    if (sched_runnable_to_running(ktcb->tcb->tid) != NULL) {
        if_recover(if_was_set);
        return 0;
    }    

    if (sched_waiting_to_running(ktcb->tcb->tid) != NULL) {
        if_recover(if_was_set);
        return 0;
    }
    
    if (tq_delete(ktcb) != 0) {
        if_recover(if_was_set);
        return 0;
    }
    
    if_recover(if_was_set); 
    return -1;
}

void sched_run() {

    pcb_t *child;
    pcb_t *pcb = sched_ktcb->tcb->pcb;

    while (1) {

        /* repeated reap child whose parents are dead already,
         * else wait */
        mutex_lock(&(pcb->children->mp));
        if ((child = (pcb_t *)ht_find(sched_ktcb->tcb->pcb->children,
                                        pcb_all_thr_exited)) == NULL) {
            /* no exited child available */
            report_progress(tag, "sched_run: going to wait for exited child");
            cond_wait(&(pcb->wait_cond), &(pcb->children->mp));
            report_progress(tag, "sched_run: signaled by exited child");
            
            mutex_unlock(&(pcb->children->mp));   
        }
        else {
            /* exited child found. reap it */
            if (ht_delete(pcb->children, child->pid) == NULL) {
                mutex_unlock(&(pcb->children->mp));
                report_error(tag, "sched_run: cannot delete exited child");
                continue;
            }

            mutex_unlock(&(pcb->children->mp));

            /* do the cleanup */
            report_progress(tag, 
                        "sched_run: going to cleanup child resource...");
            pcb_free(child);
        }
    }
}

void sched_add_child(pcb_t *child) {

    pcb_t *pcb = sched_ktcb->tcb->pcb;

    mutex_lock(&(pcb->children->mp));
    ht_insert(pcb->children, child->pid, (void *)child);
    child->parent = pcb;
    mutex_unlock(&(pcb->children->mp));
}

ktcb_t *sched_next() {

    ktcb_t *ktcb;
    int if_set = if_disable();
    
    /* check sleep ktcb */
    if ((ktcb = sched_sleep_to_running()) != NULL) {
        if_recover(if_set);
        report_misc(tag, "sched_next get %p from sleep", ktcb);
        return ktcb;
    }

    /* check runnable ktcb */
    if ((ktcb = sched_runnable_to_running(-1)) != NULL) {
        if_recover(if_set);
        report_misc(tag, "sched_next get %p from runnable", ktcb);
        return ktcb;
    }

    if_recover(if_set);
    return sched_ktcb;
    
}

ktcb_t *sched_runnable_to_running(int tid)
{
    int if_was_set = if_disable();

    if (tid == sched_ktcb->tcb->tid) {
        if_recover(if_was_set);
        report_error(tag, 
                    "sched_runnable_to_running sched_ktcb incorrectly used");
        return NULL;
    }
     
    ktcb_t *ktcb;
    
    if (tid == -1) {
        if ((ktcb = st_dequeue(runnable_ktcbs)) == NULL) {
            if_recover(if_was_set);

            report_warning(tag,
                "sched_runnable_to_running: runnable_ktcbs is empty");
            return NULL;
        }
    }
    else {
        if ((ktcb = (ktcb_t *)st_queue_delete(is_equal_tid, (void *)tid,
                    runnable_ktcbs)) == NULL) {
            if_recover(if_was_set);

            report_warning(tag, 
            "sched_runnable_to_running input tid=%d was not runnable", tid);
            return NULL;
        }
    }
    
    if (ktcb == sched_ktcb) {
        if_recover(if_was_set);

        report_error(tag, 
                "sched_runnable_to_running sched_ktcb incorrectly used");
        return NULL;
    }
    
    if_recover(if_was_set);
    return ktcb;
}

int sched_running_to_runnable(ktcb_t *ktcb)
{
    int if_was_set = if_disable();

    if (ktcb == sched_ktcb) {
        if_recover(if_was_set);

        report_error(tag, 
                    "sched_running_to_runnable sched_ktcb incorrectly used");
        return ARG_ERR;
    }

    if (st_queue_find(is_equal_ktcb, (void *)ktcb, runnable_ktcbs) != NULL) {
        if_recover(if_was_set);

        report_error(tag, 
       "sched_running_to_runnable input ktcb with tid=%d was already runnable",
                    ktcb->tcb->tid);
        return -1;
    }

    /* add ktcb to runnable */
    st_enqueue(&(ktcb->r_n), ktcb, runnable_ktcbs);
   
    if_recover(if_was_set);
    return 0;
}

int sched_running_to_sleep(ktcb_t *ktcb, unsigned int ticks)
{
    int if_was_set = if_disable();
    
    if (!tq_find(ktcb))
        tq_insert(&(ktcb->t_n), (void *)ktcb, ticks);
    else
        report_error(tag, "ktcb was already in sleep");
    
    if_recover(if_was_set);
    return 0;
}

ktcb_t *sched_sleep_to_running(void)
{
    int if_was_set = if_disable();

    ktcb_t *ktcb = (ktcb_t *)tq_get();
    
    if (ktcb != NULL) 
        report_misc(tag, "sched_sleep_to_running ktcb %p is now awaken", ktcb);

    if_recover(if_was_set);
    return ktcb;
}

int sched_running_to_waiting(ktcb_t *ktcb)
{
    int if_was_set = if_disable();

    if (ktcb == sched_ktcb) {
        if_recover(if_was_set);

        report_error(tag, 
            "sched_running_to_waiting sched_ktcb incorrectly used");
        return ARG_ERR;
    }

    if (sht_lookup(waiting_ktcbs, ktcb->tcb->tid) != NULL) {
        if_recover(if_was_set);

        report_error(tag, 
        "sched_running_to_waiting input ktcb with tid=%d was already waiting", 
            ktcb->tcb->tid);
        return -1;
    }
    
    /* add ktcb to waiting */
    sht_insert(&(ktcb->w_e), &(ktcb->w_n), waiting_ktcbs, ktcb->tcb->tid, 
                (void *)ktcb);

    if_recover(if_was_set);
    return 0;
}

ktcb_t *sched_waiting_to_running(int tid)
{
    int if_was_set = if_disable();

    if (tid == sched_ktcb->tcb->tid) {
        if_recover(if_was_set);

        report_error(tag, 
            "sched_waiting_to_running sched_ktcb incorrectly used");
        return NULL;
    }

    ktcb_t *ktcb;

    if ((ktcb = (ktcb_t *)sht_delete(waiting_ktcbs, tid)) == NULL) {
        if_recover(if_was_set);

        report_error(tag, 
            "sched_waiting_to_running input tid=%d was not waiting", tid);
        return NULL;
    }

    if (ktcb == sched_ktcb) {
        if_recover(if_was_set);

        report_error(tag, 
            "sched_waiting_to_running sched_ktcb incorrectly used");
        return NULL;
    }
    
    if_recover(if_was_set);
    return ktcb;
}

ktcb_t *sched_is_waiting(int tid)
{
    int if_was_set = if_disable();

    if (tid == sched_ktcb->tcb->tid) {
        if_recover(if_was_set);

        report_error(tag, 
                "sched_is_waiting sched_ktcb incorrectly used");
        return NULL;
    }

    ktcb_t *ktcb;
    if ((ktcb = (ktcb_t *)sht_lookup(waiting_ktcbs, tid)) == NULL) {
        if_recover(if_was_set);

        report_warning(tag, 
            "sched_is_waiting: input tid=%d was not waiting", tid);
        return NULL;
    }

    if (ktcb == sched_ktcb) {
        if_recover(if_was_set);

        report_error(tag, "sched_is_waiting: sched_ktcb incorrectly used");
        return NULL;
    }

    if_recover(if_was_set);
    return ktcb;
}


