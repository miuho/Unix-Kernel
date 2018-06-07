/** @file kernel.c
 *  @brief An initial kernel.c
 *
 *  You should initialize things in kernel_main(),
 *  and then run stuff.
 *
 *  @author Hingon Miu (hmiu)
 *  @author An Wu (anwu)
 */

#include <common_kern.h>

/* libc includes. */
#include <stdio.h>
#include <simics.h>                 /* lprintf() */

/* multiboot header file */
#include <multiboot.h>              /* boot_info */

/* x86 specific includes */
#include <x86/asm.h>                /* enable_interrupts() */

/* trap handlers init */
#include <trap_install.h>

/* initialize virtual memory */
#include <vm.h>

#include <loader.h>

#include <driver.h>
#include <pcb.h>
#include <tcb.h>
#include <kthread_pool.h>
#include <context_switch.h>
#include <sched.h>
#include <console.h>
#include <pgfault.h>
#include <circ_buf.h>
#include <syscall.h>
#include <syscall_handler.h>
#include <reporter.h>
#include <pgtable.h>
#include <x86/cr.h>
#include <malloc_init.h>

static char *tag = "kernel";

ktcb_t *running_ktcb = NULL;
unsigned int ticks_global;

void do_something(unsigned int ticks) {
    ticks_global = ticks;
}

/** @brief Kernel entrypoint.
 *  
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{
    /*
     * When kernel_main() begins, interrupts are DISABLED.
     * You should delete this comment, and enable them --
     * when you are ready.
     */
    report_progress(tag, "Hello from a brand new kernel!" );
   
    /* initialize IDT entries */
    trap_install();
    report_progress(tag, "trap install done!");
    
    /* initialize the locks ht for scheduler to store locks */ 
    sched_pcbs_sht_init();

    handler_install(do_something);
    report_progress(tag, "handler install done!");

    pgfault_handler_install();
    report_progress(tag, "pgfault_handler install done!");

    /* enable interrupts */
    enable_interrupts();

    /* initialize syscalls */
    syscall_init();

    // intialize virtual memory
    report_progress(tag, "going to init vm..");
    vm_init();

    // initialize pcb block for running first process
    report_progress(tag, "going to init pcb pool and kthread pool...");
    pcb_pool_init();
    kthr_init();
    
    report_progress(tag, "going to init sched");
    sched_init();

    /* initialize the circular buffer for console */
    report_progress(tag, "going to init console");
    cons_init();

    /* load the first program */
    report_progress(tag, "going to load prog...");
    ktcb_t *ktcb1 = kthr_alloc();
    load_prog("init", NULL, NULL, 0, ktcb1);
   
    /* set up the context switch stack */ 
    setup_exec_stack(ktcb1);

    sched_running_to_runnable(ktcb1);
   
    /* initialize the malloc mutex */
    malloc_init();
    
    report_progress(tag, "going into sched_run");
        
    sched_run();

    /* shouldn't reach here */

    return 0;
}
