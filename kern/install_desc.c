/** @file kern/install_desc.c
 *
 *  @brief install idt table descriptor manipulation
 *
 *  @author Hingon Miu (hmiu@andrew.cmu.edu)
 *  @author An Wu (anwu@andrew.cmu.edu)
 */

#include <install_desc.h>
#include <stdint.h>
#include <seg.h>

#define GATE_MISC 0b1000111000000000 // P, DPL, etc in descriptor
#define TRAP_MASK 0b100000000
#define INTERRUPT_MASK 0b0
#define SET_DPL(misc, x) ((misc) | (x << 13))
#define LOW_5(x) ((x) & 0b11111)     // get the least-sig 5 bits of a number

typedef struct {
    uint16_t offset_low;
    uint16_t seg_sel;
    uint16_t misc;
    uint16_t offset_hi;
}__attribute__((packed)) gate_desc;

void install_desc(void *base, int index, void (*handler)(), 
                    enum GATE_TYPE gate_type, int dpl) {
    /* cast to 64-bit get_desc type */
    gate_desc *gd_base = (gate_desc *)base;
    gate_desc gd = gd_base[index];

    gd.seg_sel = (uint16_t)SEGSEL_KERNEL_CS;
    gd.offset_low = ((uint32_t)handler) & 0xffff;
    gd.offset_hi = ((uint32_t)handler) >> 16;

    int lower5 = LOW_5(gd.misc);
    gd.misc = (uint16_t)GATE_MISC;
    gd.misc |= lower5;
    gd.misc = SET_DPL(gd.misc, dpl);
    if (gate_type == trap_gate)
        gd.misc |= TRAP_MASK;
    else
        gd.misc |= INTERRUPT_MASK;

    /* save back */
    gd_base[index] = gd;
}

