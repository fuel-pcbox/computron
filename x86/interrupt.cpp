// x86/interrupt.cpp
// Interrupt instructions

#include "vomit.h"
#include "vcpu.h"
#include "debug.h"

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
    cpu->jump16(ncs, nip);
    cpu->setFlags(cpu->pop());
}

void VCpu::jumpToInterruptHandler(int isr)
{
#ifdef VOMIT_DEBUG
    if (options.trapint)
        vlog(VM_PICMSG, "%04X:%08X Interrupt %02X,%02X trapped", getBaseCS(), getBaseEIP(), isr, this->regs.B.AH);

    if (isr == 0x06) {
        vlog(VM_CPUMSG, "Invalid opcode trap at %04X:%08X (%02X)", getBaseCS(), getBaseEIP(), *(codeMemory() + this->getBaseIP()));
    }
#endif

#ifdef VOMIT_C_VGA_BIOS
    if (isr == 0x10) {
        /* Call C video BIOS (faster and more complete ATM) */
        bios_interrupt10();
        return;
    }
#endif

    push(getFlags());
    setIF(0);
    setTF(0);
    push(getCS());
    push(getIP());

    WORD segment = (this->memory[isr * 4 + 3] << 8) | this->memory[isr * 4 + 2];
    WORD offset = (this->memory[isr * 4 + 1] << 8) | this->memory[isr * 4];

    jump16(segment, offset);
}
