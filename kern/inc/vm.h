/** @file kern/inc/vm.h
 *  @brief header file for virtual memory allocator
 *
 *  @author HingOn Miu (hmiu)
 *  @author An Wu (anwu)
 *  @bug No known bugs.
 */

#ifndef _KERN_INC_VM_H_
#define _KERN_INC_VM_H_

/* number of MB in kernel's pgd */
#define KERNEL_PGD_SIZE 16
/* record the aligned address of the last page */
#define LAST_PAGE_ADDR ((void *)0xfffff000)
 
#include <stdint.h>
#include <pcb.h>

/** @brief record the address of kern's pgd
 *
 */
extern void *kern_pgd;

/** @brief initialize the virtual memory allocator
 *
 *  @return 0 if successful, -1 otherwise
 */
int vm_init(void);

/** @brief set cr3 with given pgd
 *
 *
 *  @param pgd the address of pgd
 *  @return the cr3
 */
uint32_t vm_set_cr3(void *pgd);

/** @brief enable paging
 *
 *  @return Void
 */
void vm_enable_paging();

/** @brief copy the COW pages to a new pgd and make read-only based on make_ro
 *
 *  @param pgd the source
 *  @param new_pgd the destination
 *  @param make_ro if 1, make read only
 *  @return 0 on success, -1 on error
 */
int vm_ref_copy(void *pgd, void *new_pgd, int make_ro);

/** @brief roll back the ref copy
 *
 *  @param pgd the pgd
 *  @return 0 on success, -1 on error
 */
void vm_ref_copy_rollback(void *pgd);

/** @brief copy a frame based on linear_addr and make writable if instructed
 *
 *  @param pgd the pgd
 *  @param linear_addr the address where we want to copy frame
 *  @param make_writable if not 0, make the frame writable
 *  @return 0 on success, -1 on error
 */
int vm_frm_copy(void *pgd, void *linear_addr, int make_writable);

/** @brief check a range of linear address and return the access rights
 *
 *  @param pcb the pcb we want to check
 *  @param pgd the pgd of pcb
 *  @param start_addr the start address we want to check from
 *  @param len the length of memory region
 *  @return 1 if writable, 0 if read-only, -1 if not accessible or error
 */
int vm_mem_region_check(pcb_t *pcb, void *pgd, void *start_addr, int len);

/** @brief check a NULL-terminated array and return the access rights
 *
 *  @param pcb the pcb we want to check
 *  @param pgd the pgd of pcb
 *  @param elem_size the element size of the array
 *  @param start the start address we want to check from
 *  @return 1 if writable, 0 if read-only, -1 if not accessible or error
 */
int vm_mem_array_check(pcb_t *pcb, void *pgd, int elem_size, void *start);

/** @brief check a linear address and return the access rights
 *
 *  @param pcb the pcb we want to check
 *  @param pgd the pgd of pcb
 *  @param linear_addr the linear address we want to check at
 *  @return 1 if writable, 0 if read-only, -1 if not accessible or error
 */
int vm_mem_check(pcb_t *pcb, void *pgd, void *linear_addr);

#endif
