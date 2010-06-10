// x86/math.cpp
// Math (arithmetic) instuctions

#include "vcpu.h"
#include "templates.h"

WORD cpu_add8(VCpu* cpu, BYTE dest, BYTE src)
{
    WORD result = dest + src;
    cpu->mathFlags8(result, dest, src);
    cpu->setOF(((
             ((result)^(dest)) &
             ((result)^(src))
             )>>(7))&1);
    return result;
}

DWORD cpu_add16(VCpu* cpu, WORD dest, WORD src)
{
    DWORD result = dest + src;
    cpu->mathFlags16(result, dest, src);
    cpu->setOF(((
             ((result)^(dest)) &
             ((result)^(src))
             )>>(15))&1);
    return result;
}

WORD cpu_adc8(VCpu* cpu, WORD dest, WORD src)
{
    WORD result;
    src += cpu->getCF();
    result = dest + src;

    cpu->mathFlags8(result, dest, src);
    cpu->setOF(((
             ((result)^(dest)) &
             ((result)^(src))
             )>>(7))&1);
    return result;
}

DWORD cpu_adc16(VCpu* cpu, WORD dest, WORD src)
{
    DWORD result;
    src += cpu->getCF();
    result = dest + src;

    cpu->mathFlags16(result, dest, src);
    cpu->setOF(((
             ((result)^(dest)) &
             ((result)^(src))
             )>>(15))&1);
    return result;
}

WORD cpu_sub8(VCpu* cpu, BYTE dest, BYTE src)
{
    WORD result = dest - src;
    cpu->cmpFlags8(result, dest, src);
    return result;
}

DWORD cpu_sub16(VCpu* cpu, WORD dest, WORD src)
{
    DWORD result = dest - src;
    cpu->cmpFlags16(result, dest, src);
    return result;
}

WORD cpu_sbb8(VCpu* cpu, BYTE dest, BYTE src)
{
    WORD result;
    src += cpu->getCF();
    result = dest - src;
    cpu->cmpFlags8(result, dest, src);
    return result;
}

DWORD cpu_sbb16(VCpu* cpu, WORD dest, WORD src)
{
    DWORD result;
    src += cpu->getCF();
    result = dest - src;
    cpu->cmpFlags16(result, dest, src);
    return result;
}

WORD cpu_mul8(VCpu* cpu, BYTE acc, BYTE multi)
{
    WORD result = acc * multi;
    cpu->mathFlags8(result, acc, multi);

#if VOMIT_CPU_LEVEL == 0
    // 8086 CPUs set ZF on zero result
    cpu->setZF(result == 0);
#endif

    return result;
}

DWORD cpu_mul16(VCpu* cpu, WORD acc, WORD multi)
{
    DWORD result = acc * multi;
    cpu->mathFlags16(result, acc, multi);

#if VOMIT_CPU_LEVEL == 0
    // 8086 CPUs set ZF on zero result
    cpu->setZF(result == 0);
#endif

    return result;
}

SIGNED_WORD cpu_imul8(VCpu* cpu, SIGNED_BYTE acc, SIGNED_BYTE multi)
{
    SIGNED_WORD result = acc * multi;
    cpu->mathFlags8(result, acc, multi);
    return result;
}


SIGNED_DWORD cpu_imul16(VCpu* cpu, SIGNED_WORD acc, SIGNED_WORD multi)
{
    SIGNED_DWORD result = acc * multi;
    cpu->mathFlags16(result, acc, multi);
    return result;
}

DEFAULT_RM8_reg8(cpu_add, _ADD_RM8_reg8)
DEFAULT_RM16_reg16(cpu_add, _ADD_RM16_reg16)
DEFAULT_reg8_RM8(cpu_add, _ADD_reg8_RM8)
DEFAULT_reg16_RM16(cpu_add, _ADD_reg16_RM16)
DEFAULT_RM8_imm8(cpu_add, _ADD_RM8_imm8)
DEFAULT_RM16_imm16(cpu_add, _ADD_RM16_imm16)
DEFAULT_RM16_imm8(cpu_add, _ADD_RM16_imm8)
DEFAULT_AL_imm8(cpu_add, _ADD_AL_imm8)
DEFAULT_AX_imm16(cpu_add, _ADD_AX_imm16)

