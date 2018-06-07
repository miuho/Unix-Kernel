/** @file pcb.c
 *
 *  @brief process control block implementation
 *
 *  @author HingOn Miu (hmiu)
 *  @author An Wu (anwu)
 */

#include <pcb.h>
#include <common_kern.h>
#include <stdio.h>
#include <simics.h>
#include <malloc.h>
#include <tcb.h>
#include <reporter.h>
#include <asm.h>
#include <loader.h>
#include <sched.h>

int tcb_count;

static char *tag = "pcb";

int generate_tid()
{
    return xadd(&tcb_count, 1);
}

int key_compare_pid(ht_entry_t *e1, ht_entry_t *e2)
{
    return (pid_t)(e1->key) == (pid_t)(e2->key);
}

int key_compare_base(ht_entry_t *e1, ht_entry_t *e2)
{
    return (int)(e1->key) == (int)(e2->key);
}

int pcb_pool_init(void)
{
    tcb_count = 1;
    return 0;
}

int track_pages_allocated(pcb_t *pcb, void *base, int len)
{
    if (pcb == NULL) {
        report_error(tag, "track_pages_allocated: pcb is NULL");
        return -1;
    }
   
    mutex_lock(&(pcb->new_pages_ht->mp));

    /* sanity check */
    if (ht_lookup(pcb->new_pages_ht, (hash_key)base) != NULL) {
        mutex_unlock(&(pcb->new_pages_ht->mp));
        report_warning(tag, 
                        "track_pages_allocated: base was allocated before");
        return -1;
    }

    ht_insert(pcb->new_pages_ht, (hash_key)base, (hash_value)len);

    mutex_unlock(&(pcb->new_pages_ht->mp));

    return 0;
}

int was_pages_allocated(pcb_t *pcb, void *base)
{
    if (pcb == NULL) {
        report_error(tag, "was_pages_allocated: pcb is NULL");
        return -1;
    }
    
    int len;
    mutex_lock(&(pcb->new_pages_ht->mp));

    if ((len = (int)ht_lookup(pcb->new_pages_ht, (hash_key)base)) != 0) {
        mutex_unlock(&(pcb->new_pages_ht->mp));
        return len; 
    }
    
    mutex_unlock(&(pcb->new_pages_ht->mp));

    report_error(tag, "was_pages_allocated: base=%p not allocated before",
                base);
    
    return 0;
}

int untrack_pages_allocated(pcb_t *pcb, void *base)
{
    if (pcb == NULL) {
        report_error(tag, "untrack_pages_allocated: pcb is NULL");
        return -1;
    }   

    mutex_lock(&(pcb->new_pages_ht->mp));

    if (ht_lookup(pcb->new_pages_ht, (hash_key)base) == NULL) {
        report_error(tag, 
                    "untrack_pages_allocated: base was not allocated before");

        mutex_unlock(&(pcb->new_pages_ht->mp));
        return -1;  
    }   
    
    ht_delete(pcb->new_pages_ht, (int)base);

    mutex_unlock(&(pcb->new_pages_ht->mp));

    return 0;
}

pcb_t *pcb_create(unsigned long pgd, reg_t *regs, pcb_t *parent, ktcb_t *ktcb,
                  void *txt_base, int txt_len, void *rodat_base,
                  int rodat_len)
{
    /* allocate pcb */
    pcb_t *pcb;
    if ((pcb = calloc(1, sizeof(pcb_t))) == NULL) {
        report_error(tag, "pcb_create: pcb calloc gets NULL");
        return NULL;
    }

    pcb->pgd = pgd;

    pcb->parent = parent;
    pcb->exit_status = 0;
    pcb->exited_thread_count = 0;

    pcb->txt_base = txt_base;
    pcb->txt_len = txt_len;
    pcb->rodat_base = rodat_base;
    pcb->rodat_len = rodat_len;

    /* allocate children */
    if ((pcb->children = ht_new((key_compare_fn) key_compare_pid)) == NULL) {
        report_error(tag, 
                    "pcb_create: fail to create ht to store pcb children");
        free(pcb);
        return NULL;
    }

    /* allocate tcb_ht */
    if ((pcb->tcb_ht = ht_new((key_compare_fn) key_compare_tid)) == NULL) {
        report_error(tag, 
                "pcb_create: fail to create ht to store tcb");

        ht_destroy(pcb->children);
        free(pcb);
        return NULL;
    }
    
    /* allocate new_pages_ht */
    if ((pcb->new_pages_ht = ht_new((key_compare_fn) key_compare_base)) 
            == NULL) {
        report_error(tag, 
                "pcb_create: fail to initialize new pages ht");

        ht_destroy(pcb->tcb_ht);
        ht_destroy(pcb->children);
        free(pcb);
        return NULL; 
    }
    
    /* allocate wait_cond */
    if (cond_init(&(pcb->wait_cond)) != 0) {
        report_error(tag, "pcb_create: failed to init cond");

        ht_destroy(pcb->tcb_ht);
        ht_destroy(pcb->children);
        free(pcb);
        return NULL;
    }
    
    pcb->pid = generate_tid();

    /* allocate tcb */
    if (tcb_create(pcb, pcb->tcb_ht, regs, pcb->pid, ktcb) == NULL) {
        report_error(tag, "pcb_create: fail to create root tcb");

        cond_destroy(&(pcb->wait_cond));
        ht_destroy(pcb->tcb_ht);
        ht_destroy(pcb->children);
        free(pcb);
        return NULL;
    }
    
    sched_add_pcb(pcb);
    return pcb; 
}

