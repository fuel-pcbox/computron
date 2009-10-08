/* 8086/misc.cpp
 * Miscellaneous instructions
 *
 */

#include "vomit.h"
#include "debug.h"
#include <unistd.h>
#include <stdlib.h>

void _NOP(vomit_cpu_t *cpu)
{
}

void _HLT(vomit_cpu_t *cpu)
{
    cpu->state = CPU_HALTED;
    vlog(VM_CPUMSG, "%04X:%04X Halted", cpu->base_CS, cpu->base_IP);
}

void _XLAT(vomit_cpu_t *cpu)
{
    cpu->regs.B.AL = vomit_cpu_memory_read8(cpu, *(cpu->CurrentSegment), cpu->regs.W.BX + cpu->regs.B.AL);
}

void _CS(vomit_cpu_t *cpu)
{
    SET_SEGMENT_PREFIX(cpu, CS);
    cpu->opcode_handler[vomit_cpu_pfq_getbyte(cpu)](cpu);
    RESET_SEGMENT_PREFIX(cpu);
}

void _DS(vomit_cpu_t *cpu)
{
    SET_SEGMENT_PREFIX(cpu, DS);
    cpu->opcode_handler[vomit_cpu_pfq_getbyte(cpu)](cpu);
    RESET_SEGMENT_PREFIX(cpu);
}

void _ES(vomit_cpu_t *cpu)
{
    SET_SEGMENT_PREFIX(cpu, ES);
    cpu->opcode_handler[vomit_cpu_pfq_getbyte(cpu)](cpu);
    RESET_SEGMENT_PREFIX(cpu);
}

void _SS(vomit_cpu_t *cpu)
{
    SET_SEGMENT_PREFIX(cpu, SS);
    cpu->opcode_handler[vomit_cpu_pfq_getbyte(cpu)](cpu);
    RESET_SEGMENT_PREFIX(cpu);
}

void _LAHF(vomit_cpu_t *cpu)
{
    cpu->regs.B.AH = cpu->CF | (cpu->PF * 4) | (cpu->AF * 16) | (cpu->ZF * 64) | (cpu->SF * 128) | 2;
}

void _SAHF(vomit_cpu_t *cpu)
{
    cpu->CF = (cpu->regs.B.AH & 0x01) != 0;
    cpu->PF = (cpu->regs.B.AH & 0x04) != 0;
    cpu->AF = (cpu->regs.B.AH & 0x10) != 0;
    cpu->ZF = (cpu->regs.B.AH & 0x40) != 0;
    cpu->SF = (cpu->regs.B.AH & 0x80) != 0;
}

void _XCHG_AX_reg16(vomit_cpu_t *cpu)
{
    swap(*cpu->treg16[cpu->opcode & 7], cpu->regs.W.AX);
}

void _XCHG_reg8_RM8(vomit_cpu_t *cpu)
{
    BYTE rm = vomit_cpu_pfq_getbyte(cpu);
    BYTE &reg(*cpu->treg8[rmreg(rm)]);

    BYTE value = vomit_cpu_modrm_read8(cpu, rm);
    BYTE tmp = reg;
    reg = value;
    vomit_cpu_modrm_update8(cpu, tmp);
}

void _XCHG_reg16_RM16(vomit_cpu_t *cpu)
{
    BYTE rm = vomit_cpu_pfq_getbyte(cpu);
    WORD value = vomit_cpu_modrm_read16(cpu, rm);
    WORD tmp = *cpu->treg16[rmreg(rm)];
    *cpu->treg16[rmreg(rm)] = value;
    vomit_cpu_modrm_update16(cpu, tmp);
}

void _DEC_reg16(vomit_cpu_t *cpu)
{
    WORD &reg(*cpu->treg16[cpu->opcode & 7]);

    DWORD i = reg;

    /* Overflow if we'll wrap. */
    cpu->OF = reg == 0x8000;

    --i;
    vomit_cpu_setAF(cpu, i, reg, 1);
    vomit_cpu_update_flags16(cpu, i);
    --reg;
}

void _INC_reg16(vomit_cpu_t *cpu)
{
    WORD &reg(*cpu->treg16[cpu->opcode & 7]);
    DWORD i = reg;

    /* Overflow if we'll wrap. */
    cpu->OF = i == 0x7FFF;

    ++i;
    vomit_cpu_setAF(cpu, i, reg, 1);
    vomit_cpu_update_flags16(cpu, i);
    ++reg;
}

void _INC_RM16(vomit_cpu_t *cpu)
{
    WORD value = vomit_cpu_modrm_read16(cpu, cpu->rmbyte);
    DWORD i = value;

    /* Overflow if we'll wrap. */
    cpu->OF = value == 0x7FFF;

    ++i;
    vomit_cpu_setAF(cpu, i, value, 1);
    vomit_cpu_update_flags16(cpu, i);
    vomit_cpu_modrm_update16(cpu, value + 1);
}

void _DEC_RM16(vomit_cpu_t *cpu)
{
    WORD value = vomit_cpu_modrm_read16(cpu, cpu->rmbyte);
    DWORD i = value;

    /* Overflow if we'll wrap. */
    cpu->OF = value == 0x8000;

    --i;
    vomit_cpu_setAF(cpu, i, value, 1); // XXX: i can be (dword)(-1)...
    vomit_cpu_update_flags16(cpu, i);
    vomit_cpu_modrm_update16(cpu, value - 1);
}
