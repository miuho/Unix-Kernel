/** @file kern/inc/tcb.h
 *  @brief implementation of the Thread Control Block
 *
 *
 *  @author HingOn Miu (hmiu)
 *  @author An Wu (anwu)
 *  @bug No known bugs.
 */

#ifndef _KERN_INC_TCB_H_
#define _KERN_INC_TCB_H_

#include <reg.h>
#include <pcb.h>
#include <hash.h>
#include <syscall.h>

#define TCB_RUNNING 0
#define TCB_EXITED 1

/* define the type of thread's tid */
typedef int tid_t;
/* define the struct pointer for kernel's tcb */
struct ktcb;

typedef struct tcb {
    /* thread info */
    tid_t tid;
    int state;

    /* registers */
    reg_t *regs;

    /* associated pcb */
    pcb_t *pcb;
    
    /* associated ktcb */
    struct ktcb *ktcb;

    /* swexn */
    swexn_handler_t swexn_eip;
    void *swexn_esp3;
    void *swexn_arg;
} tcb_t;

/** 
 * @brief creates a TCB
 *
 * @param pcb the PCB pointer for a process
 * @param tcb_ht the process's TCB hashtable this TCB belongs to
 * @param regs the registers for the thread
 * @param tcb_count the number of TCB which is used as a unique tid
 * @param ktcb the kernel TCB that manages this TCB.
 *
 * @return Void.
 *
 */
void *tcb_create(pcb_t *pcb, ht_t *tcb_ht, reg_t *regs, int tcb_count, 
                    struct ktcb *ktcb);

/**
 * @brief compare two hashtable entry's tid 
 *
 * @param e1 the first entry
 * @param e2 the second entry
 * @return 1 if identical, 0 otherwise.
 */
int key_compare_tid(ht_entry_t *e1, ht_entry_t *e2);

/**
 * @brief get the root thread's TCB pointer for a process
 *
 * @param pcb the PCB pointer
 * @return the TCB pointer
 */
tcb_t *get_root_thr(pcb_t *pcb);

/**
 * @brief free the hashtable entry, given that it is a TCB
 *
 * @param e the hashtable entry
 * @return 1 if successful, 0 otherwise
 *
 */
int tcb_free(ht_entry_t *e);

/**
 * @brief terminate the hashtable entry, given that it is a TCB
 *
 * @param e the hashtable entry
 * @return 0 if successful, -1 otherwise. 
 *
 */
int kill_tcb(ht_entry_t *e);

#endif /* _TCB_H */
