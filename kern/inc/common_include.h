/*
 * @file kern/inc/common_include.h
 *
 * @brief header file to include other header files for 
 *        convenience.
 *
 * @author HingOn Miu (hmiu)
 * @author An Wu (anwu)
 *
 */

#ifndef _KERN_INC_COMMON_INCLUDE_H_
#define _KERN_INC_COMMON_INCLUDE_H_

#include <simics.h>
#include <stddef.h>
#include <string.h>

#include <x86/cr.h>
#include <x86/asm.h>
#include <x86/eflags.h>
#include <x86/seg.h>
#include <x86/idt.h>

#include <console.h>
#include <driver.h>
#include <ureg.h>

#include <reporter.h>
#include <loader.h>
#include <pgtable.h>
#include <sched.h>
#include <context_switch.h>
#include <reg.h>
#include <vm.h>
#include <asm.h>
#include <mode_switch.h>
#include <fault.h>
#include <install_desc.h>
#include <pgfault.h>
#include <pgfault_wrapper.h>
#include <if_flag.h>
#include <syscall_handler.h>

#endif
