/* 8086/mov.cpp
 * MOVe instructions
 *
 */

#include "vomit.h"
#include "debug.h"

void _MOV_RM8_imm8(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    (void) vomit_cpu_modrm_resolve8(cpu, rm);
    vomit_cpu_modrm_update8(cpu, cpu->fetchOpcodeByte());
}

void _MOV_RM16_imm16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    (void) vomit_cpu_modrm_resolve16(cpu, rm);
    vomit_cpu_modrm_update16(cpu, cpu->fetchOpcodeWord());
}

void _MOV_RM16_seg(VCpu* cpu)
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

void _MOV_seg_RM16(VCpu* cpu)
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

void _MOV_RM8_reg8(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    vomit_cpu_modrm_write8(cpu, rm, *cpu->treg8[rmreg(rm)]);
}

void _MOV_reg8_RM8(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    *cpu->treg8[rmreg(rm)] = vomit_cpu_modrm_read8(cpu, rm);
}

void _MOV_RM16_reg16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    vomit_cpu_modrm_write16(cpu, rm, *cpu->treg16[rmreg(rm)]);
}

void _MOV_reg16_RM16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    *cpu->treg16[rmreg(rm)] = vomit_cpu_modrm_read16(cpu, rm);
}

void _MOV_AL_imm8(VCpu* cpu)
{
    cpu->regs.B.AL = cpu->fetchOpcodeByte();
}

void _MOV_BL_imm8(VCpu* cpu)
{
    cpu->regs.B.BL = cpu->fetchOpcodeByte();
}

void _MOV_CL_imm8(VCpu* cpu)
{
    cpu->regs.B.CL = cpu->fetchOpcodeByte();
}

void _MOV_DL_imm8(VCpu* cpu)
{
    cpu->regs.B.DL = cpu->fetchOpcodeByte();
}

void _MOV_AH_imm8(VCpu* cpu)
{
    cpu->regs.B.AH = cpu->fetchOpcodeByte();
}

void _MOV_BH_imm8(VCpu* cpu)
{
    cpu->regs.B.BH = cpu->fetchOpcodeByte();
}

void _MOV_CH_imm8(VCpu* cpu)
{
    cpu->regs.B.CH = cpu->fetchOpcodeByte();
}

void _MOV_DH_imm8(VCpu* cpu)
{
    cpu->regs.B.DH = cpu->fetchOpcodeByte();
}

void _MOV_AX_imm16(VCpu* cpu)
{
    cpu->regs.W.AX = cpu->fetchOpcodeWord();
}

void _MOV_BX_imm16(VCpu* cpu)
{
    cpu->regs.W.BX = cpu->fetchOpcodeWord();
}

void _MOV_CX_imm16(VCpu* cpu)
{
    cpu->regs.W.CX = cpu->fetchOpcodeWord();
}

void _MOV_DX_imm16(VCpu* cpu)
{
    cpu->regs.W.DX = cpu->fetchOpcodeWord();
}

void _MOV_BP_imm16(VCpu* cpu)
{
    cpu->regs.W.BP = cpu->fetchOpcodeWord();
}

void _MOV_SP_imm16(VCpu* cpu)
{
    cpu->regs.W.SP = cpu->fetchOpcodeWord();
}

void _MOV_SI_imm16(VCpu* cpu)
{
    cpu->regs.W.SI = cpu->fetchOpcodeWord();
}

void _MOV_DI_imm16(VCpu* cpu)
{
    cpu->regs.W.DI = cpu->fetchOpcodeWord();
}

void _MOV_AL_moff8(VCpu* cpu)
{
    cpu->regs.B.AL = vomit_cpu_memory_read8(cpu, *(cpu->CurrentSegment), cpu->fetchOpcodeWord());
}

void _MOV_AX_moff16(VCpu* cpu)
{
    cpu->regs.W.AX = vomit_cpu_memory_read16(cpu, *(cpu->CurrentSegment), cpu->fetchOpcodeWord());
}

void _MOV_moff8_AL(VCpu* cpu)
{
    vomit_cpu_memory_write8(cpu, *(cpu->CurrentSegment), cpu->fetchOpcodeWord(), cpu->regs.B.AL);
}

void _MOV_moff16_AX(VCpu* cpu)
{
    vomit_cpu_memory_write16(cpu, *(cpu->CurrentSegment), cpu->fetchOpcodeWord(), cpu->regs.W.AX);
}

