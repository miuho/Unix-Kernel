/** @file vm.c
 *
 *  @brief basic virtual memory functions
 *
 *  @author HingOn Miu (hmiu)
 *  @author An Wu (anwu)
 */

#include <vm.h>
#include <frame.h>
#include <simics.h>
#include <cr.h>
#include <pgfault.h>
#include <common_kern.h>
#include <stdio.h>
#include <syscall.h>
#include <pgtable.h>
#include <pcb.h>
#include <loader.h>
#include <reporter.h>

#define MIN(x, y) ((x) < (y) ? x : y)


void *kern_pgd;

static char *tag = "vm";

int vm_ref_copy(void *pgd, void *new_pgd, int make_ro)
{
    report_progress(tag, "vm_ref_copy: entry");

    if (!(pgd_check_valid(pgd) && pgd_check_valid(new_pgd))) {
        report_error(tag, "vm_ref_copy: arg pgds not valid, exit");
        return -1;
    }

    /* copy the kernel page tables */
    memcpy(new_pgd, pgd, 4*4);

    /* create new page tables (but use old frames) for user space */
    int pgd_index;
    int pt_index;
    void *pgd_addr;
    void *pt_addr;
    void *pgd_entry;
    void *pt_entry;
    void *pt;
    void *frm;
    void *new_pt;

    unsigned long pt_flags;
    unsigned long frm_flags;

    for (pgd_index = 4; pgd_index < PAGE_SIZE / 4; pgd_index++) {
        pgd_addr = pgd + 4 * pgd_index;
        pgd_entry = *(void **)pgd_addr;
        
        pt = GET_ADDRESS(pgd_entry);
        pt_flags = GET_FLAGS(pgd_entry);

        /* if pt not accessible, continue */
        if (pt == NULL || !IS_PRESENT(pt_flags)) {
            continue;
        }

        /* allocate a new pt for new_pgd here */
        new_pt = check_and_alloc_pt(new_pgd, pgd_index, pt_flags, pt_flags);

        if (new_pt == NULL) {
            report_error(tag, "vm_ref_copy: check_and_alloc_pt failed, exit");
            return -1;
        }

        for (pt_index = 0; pt_index < PAGE_SIZE / 4; pt_index++) {
            pt_addr = pt + 4 * pt_index;
            pt_entry = *(void **)pt_addr;

            frm = GET_ADDRESS(pt_entry);
            frm_flags = GET_FLAGS((unsigned long)pt_entry);

            if (frm == NULL || !IS_PRESENT(frm_flags)) {
                continue;
            }
    
            /* if make readonly, rid writable flag off */
            if (make_ro) {
                frm_flags &= (~PG_WRITABLE);
            }

            /* use the same frame */
            pgd_insert(new_pgd, GET_LINEAR_ADDR(pgd_index, pt_index),
                        pt_flags, frm_flags, frm);

            /* if make readonly, also rid writable flag off the old pgd */
            if (make_ro) {
                pgd_insert(pgd, GET_LINEAR_ADDR(pgd_index, pt_index),
                            pt_flags, frm_flags, frm);
            }
            
            set_cr3((unsigned long)pgd);

            mutex_lock(&frm_ref_mp);

            int ref_count = get_frame_refs(frm);
            if (ref_count < 0) {
                report_error(tag, "vm_ref_copy: cannot find ref for frame %p",
                            frm);
                ref_count = 0;
            }
            set_frame_refs(frm, ref_count + 1);

            mutex_unlock(&frm_ref_mp);
        }
    }

    report_progress(tag, "vm_ref_copy: exit");

    return 0;
}

