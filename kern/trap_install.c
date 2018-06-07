/** @file trap_install.c
 *
 *  @brief install trap handlers
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <x86/asm.h>
#include <common_handler.h>

void trap_install() {
    /* get the idt base */
    void *idt_base_p = idt_base();

    syscall_install(idt_base_p);

}
