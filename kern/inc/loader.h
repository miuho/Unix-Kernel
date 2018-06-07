/** @file kern/inc/loader.h
 *  @brief This file defines the program loader interface.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_LOADER_H_
#define _KERN_INC_LOADER_H_

/* the user stack base (highest address) */
#define USER_STACK_BASE 0xc0000000

#include <pcb.h>
#include <kthread_pool.h>

/* the running kernel thread */
extern ktcb_t *running_ktcb;

/* the timer ticks */
extern unsigned int ticks_global;

/* --- Prototypes --- */
/** @brief get size bytes from offset from the filename into buf
 *
 *  @param filename the filename of the binary we want to load
 *  @param offset the offset we want to load from
 *  @param size the size of bytes we want to load
 *  @param buf the buffer to put into
 *  @return the number of bytes loaded. -1 on error
 */
int getbytes( const char *filename, int offset, int size, char *buf );

/*
 * Declare your loader prototypes here.
 */


/** @brief load a prog into a pcb.
 *         if pcb param is supplied, update it using the new prog
 *
 *  @param filename the filename of the program we want to load
 *  @param argvec the argvec for the program
 *  @param pcb if supplied, the pcb we want to replace
 *  @param tid if pcb supplied, the thread id of the root thread we want to use
 *  @param ktcb the kernel thread we want to bind the root thread to
 *  @return the pointer to pcb_t on success, NULL on error
 */
pcb_t *load_prog(char *filename, char **argvec, pcb_t *pcb, 
                 int tid, ktcb_t *ktcb);

/** @brief setup up a "context switch"-enabled kernel stack for a blank program
 *
 *  @param ktcb the kernel thread we want to load for
 *  @return Void
 */
void setup_exec_stack(ktcb_t *ktcb);

#endif /* _LOADER_H */
