/* 8086/mov.cpp
 * MOVe instructions
 *
 */

#include "vomit.h"
#include "debug.h"

void _MOV_RM8_imm8(vomit_cpu_t *cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    (void) vomit_cpu_modrm_resolve8(cpu, rm);
    vomit_cpu_modrm_update8(cpu, cpu->fetchOpcodeByte());
}

void _MOV_RM16_imm16(vomit_cpu_t *cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    (void) vomit_cpu_modrm_resolve16(cpu, rm);
    vomit_cpu_modrm_update16(cpu, cpu->fetchOpcodeWord());
}

void _MOV_RM16_seg(vomit_cpu_t *cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();

    VM_ASSERT(rmreg(rm) >= 0 && rmreg(rm) <= 5);

    vomit_cpu_modrm_write16(cpu, rm, *cpu->tseg[rmreg(rm)]);

#ifdef VOMIT_DEBUG
    if (rmreg(rm) == REG_FS || rmreg(rm) == REG_GS) {
        vlog(VM_CPUMSG, "%04X:%04X: Read from 80386 segment register");
    }
#endif
}

void _MOV_seg_RM16(vomit_cpu_t *cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();

    VM_ASSERT(rmreg(rm) >= 0 && rmreg(rm) <= 5);

    *cpu->tseg[rmreg(rm)] = vomit_cpu_modrm_read16(cpu, rm);

#ifdef VOMIT_DEBUG
    if (rmreg(rm) == REG_FS || rmreg(rm) == REG_GS) {
        vlog(VM_CPUMSG, "%04X:%04X: Write to 80386 segment register");
    }
#endif
}

void _MOV_RM8_reg8(vomit_cpu_t *cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    vomit_cpu_modrm_write8(cpu, rm, *cpu->treg8[rmreg(rm)]);
}

void _MOV_reg8_RM8(vomit_cpu_t *cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    *cpu->treg8[rmreg(rm)] = vomit_cpu_modrm_read8(cpu, rm);
}

void _MOV_RM16_reg16(vomit_cpu_t *cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    vomit_cpu_modrm_write16(cpu, rm, *cpu->treg16[rmreg(rm)]);
}

void _MOV_reg16_RM16(vomit_cpu_t *cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    *cpu->treg16[rmreg(rm)] = vomit_cpu_modrm_read16(cpu, rm);
}

void _MOV_reg8_imm8(vomit_cpu_t *cpu)
{
    *cpu->treg8[cpu->opcode&7] = cpu->fetchOpcodeByte();
}

void _MOV_reg16_imm16(vomit_cpu_t *cpu)
{
    *cpu->treg16[cpu->opcode&7] = cpu->fetchOpcodeWord();
}

void _MOV_AL_moff8(vomit_cpu_t *cpu)
{
    cpu->regs.B.AL = vomit_cpu_memory_read8(cpu, *(cpu->CurrentSegment), cpu->fetchOpcodeWord());
}

void _MOV_AX_moff16(vomit_cpu_t *cpu)
{
    cpu->regs.W.AX = vomit_cpu_memory_read16(cpu, *(cpu->CurrentSegment), cpu->fetchOpcodeWord());
}

void _MOV_moff8_AL(vomit_cpu_t *cpu)
{
    vomit_cpu_memory_write8(cpu, *(cpu->CurrentSegment), cpu->fetchOpcodeWord(), cpu->regs.B.AL);
}

void _MOV_moff16_AX(vomit_cpu_t *cpu)
{
    vomit_cpu_memory_write16(cpu, *(cpu->CurrentSegment), cpu->fetchOpcodeWord(), cpu->regs.W.AX);
}

