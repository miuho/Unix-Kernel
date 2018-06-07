/** @file kern/inc/pgtable.h
 *  
 *  @brief page global directory and page table entry interface
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */
#ifndef _KERN_INC_PGTABLE_H_
#define _KERN_INC_PGTABLE_H_

#include <common_kern.h>
#include <stdio.h>
#include <simics.h>
#include <frame.h>
#include <syscall.h>
#include <malloc.h>
#include <string.h>

/* the page size shift */
#define PAGE_SHIFT 12

/* Flag Bit 0 */
#define PG_PRESENT 0b1
#define PG_IS_PRESENT(entry) ((unsigned long)(entry) & 0b1)
/* Flag Bit 1 */
#define PG_WRITABLE 0b10
#define PG_READONLY (~0b10)
/* Flag Bit 2 */
#define PG_USER 0b100
#define PG_SUPERVISOR (~0b100)
/* Flag Bit 8, cr4's PGE needs to be set */
#define PG_PREVENT_MAPPING_FLUSHED 0b100000000

/* check if the bit is on in flags */
#define IS_PRESENT(flags) (int)((flags & PG_PRESENT))
#define IS_WRITABLE(flags) (int)((flags & PG_WRITABLE) >> 1)
#define IS_USER(flags) (int)((flags & PG_USER) >> 2)

/* discard least 12 bits */
#define GET_ADDRESS(x) ((void *)((unsigned long)(x) & ~0b111111111111))

/* get flags of an entry */
#define GET_FLAGS(x) ((unsigned long)(x) & 0xfff)
#define ADD_FLAGS(x, flags) (void *)(((unsigned long)(x) & 0xfffff000) | flags)

/* pgd, pte and linear address converting */
#define GET_PGD_INDEX(x) (((unsigned long)(x) >> 22) & 0b1111111111)
#define GET_PT_INDEX(x) (((unsigned long)(x) >> 12) & 0b1111111111)
#define GET_LINEAR_ADDR(pgd, pte) ((void *)(((unsigned long)(pgd) << 22) | \
                                            ((unsigned long)(pte) << 12)))

/** @brief allocate a pgd
 *
 *  @return the pointer to pgd on success, NULL on error
 */
void *pgd_alloc(void);

/** @brief insert a page into a virtual address in a pgd
 *
 *  @param pgd the pgd we want to insert page for
 *  @param linear_addr the linear address we want to insert page at
 *  @param pt_flags the page table flags we want to use
 *  @param frm_flags the frame flags we want to use
 *  @param phy_frame if present use this to insert
 *  @return 0 on success, -1 on error
 */
int pgd_insert(void *pgd, void *linear_addr, unsigned long pt_flags,
               unsigned long frm_flags, void *phy_frame);

/** @brief allocate frames for a range of linear address
 *
 *  @param pgd the pgd we want to insert page for
 *  @param start_linear_addr the starting linear address of inserting
 *  @param size the size of the linear address region
 *  @param pt_flags the page table flags we want to assign
 *  @param frm_flags the frame flags we want to assign
 *  @return 0 on success, -1 on error
 */
int pgd_alloc_pages(void *pgd, void *start_linear_addr, int size, 
                    unsigned long pt_flags, unsigned long frm_flags);

/** @brief direct map linear address and frames (for kernel)
 *
 *  @param pgd the pgd we want to direct map frames for
 *  @param linear_addr the address we want to direct map frames at
 *  @param flags the flags we want to use
 *  @return 0 on success, -1 on error
 */
int pgd_direct_map(void *pgd, void *linear_addr, unsigned long flags);

/** @brief free a pgd and the page tables inside
 *
 *  @param pgd the pgd we want to free
 *  @return Void
 */
void pgd_free(void* pgd);

/** @brief cleanup a pgd by go through its frames and decrement reference count
 *         (COW), and free frame & pt if appropriate
 *
 *  @param pgd the pgd we want to cleanup
 *  @return Void
 */
void pgd_cleanup(void *pgd);

/** @brief delete an entry in a page table and returns the frame there.
 *         delete the corresponding page table if appropriate
 *
 *  @param pgd the pgd we want to delete entry for
 *  @param linear_addr the linear addr we want to delete entry at
 *  @param delete_pt if non-zero, delete the corresponding page table
 *  @return the frame at linear_addr if exist, NULL if not or error
 */
void *pt_entry_delete(void *pgd, void *linear_addr, int delete_pt);

/** @brief get the frame of pgd at a linear address
 *
 *  @param pgd the pgd we want to get frame from
 *  @param linear_addr the linear_addr we want to get frame at
 *  @return the frame if success, NULL if non-exist or error
 */
void *pgd_get_frm(void *pgd, void *linear_addr);

/** @brief check a linear address and allocate a page table there if there wasn't
 *         one before. It uses the lower priority of pt_flags and frm_flags
 *
 *  @param pgd the pgd we want to use
 *  @param pgd_index the position of page table
 *  @param pt_flags the page table flags we want to specify
 *  @param frm_flags the frame flags we want to specify
 *  @return the page table address on success, NULL on error
 */
void *check_and_alloc_pt(void *pgd, int pgd_index, unsigned long pt_flags, 
                         unsigned long frm_flags);

/** @brief check a range of linear address and delete the corresponding page table
 *         if the linear address range contain no frames
 *
 *  @param pgd the pgd we want to use
 *  @param base the base of linear address
 *  @param len the length of address region
 *  @return Void
 */
void check_and_delete_pt(void *pgd, void *base, int len);

/** @brief check if a page table is valid
 *
 *  @param pgd the pgd we want to check
 *  @return 1 if valid, 0 if not
 */
int pgd_check_valid(void *pgd);

/** @brief clean up a pgd due to process exit. 
 *         This decrements all frame reference count by 1 (COW) and free frames if
 *         appropriate. Also frees the page tables
 *
 *  @param pgd the pgd we want to cleanup
 *  @return Void
 */
void pgd_process_cleanup(void *pgd);

#endif
