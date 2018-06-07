/*
 *
 * @file kern/inc/sched.h
 * @brief header file for scheduler to manage kernel thread control
 *        blocks.
 *
 * @author HingOn Miu (hmiu@andrew.cmu.edu)
 * @author An Wu (hmiu@andrew.cmu.edu)
 *
 */

#ifndef _KERN_INC_SCHED_H_
#define _KERN_INC_SCHED_H_

#include <kthread_pool.h>
#include <pcb.h>

/* record the scheduler's dummy kernel thread control block pointer */
extern ktcb_t *sched_ktcb;

/**
 * @brief initialize the data structures for scheduler
 *
 * @return 0 if succesful, -1 otherwise.
 *
 */
int sched_init(void);

/**
 * @brief initialize the hash table for scheduler to 
 *        keep track of pcbs.
 *
 * @return 0 if succesful, -1 otherwise.
 *
 */
int sched_pcbs_sht_init(void);

/**
 * @brief add the PCB pointer to scheduler's PCB hashtable.
 * 
 * @param pcb the PCB pointer
 * @return Void.
 *
 */
void sched_add_pcb(pcb_t *pcb);

/**
 * @brief remove the PCB pointer to scheduler's PCB hashtable.
 * 
 * @param pcb the PCB pointer
 * @return 0 if succesful, -1 otherwise.
 *
 */
int sched_remove_pcb(pcb_t *pcb);

/**
 * @brief find the PCB pointer from scheduler's PCB hashtable.
 * 
 * @param pid the process's pid
 * @return the PCB pointer
 *
 */
pcb_t *sched_find_pcb(int pid);

/**
 * @brief the scheduler function run by the schduler kernel TCB
 *        to do the scheduling.
 * 
 * @return Void.
 *
 */
void sched_run();

/**
 * @brief add a process as a child process of the scheduler.
 * 
 * @param child the child process's PCB pointer
 * @return Void.
 *
 */
void sched_add_child(pcb_t *child);

/**
 * @brief the scheduling function that returns the next
 *        kernel TCB should be run.
 * 
 * @return the kernel TCB  pointer
 *
 */
ktcb_t *sched_next();

/**
 * @brief remove the kernel TCB pointer from the scheduler.
 * 
 * @param ktcb the kernel TCB pointer
 * @return 0 if successful, -1 otherwise.
 *
 */
int sched_delete(ktcb_t *ktcb);

/**
 * @brief get a runnable KTCB from scheduler. If input is -1,
 *        it returns the next runnable KTCB. Otherwise, it fetches
 *        the KTCB that manages the TCB with the input tid.
 * 
 * @param tid the indicated KTCB managed TCB's tid
 * @return 0 if successful, -1 otherwise.
 *
 */
ktcb_t *sched_runnable_to_running(int tid);

/**
 * @brief insert a runnable KTCB to scheduler.
 * 
 * @param ktcb the KTCB pointer
 * @return 0 if successful, -1 otherwise.
 *
 */
int sched_running_to_runnable(ktcb_t *ktcb);

/**
 * @brief insert a slept KTCB to scheduler.
 * 
 * @param ktcb the KTCB pointer
 * @param ticks the time when the thread wakes up
 * @return 0 if successful, -1 otherwise.
 *
 */
int sched_running_to_sleep(ktcb_t *ktcb, unsigned int ticks);

/**
 * @brief get a awake KTCB from scheduler.
 * 
 * @return the KTCB pointer if it is awaken, NULL otherwise.
 *
 */
ktcb_t *sched_sleep_to_running(void);

/**
 * @brief insert a descheduled KTCB to scheduler.
 * 
 * @param ktcb the KTCB pointer to be descheduled
 * @return 0 if successful, -1 otherwise.
 *
 */
int sched_running_to_waiting(ktcb_t *ktcb);

/**
 * @brief get a descheduled KTCB from scheduler.
 * 
 * @return the KTCB pointer if it was descheduled, NULL otherwise.
 *
 */
ktcb_t *sched_waiting_to_running(int tid);

/**
 * @brief check a KTCB is descheduled or not.
 * 
 * @param tid the TCB managed by KTCB's tid
 * @return the KTCB pointer if it was descheduled, NULL otherwise.
 *
 */
ktcb_t *sched_is_waiting(int tid);

#endif
