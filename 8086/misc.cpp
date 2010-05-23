/* 8086/misc.cpp
 * Miscellaneous instructions
 *
 */

#include "vomit.h"
#include "debug.h"
#include <unistd.h>
#include <stdlib.h>

void _NOP(VCpu*)
{
}

void _HLT(VCpu* cpu)
{
    cpu->setState(VCpu::Halted);
    vlog(VM_CPUMSG, "%04X:%04X Halted", cpu->base_CS, cpu->base_IP);
}

void _XLAT(VCpu* cpu)
{
    cpu->regs.B.AL = vomit_cpu_memory_read8(cpu, *(cpu->CurrentSegment), cpu->regs.W.BX + cpu->regs.B.AL);
}

void _CS(VCpu* cpu)
{
    SET_SEGMENT_PREFIX(cpu, CS);
    cpu->opcode_handler[cpu->fetchOpcodeByte()](cpu);
    RESET_SEGMENT_PREFIX(cpu);
}

void _DS(VCpu* cpu)
{
    SET_SEGMENT_PREFIX(cpu, DS);
    cpu->opcode_handler[cpu->fetchOpcodeByte()](cpu);
    RESET_SEGMENT_PREFIX(cpu);
}

void _ES(VCpu* cpu)
{
    SET_SEGMENT_PREFIX(cpu, ES);
    cpu->opcode_handler[cpu->fetchOpcodeByte()](cpu);
    RESET_SEGMENT_PREFIX(cpu);
}

void _SS(VCpu* cpu)
{
    SET_SEGMENT_PREFIX(cpu, SS);
    cpu->opcode_handler[cpu->fetchOpcodeByte()](cpu);
    RESET_SEGMENT_PREFIX(cpu);
}

void _LAHF(VCpu* cpu)
{
    cpu->regs.B.AH = cpu->getCF() | (cpu->getPF() * 4) | (cpu->getAF() * 16) | (cpu->getZF() * 64) | (cpu->getSF() * 128) | 2;
}

void _SAHF(VCpu* cpu)
{
    cpu->setCF(cpu->regs.B.AH & 0x01);
    cpu->setPF(cpu->regs.B.AH & 0x04);
    cpu->setAF(cpu->regs.B.AH & 0x10);
    cpu->setZF(cpu->regs.B.AH & 0x40);
    cpu->setSF(cpu->regs.B.AH & 0x80);
}

void _XCHG_AX_reg16(VCpu* cpu)
{
    swap(*cpu->treg16[cpu->opcode & 7], cpu->regs.W.AX);
}

void _XCHG_reg8_RM8(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    BYTE &reg(*cpu->treg8[rmreg(rm)]);

    BYTE value = vomit_cpu_modrm_read8(cpu, rm);
    BYTE tmp = reg;
    reg = value;
    vomit_cpu_modrm_update8(cpu, tmp);
}

void _XCHG_reg16_RM16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    WORD value = vomit_cpu_modrm_read16(cpu, rm);
    WORD tmp = *cpu->treg16[rmreg(rm)];
    *cpu->treg16[rmreg(rm)] = value;
    vomit_cpu_modrm_update16(cpu, tmp);
}

void _DEC_reg16(VCpu* cpu)
{
    WORD &reg(*cpu->treg16[cpu->opcode & 7]);

    DWORD i = reg;

    /* Overflow if we'll wrap. */
    cpu->setOF(reg == 0x8000);

    --i;
    vomit_cpu_setAF(cpu, i, reg, 1);
    cpu->updateFlags16(i);
    --reg;
}

void _INC_reg16(VCpu* cpu)
{
    WORD &reg(*cpu->treg16[cpu->opcode & 7]);
    DWORD i = reg;

    /* Overflow if we'll wrap. */
    cpu->setOF(i == 0x7FFF);

    ++i;
    vomit_cpu_setAF(cpu, i, reg, 1);
    cpu->updateFlags16(i);
    ++reg;
}

void _INC_RM16(VCpu* cpu)
{
    WORD value = vomit_cpu_modrm_read16(cpu, cpu->rmbyte);
    DWORD i = value;

    /* Overflow if we'll wrap. */
    cpu->setOF(value == 0x7FFF);

    ++i;
    vomit_cpu_setAF(cpu, i, value, 1);
    cpu->updateFlags16(i);
    vomit_cpu_modrm_update16(cpu, value + 1);
}

void _DEC_RM16(VCpu* cpu)
{
    WORD value = vomit_cpu_modrm_read16(cpu, cpu->rmbyte);
    DWORD i = value;

    /* Overflow if we'll wrap. */
    cpu->setOF(value == 0x8000);

    --i;
    vomit_cpu_setAF(cpu, i, value, 1); // XXX: i can be (dword)(-1)...
    cpu->updateFlags16(i);
    vomit_cpu_modrm_update16(cpu, value - 1);
}
