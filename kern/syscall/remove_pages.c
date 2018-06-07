/** @file kern/remove_pages.c
 *
 *  @brief remove_pages syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "remove_pages";


int remove_pages_handler(void *base) {
    report_progress(tag, "entry");

    int len;
    if ((len = was_pages_allocated(running_ktcb->tcb->pcb, base)) <= 0) {
        report_error(tag, "base was not allocated by new_pages(), exit");
        return -1;
    }

    void *linear_addr = base;
    void *pgd = (void *)get_cr3();
    void *frm;

    /* remove frames */
    while (len != 0) {
        if ((frm = pt_entry_delete(pgd, linear_addr, 0)) == NULL) {
            report_error(tag, "cant find pt entry in pgd, exit");
            return -1;
        }
        
        mutex_lock(&(frm_mp));
        frame_free(frm);
        mutex_unlock(&(frm_mp));

        set_cr3((unsigned long)pgd);
        linear_addr += PAGE_SIZE;
        len -= PAGE_SIZE;
    }

    /* remove pt if possible */
    check_and_delete_pt(pgd, base, len);

    if (untrack_pages_allocated(running_ktcb->tcb->pcb, base) != 0) {
        report_error(tag, "failed to untrack allocated pages, exit");
        return -1;
    }

    report_progress(tag, "exit");
    return 0;
}

