/**
 * @file kern/syscall/wait.c
 * @brief implements the wait handler.
 *
 * @author HingOn Miu (hmiu)
 * @author An Wu (hmiu)
 *
 */
#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "wait";

int wait_handler(int *status_ptr) {

    report_progress(tag, "entry");

    pcb_t *pcb = running_ktcb->tcb->pcb;

    if (ht_empty(pcb->children)) {
        return -1;
    }

    if (status_ptr != NULL &&
        vm_mem_region_check(pcb, (void *)pcb->pgd, 
                            (void *)status_ptr, 4) != 1)
    {
        report_error(tag, "wait_handler: arguments not accessible, exit");
        return -1;
    }

    
    pcb_t *child = NULL;

    report_progress(tag, "going to look for exited child...");
     
    report_progress(tag, "enter children->mp (searching)");
    mutex_lock(&(pcb->children->mp));
    
    int if_set = if_disable();
    int root_tid = -1;
    if (!ht_empty(pcb->children)) {
        while ((child = (pcb_t *)ht_find(pcb->children,
                        pcb_all_thr_exited)) == NULL) {
            
            report_progress(tag, "enter wait_cond");
            cond_wait(&(pcb->wait_cond), &(pcb->children->mp));
            report_progress(tag, "leave wait_cond");
        }
        if_recover(if_set);
        
        /* delete child after finding it */
        if (ht_delete(pcb->children, child->pid) == NULL) {
            mutex_unlock(&(pcb->children->mp));
            report_error(tag, "cannot delete child from children ht, exit");
            return -1;
        }

        root_tid = child->pid;
    }
    if_recover(if_set);
     
    report_progress(tag, "leave children->mp (child found or empty)");
    mutex_unlock(&(pcb->children->mp));

    report_progress(tag, "going to cleanup child resource...");

    if (status_ptr != NULL) {
        *status_ptr = child->exit_status;
    }

    
    /* free child pcb's data structures */
    if (child != NULL)
        pcb_free(child);

    report_progress(tag, "exit");
    return root_tid;
}
