/* 8086/interrupt.cpp
 * Interrupt instructions
 *
 */

#include "vomit.h"
#include "debug.h"

#ifdef VOMIT_DOS_ON_LINUX_IDLE_HACK
#include <sched.h>
#endif

extern void bios_interrupt10();

void _INT_imm8(VCpu* cpu)
{
    BYTE isr = cpu->fetchOpcodeByte();
    cpu->jumpToInterruptHandler(isr);
}

void _INT3(VCpu* cpu)
{
    cpu->jumpToInterruptHandler(3);
}

void _INTO(VCpu* cpu)
{
    /* XXX: I've never seen this used, so it's probably good to log it. */
    vlog(VM_ALERT, "INTO used, can you believe it?");

    if (cpu->getOF())
        cpu->jumpToInterruptHandler(4);
}

void _IRET(VCpu* cpu)
{
    WORD nip = cpu->pop();
    WORD ncs = cpu->pop();
    cpu->jump(ncs, nip);
    cpu->setFlags(cpu->pop());
}

void VCpu::jumpToInterruptHandler(int isr)
{
#ifdef VOMIT_DEBUG
    if (trapint)
        vlog(VM_PICMSG, "%04X:%04X Interrupt %02X,%02X trapped", this->base_CS, this->base_IP, isr, this->regs.B.AH);

    if (isr == 0x06) {
        vlog(VM_CPUMSG, "Invalid opcode trap at %04X:%04X (%02X)", this->base_CS, this->base_IP, codeMemory() + this->base_IP);
    }
#endif

#ifdef VOMIT_DOS_ON_LINUX_IDLE_HACK
    if (isr == 0x28) {
        /* DOS idle interrupt, catch a quick rest! */
        sched_yield();
    }
#endif

    if (isr == 0x10) {
        /* Call C video BIOS (faster and more complete ATM) */
        bios_interrupt10();
        return;
    }

    push(getFlags());
    setIF(0);
    setTF(0);
    push(getCS());
    push(getIP());

    WORD segment = (this->memory[isr * 4 + 3] << 8) | this->memory[isr * 4 + 2];
    WORD offset = (this->memory[isr * 4 + 1] << 8) | this->memory[isr * 4];

    jump(segment, offset);
}
