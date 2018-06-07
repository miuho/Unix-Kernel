/** @file kern/inc/timer_driver.h
 *  
 *  @brief implementation of device-driver library for timer.
 *
 *
 *  @author An Wu (anwu)
 *  @author HingOn Miu (hmiu)
 *
 */

#ifndef _KERN_INC_TIMER_DRIVER_H_
#define _KERN_INC_TIMER_DRIVER_H_


/**
 * @brief init the timer handler
 *
 * @param idt_base_p the idt base
 *
 * @return Void
 *
 */
void timer_init(void *idt_base_p, void (*tickback)(unsigned int));

#endif