void vm_ref_copy_rollback(void *pgd) 
{
    report_progress(tag, "vm_ref_copy_rollback: entry");

    if (pgd == NULL) {
        report_error(tag, "vm_ref_copy_rollback: pgd is NULL");
        return;
    }

    int pgd_index;
    void *pgd_addr;
    void *pgd_entry;
    void *pt;
    unsigned long pt_flags;

    for (pgd_index = 4; pgd_index < PAGE_SIZE / 4; pgd_index++) {
        pgd_addr = pgd + 4 * pgd_index;
        pgd_entry = *(void **)pgd_addr;

        pt = GET_ADDRESS(pgd_entry);
        pt_flags = GET_FLAGS(pgd_entry);

        /* if pt not accessible, continue */
        if (pt == NULL || !IS_PRESENT(pt_flags)) {
            continue;
        }

        report_progress(tag, "vm_ref_copy_rollback: free pt %p at %p",
                        pt, GET_LINEAR_ADDR(pgd_index, 0));
        sfree(pt, PAGE_SIZE);
    }
}

int vm_frm_copy(void *pgd, void *linear_addr, int make_writable)
{
    report_progress(tag, "vm_frm_copy: entry");

    if (!pgd_check_valid(pgd)) {
        report_error(tag, "vm_frm_copy: get invalid pgd, exit");
        return -1;
    }

    int pgd_index;
    int pt_index;
    void *pgd_addr;
    void *pt_addr;
    void *pgd_entry;
    void *pt_entry;
    void *pt;
    void *frm;

    unsigned long pt_flags;
    unsigned long frm_flags;

    int ref_count;
    void *new_frm_addr;

    pgd_index = GET_PGD_INDEX(linear_addr);
    pt_index = GET_PT_INDEX(linear_addr);

    pgd_addr = pgd + 4 * pgd_index;
    pgd_entry = *(void **)pgd_addr;

    pt = GET_ADDRESS(pgd_entry);
    pt_flags = GET_FLAGS(pgd_entry);

    if (pgd_entry == NULL) {
        report_error(tag, "vm_frm_copy: get NULL pgd_entry, exit");
        return -1;
    }

    pt_addr = pt + 4 * pt_index;
    pt_entry = *(void **)pt_addr;

    frm = GET_ADDRESS(pt_entry);
    frm_flags = GET_FLAGS(pt_entry);
    
    mutex_lock(&(frm_ref_mp));
    ref_count = get_frame_refs(frm);

    if (ref_count == 1) {
        report_progress(tag, "vm_frm_copy: address %p last reference", 
                        linear_addr);

        if (make_writable) {
            /* last frame, just make itself writable */
            *(void **)pt_addr = ADD_FLAGS(pt_entry, frm_flags | PG_WRITABLE);
            set_cr3((unsigned long)pgd);
        }
        mutex_unlock(&(frm_ref_mp));

        return 0;
    }

    set_frame_refs(frm, ref_count - 1);
    mutex_unlock(&(frm_ref_mp));

    /* not last frame, need to copy */
    if (make_writable) {
        pt_flags |= PG_WRITABLE;
        frm_flags |= PG_WRITABLE;
    }

    if (pgd_insert(pgd, LAST_PAGE_ADDR, pt_flags, frm_flags, NULL) < 0) {
        report_error(tag, "vm_frm_copy: can't allocate temp page, exit");
        return -1;
    }

    report_progress(tag, "vm_frm_copy: going to memcpy from %p to LAST_PAGE", 
                    GET_LINEAR_ADDR(pgd_index, pt_index));
    memcpy((void *)LAST_PAGE_ADDR, GET_LINEAR_ADDR(pgd_index, pt_index),
            PAGE_SIZE);

    pt_entry_delete(pgd, linear_addr, 0);
    report_progress(tag, "vm_frm_copy: memcpy to LAST_PAGE done");

    new_frm_addr = pgd_get_frm(pgd, LAST_PAGE_ADDR);

    if (pgd_insert(pgd, linear_addr, pt_flags, frm_flags, new_frm_addr) < 0) {
        report_error(tag, 
                    "vm_frm_copy: cannot insert new frame into pgd, exit");
        return -1;
    }

    if (pt_entry_delete(pgd, LAST_PAGE_ADDR, 1) == NULL) {
        report_error(tag, 
                "vm_frm_copy: cannot delete new frame from last page, exit");
        return -1;
    }

    set_cr3((unsigned long)pgd);

    report_progress(tag, "vm_frm_copy: exit");

    return 0;
}


