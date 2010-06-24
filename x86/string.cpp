// x86/string.c
// String instructions

#include "vcpu.h"

void _LODSB(VCpu* cpu)
{
    cpu->regs.B.AL = cpu->readMemory8(cpu->currentSegment(), cpu->regs.W.SI);

    if (cpu->getDF() == 0)
        ++cpu->regs.W.SI;
    else
        --cpu->regs.W.SI;
}

void _LODSW(VCpu* cpu)
{
    cpu->regs.W.AX = cpu->readMemory16(cpu->currentSegment(), cpu->regs.W.SI);

    if (cpu->getDF() == 0)
        cpu->regs.W.SI += 2;
    else
        cpu->regs.W.SI -= 2;
}

void _STOSB(VCpu* cpu)
{
    cpu->writeMemory8(cpu->ES, cpu->regs.W.DI, cpu->regs.B.AL);

    if (cpu->getDF() == 0)
        ++cpu->regs.W.DI;
    else
        --cpu->regs.W.DI;
}

void _STOSW(VCpu* cpu)
{
    cpu->writeMemory16(cpu->ES, cpu->regs.W.DI, cpu->regs.W.AX);

    if (cpu->getDF() == 0)
        cpu->regs.W.DI += 2;
    else
        cpu->regs.W.DI -= 2;
}

void _STOSD(VCpu* cpu)
{
    if (cpu->addressSize() == VCpu::AddressSize32) {
        cpu->writeMemory32(cpu->ES, cpu->regs.D.EDI, cpu->regs.W.AX);
        cpu->regs.D.EDI += (cpu->getDF() ? -4 : 4);
    } else {
        cpu->writeMemory32(cpu->ES, cpu->regs.W.DI, cpu->regs.W.AX);
        cpu->regs.W.DI += (cpu->getDF() ? -2 : 2);
    }
}

void _CMPSB(VCpu* cpu)
{
    BYTE src = cpu->readMemory8(cpu->currentSegment(), cpu->regs.W.SI);
    BYTE dest = cpu->readMemory8(cpu->ES, cpu->regs.W.DI);

    cpu->cmpFlags8(src - dest, src, dest);

    if (cpu->getDF() == 0)
        ++cpu->regs.W.DI, ++cpu->regs.W.SI;
    else
        --cpu->regs.W.DI, --cpu->regs.W.SI;
}

void _CMPSW(VCpu* cpu)
{
    WORD src = cpu->readMemory16(cpu->currentSegment(), cpu->regs.W.SI);
    WORD dest = cpu->readMemory16(cpu->ES, cpu->regs.W.DI);

    cpu->cmpFlags16(src - dest, src, dest);

    if (cpu->getDF() == 0)
        cpu->regs.W.DI += 2, cpu->regs.W.SI += 2;
    else
        cpu->regs.W.DI -= 2, cpu->regs.W.SI -= 2;
}

void _SCASB(VCpu* cpu)
{
    BYTE dest = cpu->readMemory8(cpu->ES, cpu->regs.W.DI);

    cpu->cmpFlags8(cpu->regs.B.AL - dest, dest, cpu->regs.B.AL);

    if (cpu->getDF() == 0)
        ++cpu->regs.W.DI;
    else
        --cpu->regs.W.DI;
}

void _SCASW(VCpu* cpu)
{
    WORD dest = cpu->readMemory16(cpu->ES, cpu->regs.W.DI);

    cpu->cmpFlags16(cpu->regs.W.AX - dest, dest, cpu->regs.W.AX);

    if (cpu->getDF() == 0)
        cpu->regs.W.DI += 2;
    else
        cpu->regs.W.DI -= 2;
}

void _MOVSB(VCpu* cpu)
{
    BYTE tmpb = cpu->readMemory8(cpu->currentSegment(), cpu->regs.W.SI);
    cpu->writeMemory8(cpu->ES, cpu->regs.W.DI, tmpb);

    if (cpu->getDF() == 0)
        ++cpu->regs.W.SI, ++cpu->regs.W.DI;
    else
        --cpu->regs.W.SI, --cpu->regs.W.DI;
}

void _MOVSW(VCpu* cpu)
{
    WORD tmpw = cpu->readMemory16(cpu->currentSegment(), cpu->regs.W.SI);
    cpu->writeMemory16(cpu->ES, cpu->regs.W.DI, tmpw);

    if (cpu->getDF() == 0)
        cpu->regs.W.SI += 2, cpu->regs.W.DI += 2;
    else
        cpu->regs.W.SI -= 2, cpu->regs.W.DI -= 2;
}

void _MOVSD(VCpu* cpu)
{
    if (cpu->addressSize() == VCpu::AddressSize16) {
        DWORD tmpw = cpu->readMemory32(cpu->currentSegment(), cpu->regs.W.SI);
        cpu->writeMemory32(cpu->ES, cpu->regs.W.DI, tmpw);

        if (cpu->getDF() == 0)
            cpu->regs.W.SI += 4, cpu->regs.W.DI += 4;
        else
            cpu->regs.W.SI -= 4, cpu->regs.W.DI -= 4;
    } else {
        DWORD tmpw = cpu->readMemory32(cpu->currentSegment(), cpu->regs.D.ESI);
        cpu->writeMemory32(cpu->ES, cpu->regs.D.EDI, tmpw);

        if (cpu->getDF() == 0)
            cpu->regs.D.ESI += 4, cpu->regs.D.EDI += 4;
        else
            cpu->regs.D.ESI -= 4, cpu->regs.D.EDI -= 4;
    }
}
