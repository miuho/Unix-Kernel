/** @file kern/pgtable.c
 *  
 *  @brief page global directory and page table entry interface
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <pgtable.h>
#include <cr.h>
#include <reporter.h>

static char *tag = "pgtable";

void *pgd_alloc(void)
{
    report_progress(tag, "pgd_alloc: entry");

    void *free_frm = smemalign(PAGE_SIZE, PAGE_SIZE);
    
    if (free_frm == NULL) {
        report_error(tag, "pgd_alloc: smemalign failed");
        return NULL;
    }
    
    memset(free_frm, 0, PAGE_SIZE);
    
    report_progress(tag, "pgd_alloc: allocated %p", free_frm);
    return free_frm;
}

void pgd_free(void* pgd)
{
    report_progress(tag, "pgd_free: entry");

    int i;
    void *pgd_addr;
    void *pgd_entry;
    void *pt;

    unsigned long pt_flags;

    for (i = 4; i < PAGE_SIZE/4; i++) {
        pgd_addr = pgd + 4 * i;
        pgd_entry = *(void **)pgd_addr;
   
        pt = GET_ADDRESS(pgd_entry);
        pt_flags = GET_FLAGS(pgd_entry);

        if (pt == NULL || !IS_PRESENT(pt_flags)) 
            continue;
    
        sfree(pt, PAGE_SIZE);
    }
    
    sfree(pgd, PAGE_SIZE);
    return;
}

int pgd_check_valid(void *pgd)
{
    return (pgd != NULL) && ((unsigned long)pgd < USER_MEM_START) &&
            ((unsigned long)pgd % PAGE_SIZE == 0);
}

void *check_and_alloc_pt(void *pgd, int pgd_index, 
                        unsigned long pt_flags, unsigned long frm_flags) {
    void *pgd_addr = pgd + 4 * pgd_index;
    void *pgd_entry = *(void **)pgd_addr;

    void *pt = GET_ADDRESS(pgd_entry);
    unsigned long current_pt_flags = GET_FLAGS(pgd_entry);

    /* use the lower priviledge one */
    if ((frm_flags & PG_PRESENT) != 0)
        pt_flags |= PG_PRESENT;
    if ((frm_flags & PG_WRITABLE) != 0)
        pt_flags |= PG_WRITABLE;
    if ((frm_flags & PG_USER) != 0)
        pt_flags |= PG_USER;


    if (pt == NULL || !IS_PRESENT(current_pt_flags)) {
        pt = smemalign(PAGE_SIZE, PAGE_SIZE);
        if (pt == NULL) {
            report_error(tag, "check_and_alloc_pt: can't smemalign returned");
            return NULL;
        }
        memset(pt, 0, PAGE_SIZE);

        *(void **)pgd_addr = (void *)((unsigned long)pt | pt_flags);
        return pt;
    }
    else {
        /* update flags */
        *(void **)pgd_addr = (void *)((unsigned long)pgd_entry | pt_flags);
        return pt;
    }
}

void check_and_delete_pt(void *pgd, void *base, int len) {
   
    report_progress(tag, "check_and_delete_pt: entry");
    
    if (pgd == NULL) {
        report_error(tag, "check_and_delete_pt: pgd NULL");
        return;
    }
    
    if ((unsigned long)base < USER_MEM_START || len <= 0) {
        report_warning(tag, "check_and_delete_pt: invalid base or len");
        return;
    }
     
    int pgd_index;
    int pt_index;

    void *pgd_addr;
    void *pgd_entry;

    void *pt_addr;
    void *pt_entry;

    void *pt;
    void *frm;

    unsigned long pt_flags;
    unsigned long frm_flags;

    void *linear_addr;
    for (linear_addr = base; linear_addr < base + len; 
            linear_addr += PAGE_SIZE) {

        pgd_index = GET_PGD_INDEX(linear_addr);
        pgd_addr = pgd + 4 * pgd_index;
        pgd_entry = *(void **)pgd_addr;

        pt = GET_ADDRESS(pgd_entry);
        pt_flags = GET_FLAGS(pgd_entry);

        if (pt == NULL || !IS_PRESENT(pt_flags)) {
            report_error(tag, "check_and_delete_pt: pt not accessible");
            return;
        }

        report_progress(tag, "check_and_delete_pt: checking linear_addr %p",
                        linear_addr);

        int has_content = 0;
        for (pt_index = 0; pt_index < PAGE_SIZE / 4; pt_index++) {
            
            pt_addr = pt + 4 * pt_index;
            pt_entry = *(void **)pt_addr;

            frm = GET_ADDRESS(pt_entry);
            frm_flags = GET_FLAGS(pt_entry);

            /* found content */
            if (frm != NULL && IS_PRESENT(frm_flags)) {
                has_content = 1;
                break;
            }
        }

        if (!has_content) {
            /* no content in pt, delete it */
            report_progress(tag, "check_and_delete_pt: deleting pt %p", pt);
            *(void **)pgd_addr = NULL;
            sfree(pt, PAGE_SIZE);
        }
        else {
            report_progress(tag, "check_and_delete_pt: pt %p has content", pt);
        }
    }

    return;
}