uint32_t vm_set_cr3(void *pgd) {
    uint32_t cr3 = get_cr3();
    cr3 &= (~CR3_PWT);
    cr3 &= (~CR3_PCD);
    cr3 <<= 20;
    cr3 >>= 20;
    cr3 |= (uint32_t)pgd;
    set_cr3(cr3);
    return cr3;
}


void vm_enable_paging() {
    uint32_t cr0 = get_cr0();
    cr0 |= CR0_PG;
    set_cr0(cr0);
}

void vm_set_global_enable() {
    uint32_t cr4 = get_cr4();
    cr4 |= CR4_PGE;
    set_cr4(cr4);
}

int vm_init(void)
{
    if (frame_init() != 0) {
        report_error(tag, "frame initialization failed in vm_init");
        return -1;
    }

    if ((kern_pgd = frame_kern_init()) == NULL) {
        report_error(tag, "kernel frame initialization failed in vm_init");
        return -1;
    }
    
    vm_set_cr3(kern_pgd);
    vm_enable_paging();
    vm_set_global_enable();

    return 0;
}

int vm_mem_region_check(pcb_t *pcb, void *pgd, void *start_addr, int len) {
    int writable = 1;

    void *linear_addr;

    for (linear_addr = start_addr; linear_addr < start_addr + len; 
            linear_addr += PAGE_SIZE) {
        if ((writable = MIN(writable, vm_mem_check(pcb, pgd, linear_addr))) 
                < 0) {
            return -1;
        }
    }

    return writable;
}

int vm_mem_check(pcb_t *pcb, void *pgd, void *linear_addr) {
    if (pgd == NULL) {
        return -1;
    }

    int pgd_index, pt_index;
    void *pgd_addr, *pgd_entry, *pt, *pt_addr, *pt_entry, *frm;
    unsigned long pt_flags, frm_flags;

    int writable = 1;

    pgd_index = GET_PGD_INDEX(linear_addr);
    pt_index = GET_PT_INDEX(linear_addr);

    pgd_addr = pgd + 4 * pgd_index;
    pgd_entry = *(void **)pgd_addr;

    pt = GET_ADDRESS(pgd_entry);
    pt_flags = GET_FLAGS(pgd_entry);

    if (pt == NULL || (pt_flags && PG_PRESENT == 0) || 
        (pt_flags && PG_USER == 0)) {
        return -1;
    }

    pt_addr = pt + 4 * pt_index;
    pt_entry = *(void **)pt_addr;

    frm = GET_ADDRESS(pt_addr);
    frm_flags = GET_FLAGS(pt_addr);

    if (frm == NULL || (frm_flags && PG_PRESENT == 0) || 
        (pt_flags && PG_USER == 0)) {
        return -1;
    }

    if (((linear_addr >= pcb->txt_base) && 
        (linear_addr < pcb->txt_base + pcb->txt_len)) ||
        ((linear_addr >= pcb->rodat_base) && 
        (linear_addr < pcb->rodat_base + pcb->rodat_len))) {
        writable = 0;
    }

    return writable;
}


int vm_mem_array_check(pcb_t *pcb, void *pgd, int elem_size, void *start) {
    int writable = 1;
    void *linear_addr = start;

    while (1) {
        if ((writable = MIN(writable, vm_mem_check(pcb, pgd, linear_addr))) 
                < 0) {
            return -1;
        }

        /* check for terminated 0 and increment linear address according to
         * elem size 
         */
        do {
            switch (elem_size) {
                case 1:
                    if (*(char *)linear_addr == '\0')
                        return writable;
                    linear_addr++;
                    break;
                case 4:
                    if (*(void **)linear_addr == NULL)
                        return writable;
                    linear_addr += 4;
                    break;
                default:
                    report_error(tag, 
                                "vm_mem_array_check: get wrong elem size");
                    break;
            }
        } while ((unsigned long)linear_addr % PAGE_SIZE != 0);
    }

    /* never reach here */
    report_error(tag, "vm_mem_array_check: out of loop");
    return -1;
}


