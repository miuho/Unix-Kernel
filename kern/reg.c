/** @file kern/reg.c
 *
 *  @brief register (normal reg and ureg) manipulation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <reg.h>
#include <stddef.h>
#include <simics.h>
#include <malloc.h>
#include <reporter.h>
#include <if_flag.h>
#include <console.h>
#include <loader.h>

static char *tag = "reg";

reg_t *reg_copy(reg_t *reg) {
    
    if (reg == NULL) {
        report_error(tag, "reg.c: given reg is NULL");
        return NULL;
    }

    /* allocate a new reg_t */
    reg_t *new_reg = calloc(1, sizeof(reg_t));
    
    if (new_reg == NULL) {
        report_error(tag, "reg.c:an't alloc new reg");
        return NULL;
    }

    new_reg->edi = reg->edi;
    new_reg->esi = reg->esi;
    new_reg->ebp = reg->ebp;
    new_reg->ebx = reg->ebx;
    new_reg->edx = reg->edx;
    new_reg->ecx = reg->ecx;
    new_reg->eax = reg->eax;

    new_reg->eip = reg->eip;
    new_reg->eflags = reg->eflags;
    new_reg->esp = reg->esp;

    new_reg->esp0 = reg->esp0;

    return new_reg;
}

int ureg_create(ureg_t *ureg,
                unsigned int cause, unsigned cr2, unsigned ds, 
                unsigned int es, unsigned int fs, unsigned int gs,
                unsigned int edi, unsigned int esi, unsigned int ebp,
                unsigned int ebx, unsigned int edx, unsigned int ecx,
                unsigned int eax, unsigned int error_code, 
                unsigned int eip, unsigned int cs, unsigned int eflags,
                unsigned int esp, unsigned int ss) {
    
    if (ureg == NULL) {
        report_error(tag, "ureg_create: gets NULL");
        return -1;
    }

    ureg->cause = cause;
    ureg->cr2 = cr2;

    ureg->ds = ds;
    ureg->es = es;
    ureg->fs = fs;
    ureg->gs = gs;

    ureg->edi = edi;
    ureg->esi = esi;
    ureg->ebp = ebp;
    ureg->zero = 0;
    ureg->ebx = ebx;
    ureg->edx = edx;
    ureg->ecx = ecx;
    ureg->eax = eax;

    ureg->error_code = error_code;
    ureg->eip = eip;
    ureg->cs = cs;
    ureg->eflags = eflags;
    ureg->esp = esp;
    ureg->ss = ss;

    return 0;
}

void print_ureg(ureg_t *ureg) {

    int len = 0;
    char buf[42];

    switch (ureg->cause) {
        case SWEXN_CAUSE_PAGEFAULT:
            len = sprintf(buf, 
                " Page Fault: Invalid read from non-");
            putbytes(buf, len);
            len = sprintf(buf, 
                " present page at address 0x%08x\n",
                ureg->cr2);
            break;
        case SWEXN_CAUSE_DIVIDE:
            len = sprintf(buf, " Divide: \n");
            break;       
        case SWEXN_CAUSE_DEBUG:
            len = sprintf(buf, " Debug: \n");
            break;        
        case SWEXN_CAUSE_BREAKPOINT:
            len = sprintf(buf, " Breakpoint: \n");
            break;        
        case SWEXN_CAUSE_OVERFLOW:
            len = sprintf(buf, " Overflow: \n");
            break;        
        case SWEXN_CAUSE_BOUNDCHECK:
            len = sprintf(buf, " Boundcheck: \n");
            break;        
        case SWEXN_CAUSE_OPCODE:
            len = sprintf(buf, " Opcode: \n");
            break;        
        case SWEXN_CAUSE_NOFPU:
            len = sprintf(buf, " No Fpu: \n");
            break;
        case SWEXN_CAUSE_SEGFAULT:
            len = sprintf(buf, " Seg Fault: \n");
            break;           
        case SWEXN_CAUSE_STACKFAULT:
            len = sprintf(buf, " Stack Fault: \n");
            break;       
        case SWEXN_CAUSE_PROTFAULT:
            len = sprintf(buf, " Prot Fault: \n");
            break;
        case SWEXN_CAUSE_FPUFAULT:
            len = sprintf(buf, " Fpu Fault: \n");
            break;
        case SWEXN_CAUSE_ALIGNFAULT:
            len = sprintf(buf, " Align Fault: \n");
            break;
        case SWEXN_CAUSE_SIMDFAULT:
            len = sprintf(buf, " Simd Fault: \n");
            break;
        default:
            break;
    }
    if (len != 0)
        putbytes(buf, len);
    
    len = sprintf(buf , " Thread: %d\n", running_ktcb->tcb->tid);
    putbytes(buf, len);
    
    len = sprintf(buf , " Registers:\n");
    putbytes(buf, len);
    
    len = sprintf(buf , " eax: 0x%08x, ebx: 0x", ureg->eax);
    putbytes(buf, len);
    len = sprintf(buf , "%08x, ecx: 0x%08x\n", ureg->ebx, ureg->ecx);
    putbytes(buf, len);
    
    len = sprintf(buf , " edx: 0x%08x, edi: 0x", ureg->edx);
    putbytes(buf, len);
    len = sprintf(buf , "%08x, esi: 0x%08x\n", ureg->edi, ureg->esi);
    putbytes(buf, len);
    
    len = sprintf(buf , " ebp: 0x%08x, esp: 0x", ureg->ebp);
    putbytes(buf, len);
    len = sprintf(buf , "%08x, eip: 0x%08x\n", ureg->esp, ureg->eip);
    putbytes(buf, len);
    
    len = sprintf(buf , " ss:      0x%04x, cs:      0x", ureg->ss);
    putbytes(buf, len);
    len = sprintf(buf , "%04x, ds:      0x%04x\n", ureg->cs, ureg->ds);
    putbytes(buf, len);
    
    len = sprintf(buf , " es:      0x%04x, fs:      0x", ureg->es);
    putbytes(buf, len);
    len = sprintf(buf , "%04x, gs:      0x%04x\n", ureg->fs, ureg->gs);
    putbytes(buf, len);
    
    len = sprintf(buf , " eflags: 0x%08x\n", ureg->eflags);
    putbytes(buf, len);
    
    return;
}

