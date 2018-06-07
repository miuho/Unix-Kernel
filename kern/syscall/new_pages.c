/** @file kern/new_pages.c
 *
 *  @brief new_pages syscall implementation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <syscall_handler.h>

#include <common_include.h>

static char *tag = "new_pages";

int new_pages_handler(void *args) {

    report_progress(tag, "entry");

    pcb_t *pcb = running_ktcb->tcb->pcb;
    void *pgd = (void *)pcb->pgd;

    if (vm_mem_region_check(pcb, pgd, args, 8) < 0) {
        report_error(tag, "new_pages_handler: arguments not accessible, exit");
        return -1;
    }

    void *base = *(void **)args;
    int len = *(int *)(args + 4);

    /* check if base aligned */
    if (((unsigned long)base & (PAGE_SIZE - 1)) != 0) {
        report_error(tag, "base is not page-aligned, exit");
        return -1;
    }

    /* check if len is multiple of PAGE_SIZE */
    if (len <= 0 || (len % PAGE_SIZE != 0)) {
        report_error(tag, "len is not multiple of PAGE_SIZE, exit");
        return -1;
    }

    /* check if there are enough frames left */
    if (len > frame_get_count() * PAGE_SIZE) {
        report_error(tag, "len is more than available frames");
        return -1;
    }

    /* check if base is in kernel memory region */
    if ((unsigned long)base < USER_MEM_START ||
         (unsigned long)base + len > USER_STACK_BASE) {
        report_error(tag, "base cannot contain kernel reserved memory, exit");
        return -1;
    }

    /* check if any portion was allocated before */
    int remain = len;
    void *linear_addr = base;

    while (remain != 0) {
        report_misc(tag, "going to check if memory %p allocated", linear_addr);
        if (pgd_get_frm(pgd, linear_addr) != NULL) {
            report_error(tag, "contains allocated memory region %p, exit", 
                            linear_addr);
            return -1;
        }
        linear_addr += PAGE_SIZE;
        remain -= PAGE_SIZE;
    }

    /* allocate pages */
    report_progress(tag, "going to allocate new page at %p for 0x%x", base,
                    len);
    if (pgd_alloc_pages((void *)get_cr3(), base, len,
        PG_PRESENT | PG_WRITABLE | PG_USER,
        PG_PRESENT | PG_WRITABLE | PG_USER) != 0) {
        report_error(tag, "failed to allocate pages, exit");
        return -1;
    }

    if (track_pages_allocated(running_ktcb->tcb->pcb, base, len) != 0) {
        report_error(tag, "failed to track allocated pages, exit");
        return -1;
    }

    memset(base, 0, len);

    report_progress(tag, "exit");

    return 0;
}
