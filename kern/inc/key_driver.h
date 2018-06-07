/** @file kern/inc/key_driver.h
 *
 *  @brief This file defines the key driver interface.
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#ifndef _KERN_INC_KEY_DRIVER_H_
#define _KERN_INC_KEY_DRIVER_H_

/** @brief init the key driver
 *
 *  @param idt_base_p the idt base
 *  @return 0 on success, -1 on error
 */
int key_init(void *idt_base_p);

/** @brief read the next char from the buffer
 *
 *  @return the char if there's one, -1 if not
 */
int key_readchar();

#endif
