/** @file kern/inc/pcb.h
 *
 *  @brief the process control block interfaces
 *
 *  @author HingOn Miu (hmiu)
 *  @author An Wu (anwu)
 */

#ifndef _KERN_INC_PCB_H_
#define _KERN_INC_PCB_H_

#include <hash.h>
#include <st_hash.h>
#include <reg.h>
#include <cond.h>
#include <mutex.h>

/* the pid_t declaration */
typedef int pid_t;

/* the pcb_t type declaration */
typedef struct pcb pcb_t;

/* the ktcb type declaration */
struct ktcb;

/* the process control block struct */
struct pcb {
    /* process id */
    pid_t pid;

    /* threads */
    ht_t *tcb_ht;
    
    /* track allocated pages */
    ht_t *new_pages_ht;
    
    /* process parent & children */
    pcb_t *parent;
    ht_t *children;

    /* process info */
    unsigned long pgd;

    /* st_queue node and st_hash table entry */
    /* so that insert to hash table does not need */
    /* to dynamically allocate memory */
    struct st_node n;
    sht_entry_t e;


    int exit_status;

    /* for wait system call */
    cond_t wait_cond;
    
    /* track read only memory regions */
    void *txt_base;
    int txt_len;
    void *rodat_base;
    int rodat_len;

    int exited_thread_count;
};

/** @brief generate a tid
 *
 *  @return the generated tid
 */
int generate_tid();

/** @brief init the pcb pool
 *
 *  @return 0 on success, -1 on error
 */
int pcb_pool_init(void);

/** @brief the pid compare function for hash table
 *
 *  @param e1 the first hash table entry
 *  @param e2 the second hash table entry
 *  @return 1 if e1->key > e1->key, 0 if equal, -1 if less
 */
int key_compare_pid(ht_entry_t *e1, ht_entry_t *e2);

/** @brief check if the threads of a process are all exited
 *
 *  @param e the hash table entry containing pcb
 *  @return 1 if all exited, 0 if not, -1 on error
 */
int pcb_all_thr_exited(ht_entry_t *e);

/** @brief announce a death of a parent by setting the parent of all its 
 *         children to sched pcb (so it can reap children from time to time)
 *
 *  @param e the hash table entry containing pcb
 *  @return 0 on success, -1 on error
 */
int announce_parent_death(ht_entry_t *e);

/** @brief track allocated new pages (by new_pages syscall) of a process
 *
 *  @param pcb the pcb we want to track for
 *  @param base the base of new_pages
 *  @param len the len of new_pages
 *  @return 0 on success, -1 on error
 */
int track_pages_allocated(pcb_t *pcb, void *base, int len);

/** @brief untrack allocated new pages (by new_pages syscall) of a process
 *
 *  @param pcb the pcb we want to untrack for
 *  @param base the base of new_pages
 *  @param len the len of new_pages
 *  @return 0 on success, -1 on error
 */
int untrack_pages_allocated(pcb_t *pcb, void *base);

/** @brief check if base is allocated by new_pages before 
 *
 *  @param pcb the pcb we want to untrack for
 *  @param base the base of new_pages
 *  @return 1 if allocated before, 0 if not, -1 on error
 */
int was_pages_allocated(pcb_t *pcb, void *base);

/** @brief create a pcb
 *
 *  @param pgd the pgd of this process
 *  @param regs the regs to give to root thread
 *  @param parent the parent of this pcb
 *  @param ktcb the kernel thread to bind to root thread
 *  @param txt_base the txt_base of process
 *  @param txt_len the length of txt section
 *  @param rodat_base the rodat_base of process
 *  @param rodat_len the length of rodat section
 *  @return pointer to pcb on success, NULL on error
 */
pcb_t *pcb_create(unsigned long pgd, reg_t *regs, pcb_t *parent, 
                    struct ktcb *ktcb,
                    void *txt_base, int txt_len, void *rodat_base,
                    int rodat_len);

/** @brief update a pcb (free the old structures)
 *
 *  @param pcb the pcb we want to update
 *  @param pgd the pgd of this process
 *  @param regs the regs to give to root thread
 *  @param tid the tid to use for root thread
 *  @param ktcb the kernel thread to bind to root thread
 *  @param txt_base the txt_base of process
 *  @param txt_len the length of txt section
 *  @param rodat_base the rodat_base of process
 *  @param rodat_len the length of rodat section
 *  @return pointer to pcb on success, NULL on error
 */

pcb_t *pcb_update(pcb_t *pcb, unsigned long pgd, reg_t *regs, int tid,
                  struct ktcb *ktcb, 
                  void *txt_base, int txt_len, void *rodat_base,
                  int rodat_len);

/** @brief free a pcb and its internal structures
 *
 *  @param pcb the pcb we want to free
 *  @return Void
 */
void pcb_free(pcb_t *pcb);

#endif /* _KERN_INC_PCB_H_ */