int pgd_direct_map(void *pgd, void *linear_addr, unsigned long flags)
{
    if (pgd == NULL)
        return -1;   
    
    int pgd_index = GET_PGD_INDEX((unsigned long)linear_addr);
    int pt_index = GET_PT_INDEX((unsigned long)linear_addr);

    void *pt;
    if ((pt = check_and_alloc_pt(pgd, pgd_index, flags, flags)) 
            == NULL) {
        report_error(tag, "fail to alloc pte");
        return -1;
    }
    
    /* guarantee page table exists now */
    void *pt_addr = pt + 4 * pt_index;

    /* physical frame did exist */
    if (*(void **)pt_addr != NULL) {
        report_error(tag, "Warning: frame exist, overwritting...");
    }

    *(void **)pt_addr = (void *)((unsigned long)linear_addr | flags);
    return 0;
}


int pgd_alloc_pages(void *pgd, void *start_linear_addr, int size, 
                    unsigned long pt_flags, unsigned long frm_flags)
{
    void *aligned_linear_addr = GET_ADDRESS(start_linear_addr);
    while (aligned_linear_addr <=  start_linear_addr + size - 1) {

        if (pgd_insert(pgd, aligned_linear_addr, 
                        pt_flags, frm_flags, NULL) != 0) {
            
            report_error(tag, "failed to add one page from %p", 
                        aligned_linear_addr);
            return -1;
        }

        aligned_linear_addr += PAGE_SIZE;
    }
    
    return 0;
}

void *pt_entry_delete(void *pgd, void *linear_addr, int delete_pt)
{
    if (pgd == NULL) {
        report_error(tag, "pt_entry_delete: get NULL pgd");
        return NULL;   
    }
    
    int pgd_index = GET_PGD_INDEX((unsigned long)linear_addr);
    int pt_index = GET_PT_INDEX((unsigned long)linear_addr);

    void *pgd_addr = pgd + 4 *pgd_index;
    void *pgd_entry = *(void **)pgd_addr;

    /* check pt */
    void *pt = GET_ADDRESS(pgd_entry);
    unsigned long pt_flags = GET_FLAGS(pgd_entry);
    
    if (pt == NULL || !IS_PRESENT(pt_flags)) {
        report_error(tag, "pt_entry_delete: failed to find pt");
        return NULL;
    }
    
    void *pt_addr = pt + 4 * pt_index;
    void *pt_entry = *(void **)pt_addr;

    /* check frm */
    void *frm = GET_ADDRESS(pt_entry);
    unsigned long frm_flags = GET_FLAGS(pt_entry);

    if (frm == NULL || !IS_PRESENT(frm_flags)) {
        report_error(tag, "pt_entry_delete: failed to find frm");
        return NULL;
    }

    /* clear the entry */
    *(void **)pt_addr = NULL;
    if (delete_pt) {
        *(void **)pgd_addr = NULL;
    }

    if (delete_pt) {
        sfree(pt, PAGE_SIZE);
    }
    
    return frm;
}