DEFAULT_RM8_reg8(cpu_adc, _ADC_RM8_reg8)
DEFAULT_RM16_reg16(cpu_adc, _ADC_RM16_reg16)
DEFAULT_reg8_RM8(cpu_adc, _ADC_reg8_RM8)
DEFAULT_reg16_RM16(cpu_adc, _ADC_reg16_RM16)
DEFAULT_RM8_imm8(cpu_adc, _ADC_RM8_imm8)
DEFAULT_RM16_imm16(cpu_adc, _ADC_RM16_imm16)
DEFAULT_RM16_imm8(cpu_adc, _ADC_RM16_imm8)
DEFAULT_AL_imm8(cpu_adc, _ADC_AL_imm8)
DEFAULT_AX_imm16(cpu_adc, _ADC_AX_imm16)

DEFAULT_RM8_reg8(cpu_sub, _SUB_RM8_reg8)
DEFAULT_RM16_reg16(cpu_sub, _SUB_RM16_reg16)
DEFAULT_reg8_RM8(cpu_sub, _SUB_reg8_RM8)
DEFAULT_reg16_RM16(cpu_sub, _SUB_reg16_RM16)
DEFAULT_RM8_imm8(cpu_sub, _SUB_RM8_imm8)
DEFAULT_RM16_imm16(cpu_sub, _SUB_RM16_imm16)
DEFAULT_RM16_imm8(cpu_sub, _SUB_RM16_imm8)
DEFAULT_AL_imm8(cpu_sub, _SUB_AL_imm8)
DEFAULT_AX_imm16(cpu_sub, _SUB_AX_imm16)

DEFAULT_RM8_reg8(cpu_sbb, _SBB_RM8_reg8)
DEFAULT_RM16_reg16(cpu_sbb, _SBB_RM16_reg16)
DEFAULT_reg8_RM8(cpu_sbb, _SBB_reg8_RM8)
DEFAULT_reg16_RM16(cpu_sbb, _SBB_reg16_RM16)
DEFAULT_RM8_imm8(cpu_sbb, _SBB_RM8_imm8)
DEFAULT_RM16_imm16(cpu_sbb, _SBB_RM16_imm16)
DEFAULT_RM16_imm8(cpu_sbb, _SBB_RM16_imm8)
DEFAULT_AL_imm8(cpu_sbb, _SBB_AL_imm8)
DEFAULT_AX_imm16(cpu_sbb, _SBB_AX_imm16)

READONLY_RM8_reg8(cpu_sub, _CMP_RM8_reg8)
READONLY_RM16_reg16(cpu_sub, _CMP_RM16_reg16)
READONLY_reg8_RM8(cpu_sub, _CMP_reg8_RM8)
READONLY_reg16_RM16(cpu_sub, _CMP_reg16_RM16)
READONLY_RM8_imm8(cpu_sub, _CMP_RM8_imm8)
READONLY_RM16_imm16(cpu_sub, _CMP_RM16_imm16)
READONLY_RM16_imm8(cpu_sub, _CMP_RM16_imm8)
READONLY_AL_imm8(cpu_sub, _CMP_AL_imm8)
READONLY_AX_imm16(cpu_sub, _CMP_AX_imm16)

void _MUL_RM8(VCpu* cpu)
{
    BYTE value = cpu->readModRM8(cpu->rmbyte);
    cpu->regs.W.AX = cpu_mul8(cpu, cpu->regs.B.AL, value);

    if (cpu->regs.B.AH == 0x00) {
        cpu->setCF(0);
        cpu->setOF(0);
    } else {
        cpu->setCF(1);
        cpu->setOF(1);
    }
}

