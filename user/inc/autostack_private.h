/** @file autostack_private.h
 *
 *  @brief private variables for autostack
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _AUTOSTACK_H
#define _AUTOSTACK_H

extern void *root_stack_lo;     /* the low address of root stack */
extern void *root_stack_hi;     /* the high address of root stack */
extern void *exn_stack_base;    /* the exception stack base */

#endif