int pcb_all_thr_exited(ht_entry_t *e)
{
    if (e == NULL) {
        report_error(tag, "pcb_all_thr_exited: arg NULL");
        return -1;
    }

    pcb_t *pcb = (void *)(e->value);
    if (pcb == NULL) {
        report_error(tag, "pcb_all_thr_exited: pcb NULL");
        return -1;
    }

    report_progress(tag, "pcb_all_thr_exited: checking process %d", 
                    ((pcb_t *)pcb)->pid);

    return pcb->exited_thread_count == ht_size(pcb->tcb_ht);
}

int announce_parent_death(ht_entry_t *e)
{
    if (e == NULL) {
        report_error(tag, "announce_parent_death: arg NULL");
        return -1;
    }
    
    void *pcb = (void *)e->value;
    if (pcb == NULL) {
        report_error(tag, "announce_parent_death: pcb NULL");
        return -1;
    }
     
    report_progress(tag, "announce_parent_death: entry, pcb=%p, pid=%d", 
                    pcb, ((pcb_t *)pcb)->pid); 
    
    sched_add_child(pcb);
    return 1;
}

pcb_t *pcb_update(pcb_t *pcb, unsigned long pgd, reg_t *regs, tid_t tid,
                  ktcb_t *ktcb,
                  void *txt_base, int txt_len, void *rodat_base, int rodat_len)
{
    /* check old pcb */
    if (pcb == NULL) {
        report_error(tag, "pcb passed to update is NULL");
        return NULL;
    }

    if (pcb->tcb_ht == NULL) {
        report_error(tag, "pcb_update: pcb's ht is NULL");
        return NULL;
    }

    if (pcb->new_pages_ht == NULL) {
        report_error(tag, "pcb_update: pcb's new_page_ht is NULL");
        return NULL;
    }

    if (pcb->children == NULL) {
        report_error(tag, "pcb_update: pcb's children is NULL");
        return NULL;
    }

    /* alloc new structures */
    /* alloc new new_pages_ht */
    ht_t *new_new_pages_ht = ht_new((key_compare_fn) key_compare_base);
    ht_t *old_new_pages_ht = pcb->new_pages_ht;
    if (new_new_pages_ht == NULL) {
        report_error(tag, "pcb_update: can't alloc new ht");

        return NULL;
    }

    /* alloc new tcb_ht */
    ht_t *new_tcb_ht = ht_new((key_compare_fn) key_compare_tid);
    ht_t *old_tcb_ht = pcb->tcb_ht;
    if (new_tcb_ht == NULL) {
        report_error(tag, "pcb_update: can't alloc new tcb_ht");

        ht_destroy(new_new_pages_ht);
        return NULL;
    }

    if (tcb_create(pcb, new_tcb_ht, regs, tid, ktcb) == NULL) {
        report_error(tag, "pcb_update: can't create tcb");

        ht_destroy(new_tcb_ht);
        ht_destroy(new_new_pages_ht);
        return NULL;
    }

    /* update pcb fields */
    pcb->pgd = pgd;

    pcb->exit_status = 0;
    pcb->exited_thread_count = 0;

    pcb->txt_base = txt_base;
    pcb->txt_len = txt_len;
    pcb->rodat_base = rodat_base;
    pcb->rodat_len = rodat_len;

    pcb->new_pages_ht = new_new_pages_ht;
    pcb->tcb_ht = new_tcb_ht;

    /* clear old structures */
    ht_destroy(old_tcb_ht);

    /* allocate pages frames have already been freed */
    ht_destroy(old_new_pages_ht);
 
    return pcb;
}

void pcb_free(pcb_t *pcb) {

    /* destroy new pages ht */
    ht_destroy(pcb->new_pages_ht);

    /* destroy threads ht */
    ht_traverse_all(pcb->tcb_ht, tcb_free);
    ht_destroy(pcb->tcb_ht);

    /* destroy children ht */
    ht_destroy(pcb->children);

    /* destroy locks */
    cond_destroy(&(pcb->wait_cond));
    
    sched_remove_pcb(pcb);

    /* destroy itself */
    free(pcb);
}