int pgd_insert(void *pgd, void *linear_addr, unsigned long pt_flags,
               unsigned long frm_flags, void *phy_frame)
{
    if (pgd == NULL)
        return -1;   
    
    void *free_frm;
    int pgd_index = GET_PGD_INDEX((unsigned long)linear_addr);
    int pt_index = GET_PT_INDEX((unsigned long)linear_addr);

    report_progress(tag, "pgd_insert: going to insert pt at %p", 
                    GET_LINEAR_ADDR(pgd_index, 0));
    void *pt;
    if ((pt = check_and_alloc_pt(pgd, pgd_index, pt_flags, frm_flags))== NULL)     {
        report_error(tag, "fail to alloc pte");
        return -1;
    }

    /* guarantee page table exists now */
    void *pt_addr = pt + 4 * pt_index;

    /* physical frame did exist */
    if (*(void **)pt_addr != NULL) {
        *(void **)pt_addr = ADD_FLAGS(*(void **)pt_addr, frm_flags);
        report_warning(tag, "pgd_insert: frame exist.");
        return 0;
    }

    /* physical frame didn't exist */
    if (phy_frame == NULL) {
        mutex_lock(&(frm_mp));
        free_frm = frame_alloc();
        mutex_unlock(&(frm_mp));

        if (free_frm == NULL) {
            report_error(tag, "no physical pages");
            return -1;
        }
    }
    else {
        /* given a specific physical frame that the linear address */
        /* should map to */
        free_frm = phy_frame;
    }

    *(void **)pt_addr = (void *)((unsigned long)free_frm | frm_flags);

    return 0;
}

void *pgd_get_frm(void *pgd, void *linear_addr) 
{
    if (pgd == NULL) {
        report_error(tag, "pgd_get_frm: got NULL pgd");
        return NULL;
    }

    int pgd_index = GET_PGD_INDEX((unsigned long)linear_addr);
    int pt_index = GET_PT_INDEX((unsigned long)linear_addr);

    void *pgd_entry = *(void **)(pgd + 4 *pgd_index);
    void *pt = GET_ADDRESS(pgd_entry);
    if (pgd_entry == NULL) {
        report_warning(tag, "directory has no entry at %p", 
                    (void *)(pgd + 4 *pgd_index));
        return NULL;
    }

    void *pt_entry = *(void **)(pt + 4 * pt_index);
    if (pt_entry == NULL) {
        report_warning(tag, "table has no entry at %p", 
                    (void *)(pt + 4 * pt_index));
        return NULL;
    }

    return GET_ADDRESS((unsigned long)pt_entry);
}

unsigned long pgd_get_ptflags(void *pgd, void *linear_addr) 
{
    if (pgd == NULL) {
        report_error(tag, "pgd_get_ptflags: got NULL pgd");
        return 0;
    }   

    int pgd_index = GET_PGD_INDEX((unsigned long)linear_addr);

    void *pgd_entry = *(void **)(pgd + 4 *pgd_index);
    
    return GET_FLAGS(pgd_entry);
}

unsigned long pgd_get_frmflags(void *pgd, void *linear_addr) 
{
    if (pgd == NULL) {
        report_error(tag, "pgd_get_frmflags: got NULL pgd");
        return 0;
    }   

    int pgd_index = GET_PGD_INDEX((unsigned long)linear_addr);
    int pt_index = GET_PT_INDEX((unsigned long)linear_addr);

    void *pgd_entry = *(void **)(pgd + 4 *pgd_index);
    void *pt = GET_ADDRESS(pgd_entry);
    if (pgd_entry == NULL) {
        report_error(tag, "pgd_get_frmflags: pgd_entry is NULL");
        return 0;
    }   

    void *pt_entry = *(void **)(pt + 4 * pt_index);

    return GET_FLAGS(pt_entry);
}

