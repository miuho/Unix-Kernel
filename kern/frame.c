/** @file frame.c
 *
 *  @brief physical frame manipulation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <frame.h>
#include <pgtable.h>
#include <syscall.h>
#include <queue.h>
#include <mutex.h>
#include <hash.h>
#include <reporter.h>

queue free_frame_queue;

ht_t *allocated_frame_ht;

mutex_t frm_ref_mp;

mutex_t frm_mp;

int frame_count;

static char *tag = "frame";

int key_compare_frm(ht_entry_t *e1, ht_entry_t *e2)
{
    return (int)(e1->key) == (int)(e2->key);
}

int frame_init() {
    /* since kernal memory is direct mapping of virtual memory to physical
     * memory, we only need to manage the frames for the user.
     */
    frame_count = machine_phys_frames() - (USER_MEM_START >> PAGE_SHIFT);

    if (frame_count <= 0)
        return -1;

    free_frame_queue = queue_new();
    if (free_frame_queue == NULL) {
        report_error(tag, "cannot alloc free frame queue");
        return -1;
    }

    allocated_frame_ht = ht_new((key_compare_fn) key_compare_frm);
    if (allocated_frame_ht == NULL) {
        report_error(tag, "cannot alloc frame ht");
        return -1;
    }

    if (mutex_init(&frm_ref_mp) != 0) {
        report_error(tag, "failed to init frm_ref_mp");
        return -1;
    }

    if (mutex_init(&frm_mp) != 0) {
        report_error(tag, "failed to init frm_mp");
        return -1;
    }

    /* builds relation between free frames 
     * for each frame, the first two pointers are prev and next,
     * which points to previous free frame and next free frame, respectively
     */
    void *frm_addr;
    int i;
    for (i = 0; i < frame_count; i++) {
        frm_addr = (void *) (USER_MEM_START + (i * PAGE_SIZE));
        
        if (enqueue(frm_addr, free_frame_queue) < 0) {
            return -1;
        }
    }

    return 0;
}

int get_frame_refs(void *frame)
{
    void *refs;
    
    mutex_lock(&(allocated_frame_ht->mp));
    
    if ((refs = ht_lookup(allocated_frame_ht, (int)frame)) == NULL) {
        
        mutex_unlock(&(allocated_frame_ht->mp));
        
        report_error(tag, "cannot find refs for frame %p", frame);
        return -1;
    }

    mutex_unlock(&(allocated_frame_ht->mp));

    return (int)refs;
}

int set_frame_refs(void *frame, int refs)
{
    mutex_lock(&(allocated_frame_ht->mp));
    
    if (ht_delete(allocated_frame_ht, (int)frame) == NULL) {
        
        mutex_unlock(&(allocated_frame_ht->mp));
        
        report_error(tag, "cannot delete refs for frame %p", frame);
        return -1;
    }

    ht_insert(allocated_frame_ht, (int)frame, (void *)refs);
    
    mutex_unlock(&(allocated_frame_ht->mp));

    return 0;
}

void *frame_kern_init() {
    unsigned long flags;
    flags = PG_WRITABLE | PG_PRESENT | PG_PREVENT_MAPPING_FLUSHED;
    void *kern_pgd = pgd_alloc();
    
    report_progress(tag, "kern_pgd is %p, flags is %d", kern_pgd, (int)flags);
    if (kern_pgd == NULL)
        return NULL;

    void *kern_addr;
    report_progress(tag, "USER_MEM_START = 0x%x", USER_MEM_START);
    for (kern_addr = NULL + PAGE_SIZE; 
         (unsigned long)kern_addr < USER_MEM_START; 
         kern_addr += PAGE_SIZE) {

        if (pgd_direct_map(kern_pgd, kern_addr, flags) != 0) {
            report_error(tag, "direct map failed at %p", kern_addr);
            pgd_free(kern_addr);
            return NULL;
        }
    }

    return kern_pgd;
}

void *frame_alloc() {
    mutex_lock(&(free_frame_queue->mp));
    void *data = dequeue(free_frame_queue);
    mutex_unlock(&(free_frame_queue->mp));

    report_progress(tag, "frame_alloc: going to alloc %p", data);
    
    mutex_lock(&(allocated_frame_ht->mp));
    
    report_misc(tag, "frame_alloc %p", data);
    
    ht_insert(allocated_frame_ht, (hash_key)data, (hash_value)1);
    mutex_unlock(&(allocated_frame_ht->mp));

    if (data == NULL) {
        report_error(tag, "no more frames left");
    }

    frame_count--;

    return data;
}

void frame_free(void *frame) {
    if (frame == NULL) {
        return;
    }
    
    report_progress(tag, "frame_free: going to free %p", frame);
     
    mutex_lock(&(free_frame_queue->mp));
    enqueue(frame, free_frame_queue);
    mutex_unlock(&(free_frame_queue->mp));
    
    mutex_lock(&(allocated_frame_ht->mp));
    
    report_misc(tag, "frame_free %p", frame);

    if (ht_delete(allocated_frame_ht, (hash_key)frame) == NULL) {
        report_error(tag, "fail to remove frame %p from allocated_frame_ht", 
                    frame);
    }

    mutex_unlock(&(allocated_frame_ht->mp));

    frame_count++;

    return;
}

int frame_get_count() {
    return frame_count;
}
