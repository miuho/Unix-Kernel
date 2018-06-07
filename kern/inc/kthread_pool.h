/** @file kern/inc/kthread_pool.h
 *
 *  @brief defines the kernel thread interface.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_KTHREAD_POOL_H_
#define _KERN_INC_KTHREAD_POOL_H_

#include <queue.h>
#include <common_kern.h>
#include <stdio.h>
#include <simics.h>
#include <malloc.h>
#include <string.h>
#include <reg.h>
#include <tcb.h>
#include <timed_queue.h>

/* the initial kernel pool size */
#define KTHREAD_POOL_SIZE 10

/* the kernel thread struct */
typedef struct ktcb {
    /* the registers of this kernel thread */
    reg_t *regs;

    /* the thread that kernel thread is bind to */
    tcb_t *tcb;
    
    node_t t_n;
      
    /* st_queue nodes and st_hash table entries */
    /* so that inserting to the hash table and*/
    /* queue do not need to dynamically allocate */
    /* memory */   
    struct st_node c_n;
    struct st_node m_n;
    struct st_node k_n;    
    struct st_node w_n;
    struct st_node r_n; 
    sht_entry_t w_e;

    /* the mutex that this kernel thread is blocking on */
    mutex_t *blocked_mutex;
} ktcb_t;

/** @brief init the kernel threads pool
 *
 *  @return 0 on success, -1 on error
 */
int kthr_init(void);

/** @brief allocate a kernel thread
 *
 *  @return pointer to the kernel thread on success, NULL on failure
 */
ktcb_t *kthr_alloc(void);

/** @brief free a kernel thread
 *
 *  @param ktcb the kernel thread we want to free
 *  @return Void
 */
void kthr_free(ktcb_t *ktcb);

/** @brief build relation between a kernel thread and a user thread
 *  
 *  @param ktcb the kernel thread
 *  @param tcb the user thread
 *  @return Void
 */
void kthr_build_relation(ktcb_t *ktcb, tcb_t *tcb);

#endif