void pgd_print(void *pgd)
{
    int i, j;
    void *pgd_addr;
    void *pgd_entry;
    void *pt;
    unsigned long pt_flags;
    void *pt_addr;
    void *pt_entry;
    void *frm;
    void *linear;
    unsigned long frm_flags;
    for (i = 4; i < 1024; i++) {
        pgd_addr = pgd + 4 * i;
        pgd_entry = *(void **)pgd_addr;
        pt = GET_ADDRESS(pgd_entry);
        pt_flags = GET_FLAGS(pgd_entry);
        if (pt == NULL) 
            continue;
        for (j = 0; j < 1024; j++) {
            pt_addr = pt + 4 * j;
            pt_entry = *(void **)pt_addr;
            frm = GET_ADDRESS(pt_entry);
            frm_flags = GET_FLAGS(pt_entry);
            if (frm == NULL)
                continue;
            
            linear = GET_LINEAR_ADDR(i, j);
            lprintf("linear=%p pt=%p present=%d writable=%d user=%d",
                    linear, pt, IS_PRESENT(pt_flags),
                    IS_WRITABLE(pt_flags), IS_USER(pt_flags));
            lprintf("linear=%p frm=%p present=%d writable=%d user=%d",
                    linear, frm, IS_PRESENT(frm_flags),
                    IS_WRITABLE(frm_flags), IS_USER(frm_flags));
        }
    }
}

void pgd_cleanup(void *pgd)
{
    if (pgd == NULL) {
        report_error(tag, "pgd_cleanup: pgd is NULL");
        return;
    }

    int i, j;
    void *pgd_addr;
    void *pgd_entry;
    void *pt;
    void *pt_addr;
    void *pt_entry;
    void *frm;
    int refs;

    for (i = 4; i < PAGE_SIZE/4; i++) {
        pgd_addr = pgd + 4 * i;
        pgd_entry = *(void **)pgd_addr;
        
        pt = GET_ADDRESS(pgd_entry);
        if (pt == NULL) 
            continue;
           
        for (j = 0; j < PAGE_SIZE/4; j++) {
            pt_addr = pt + 4 * j;
            pt_entry = *(void **)pt_addr;

            frm = GET_ADDRESS(pt_entry);
            if (frm == NULL)
                continue;
            
            mutex_lock(&frm_ref_mp);
            refs = get_frame_refs(frm);
            set_frame_refs(frm, (refs == 1) ? -1 : refs - 1);
            mutex_unlock(&frm_ref_mp);
            
            if (refs < 0) 
                report_error(tag, "pgd_cleanup: ref count less than 1");

            if (refs == 1) {
                mutex_lock(&frm_mp);
                frame_free(frm);
                mutex_unlock(&frm_mp);
            }

        }

        *(void **)pgd_addr = NULL;
        sfree(pt, PAGE_SIZE);
    }
}

void pgd_compare(void *pgd1, void *pgd2) {
    int i, j;
    void *temp = smemalign(PAGE_SIZE, PAGE_SIZE);
    for (i = 0; i < 1024; i++) {
        unsigned long pgd_addr1 = (unsigned long)(pgd1 + 4 * i);
        unsigned long pgd_addr2 = (unsigned long)(pgd2 + 4 * i);
        unsigned long pgd_entry1 = *(unsigned long*)(pgd_addr1);
        unsigned long pgd_entry2 = *(unsigned long*)(pgd_addr2);

        unsigned long pgd_flag1 = GET_FLAGS(pgd_entry1);
        unsigned long pgd_flag2 = GET_FLAGS(pgd_entry2);

        unsigned long pt1 = (unsigned long)GET_ADDRESS(pgd_entry1);
        unsigned long pt2 = (unsigned long)GET_ADDRESS(pgd_entry2);

        if (pt1 == 0 && pt2 == 0)
            continue;
       
        if (!(i < 4 && pt1 == pt2 && pgd_flag1 == pgd_flag2)) {
        
            lprintf(
                "pgd1 directory entry at offset 0x%x is 0x%x, flag is 0x%x", 
            i, (int)pt1, (int)pgd_flag1);
            lprintf(
                "pgd2 directory entry at offset 0x%x is 0x%x, flag is 0x%x", 
            i, (int)pt2, (int)pgd_flag2);

            if (pgd_flag1 != pgd_flag2)
                lprintf("WARNING: flags differ!");
            if (pt1 == 0 || pt2 == 0)
                lprintf("WARNING: pgd differ!");
        }

        for (j = 0; j < 1024; j++) {
            unsigned long pt_addr1 = pt1 + 4 * j;
            unsigned long pt_addr2 = pt2 + 4 * j;

            unsigned long pt_entry1 = *(unsigned long*)pt_addr1;
            unsigned long pt_entry2 = *(unsigned long*)pt_addr2;

            unsigned long pt_flag1 = GET_FLAGS(pt_entry1);
            unsigned long pt_flag2 = GET_FLAGS(pt_entry2);

            unsigned long frm1 = (unsigned long)GET_ADDRESS(pt_entry1);
            unsigned long frm2 = (unsigned long)GET_ADDRESS(pt_entry2);

            if ((frm1 == 0 || pt1 == 0) && (frm2 == 0 || pt2 == 0))
                continue;

            if (!(i < 4 && frm1 == frm2 && pt_flag1 == pt_flag2)) 
            {

                lprintf(
                "pgd1 physical frame at offset 0x%x is 0x%x, flag is 0x%x", 
                j, (int)frm1, (int)pt_flag1);
                lprintf(
                "pgd2 physical frame at offset 0x%x is 0x%x, flag is 0x%x", 
                j, (int)frm2, (int)pt_flag2);


                if (pt_flag1 != pt_flag2)
                    lprintf("WARNING: flags differ!");
                if (frm1 == 0 || frm2 == 0)
                    lprintf("WARNING: pte differ!");

                lprintf("comparing content...");
                memcpy(temp, (void *)GET_LINEAR_ADDR(i, j), PAGE_SIZE);
                set_cr3((unsigned long)pgd2);
                if (memcmp(temp, (void *)GET_LINEAR_ADDR(i, j), 
                                PAGE_SIZE) != 0) {
                    lprintf("WARNING: Different!");
                }
                else {
                    lprintf("Same");
                }
                set_cr3((unsigned long)pgd1);

            }

        }
    }
    sfree(temp, PAGE_SIZE);
}

