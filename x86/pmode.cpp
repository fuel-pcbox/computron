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