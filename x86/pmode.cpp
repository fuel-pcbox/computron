#include "vcpu.h"

void _SGDT(VCpu* cpu)
{
    WORD tableAddress = cpu->fetchOpcodeWord();
    cpu->writeMemory32(cpu->currentSegment(), tableAddress + 2, cpu->GDTR.base);
    cpu->writeMemory16(cpu->currentSegment(), tableAddress, cpu->GDTR.limit);
}

void _SIDT(VCpu* cpu)
{
    WORD tableAddress = cpu->fetchOpcodeWord();
    cpu->writeMemory32(cpu->currentSegment(), tableAddress + 2, cpu->IDTR.base);
    cpu->writeMemory16(cpu->currentSegment(), tableAddress, cpu->IDTR.limit);
}

void _LGDT(VCpu* cpu)
{
    WORD tableAddress = cpu->fetchOpcodeWord();
    cpu->GDTR.base = cpu->readMemory32(cpu->currentSegment(), tableAddress + 2);
    cpu->GDTR.limit = cpu->readMemory16(cpu->currentSegment(), tableAddress);
}

void _LIDT(VCpu* cpu)
{
    WORD tableAddress = cpu->fetchOpcodeWord();
    cpu->IDTR.base = cpu->readMemory32(cpu->currentSegment(), tableAddress + 2);
    cpu->IDTR.limit = cpu->readMemory16(cpu->currentSegment(), tableAddress);
}