void pgd_process_cleanup(void *pgd) {

    report_progress(tag, "pgd_process_cleanup: entry, pgd=%p", pgd);

    if (pgd == NULL) {
        report_error(tag, "pgd_process_cleanup: pgd is NULL");
        return;
    }

    int pgd_index, pt_index;
    void *pgd_addr, *pgd_entry, *pt_addr, *pt_entry, *pt, *frm;
    unsigned long pt_flags, frm_flags;

    int refs_count;

    for (pgd_index = 4; pgd_index < PAGE_SIZE/4; pgd_index++) {
        pgd_addr = pgd + 4 * pgd_index;
        pgd_entry = *(void **)pgd_addr;

        pt = GET_ADDRESS(pgd_entry);
        pt_flags = GET_FLAGS(pgd_entry);

        /* if pt not accessible, continue */
        if (pt == NULL || !IS_PRESENT(pt_flags)) {
            continue;
        }

        for (pt_index = 0; pt_index < PAGE_SIZE/4; pt_index++) {

            pt_addr = pt + 4 * pt_index;
            pt_entry = *(void **)pt_addr;

            frm = GET_ADDRESS(pt_entry);
            frm_flags = GET_FLAGS(pt_entry);

            /* if frm not accessible, continue */
            if (frm == NULL || !IS_PRESENT(frm_flags)) {
                continue;
            }

            mutex_lock(&frm_ref_mp);
            refs_count = get_frame_refs(frm);
            set_frame_refs(frm, (refs_count == 1) ? -1 : refs_count - 1);
            mutex_unlock(&frm_ref_mp);
            
            if (refs_count < 1) {
                report_error(tag, "pgd_process_cleanup: refs_count < 1");
                return;
            }

            if (refs_count == 1) {
                report_progress(tag, 
            "pgd_process_cleanup: going to free frame %p at linear_addr %p", 
                                frm, GET_LINEAR_ADDR(pgd_index, pt_index));
                mutex_lock(&(frm_mp));
                frame_free(frm);
                mutex_unlock(&(frm_mp));
            }
        }

        /* free the pt */
        report_progress(tag, "pgd_process_cleanup: going to free pt %p", pt);
        sfree(pt, PAGE_SIZE);
    }

    /* free the pgd */
    report_progress(tag, "pgd_process_cleanup: going to free pgd %p", pgd);
    sfree(pgd, PAGE_SIZE);

}