void _MUL_RM16(VCpu* cpu)
{
    WORD value = cpu->readModRM16(cpu->rmbyte);
    DWORD result = cpu_mul16(cpu, cpu->regs.W.AX, value);
    cpu->regs.W.AX = result & 0xFFFF;
    cpu->regs.W.DX = (result >> 16) & 0xFFFF;

    if (cpu->regs.W.DX == 0x0000) {
        cpu->setCF(0);
        cpu->setOF(0);
    } else {
        cpu->setCF(1);
        cpu->setOF(1);
    }
}

void _IMUL_RM8(VCpu* cpu)
{
    SIGNED_BYTE value = (SIGNED_BYTE)cpu->readModRM8(cpu->rmbyte);
    cpu->regs.W.AX = (SIGNED_WORD)cpu_imul8(cpu, cpu->regs.B.AL, value);

    if (cpu->regs.B.AH == 0x00 || cpu->regs.B.AH == 0xFF) {
        cpu->setCF(0);
        cpu->setOF(0);
    } else {
        cpu->setCF(1);
        cpu->setOF(1);
    }
}

void _IMUL_reg16_RM16_imm8(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    BYTE imm = cpu->fetchOpcodeByte();
    SIGNED_WORD value = (SIGNED_WORD)cpu->readModRM16(rm);
    SIGNED_WORD result = cpu_imul16(cpu, value, imm);

    *cpu->treg16[rmreg(rm)] = result;

    if ((result & 0xFF00) == 0x00 || (result & 0xFF00) == 0xFF) {
        cpu->setCF(0);
        cpu->setOF(0);
    } else {
        cpu->setCF(1);
        cpu->setOF(1);
    }
}

void _IMUL_RM16(VCpu* cpu)
{
    SIGNED_WORD value = cpu->readModRM16(cpu->rmbyte);
    SIGNED_DWORD result = cpu_imul16(cpu, cpu->regs.W.AX, value);
    cpu->regs.W.AX = result;
    cpu->regs.W.DX = result >> 16;

    if (cpu->regs.W.DX == 0x0000 || cpu->regs.W.DX == 0xFFFF) {
        cpu->setCF(0);
        cpu->setOF(0);
    } else {
        cpu->setCF(1);
        cpu->setOF(1);
    }
}

void _DIV_RM8(VCpu* cpu)
{
    BYTE value = cpu->readModRM8(cpu->rmbyte);
    WORD tAX = cpu->regs.W.AX;

    if (value == 0) {
        cpu->exception(0);
        return;
    }

    cpu->regs.B.AL = (byte)(tAX / value); // Quote
    cpu->regs.B.AH = (byte)(tAX % value); // Remainder
}

void _DIV_RM16(VCpu* cpu)
{
    WORD value = cpu->readModRM16(cpu->rmbyte);
    DWORD tDXAX = cpu->regs.W.AX + (cpu->regs.W.DX << 16);

    if (value == 0) {
        cpu->exception(0);
        return;
    }

    cpu->regs.W.AX = (WORD)(tDXAX / value); // Quote
    cpu->regs.W.DX = (WORD)(tDXAX % value); // Remainder
}

void _IDIV_RM8(VCpu* cpu)
{
    SIGNED_BYTE value = (SIGNED_BYTE)cpu->readModRM8(cpu->rmbyte);
    SIGNED_WORD tAX = (SIGNED_WORD)cpu->regs.W.AX;

    if (value == 0) {
        cpu->exception(0);
        return;
    }

    cpu->regs.B.AL = (SIGNED_BYTE)(tAX / value); // Quote
    cpu->regs.B.AH = (SIGNED_BYTE)(tAX % value); // Remainder
}

void _IDIV_RM16(VCpu* cpu)
{
    SIGNED_WORD value = cpu->readModRM16(cpu->rmbyte);
    SIGNED_DWORD tDXAX = (cpu->regs.W.AX + (cpu->regs.W.DX << 16));

    if (value == 0) {
        cpu->exception(0);
        return;
    }

    cpu->regs.W.AX = (SIGNED_WORD)(tDXAX / value); // Quote
    cpu->regs.W.DX = (SIGNED_WORD)(tDXAX % value); // Remainder
}
