/* 8086/memory.c
 * Memory Functions
 *
 */

#include <stdlib.h>
#include <string.h>

#include "vomit.h"

void VCpu::push(WORD value)
{
    this->regs.W.SP -= 2;
    writeMemory16(getSS(), this->regs.W.SP, value);
}

WORD VCpu::pop()
{
    WORD w = readMemory16(getSS(), this->regs.W.SP);
    this->regs.W.SP += 2;
    return w;
}

void _LDS_reg16_mem16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    DWORD value = vomit_cpu_modrm_read32(cpu, rm);
    *cpu->treg16[rmreg(rm)] = LSW(value);
    cpu->DS = MSW(value);
}
void _LES_reg16_mem16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    DWORD value = vomit_cpu_modrm_read32(cpu, rm);
    *cpu->treg16[rmreg(rm)] = LSW(value);
    cpu->ES = MSW(value);
}
