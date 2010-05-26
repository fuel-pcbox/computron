// x86/bitwise.cpp
// Bitwise instructions

#include "vcpu.h"
#include "templates.h"

DEFAULT_RM8_reg8(cpu_and, _AND_RM8_reg8)
DEFAULT_RM16_reg16(cpu_and, _AND_RM16_reg16)
DEFAULT_reg8_RM8(cpu_and, _AND_reg8_RM8)
DEFAULT_reg16_RM16(cpu_and, _AND_reg16_RM16)
DEFAULT_RM8_imm8(cpu_and, _AND_RM8_imm8)
DEFAULT_RM16_imm16(cpu_and, _AND_RM16_imm16)
DEFAULT_RM16_imm8(cpu_and, _AND_RM16_imm8)
DEFAULT_AL_imm8(cpu_and, _AND_AL_imm8)
DEFAULT_AX_imm16(cpu_and, _AND_AX_imm16)

DEFAULT_RM8_reg8(cpu_xor, _XOR_RM8_reg8)
DEFAULT_RM16_reg16(cpu_xor, _XOR_RM16_reg16)
DEFAULT_reg8_RM8(cpu_xor, _XOR_reg8_RM8)
DEFAULT_reg16_RM16(cpu_xor, _XOR_reg16_RM16)
DEFAULT_RM8_imm8(cpu_xor, _XOR_RM8_imm8)
DEFAULT_RM16_imm16(cpu_xor, _XOR_RM16_imm16)
DEFAULT_RM16_imm8(cpu_xor, _XOR_RM16_imm8)
DEFAULT_AL_imm8(cpu_xor, _XOR_AL_imm8)
DEFAULT_AX_imm16(cpu_xor, _XOR_AX_imm16)

DEFAULT_RM8_reg8(cpu_or, _OR_RM8_reg8)
DEFAULT_RM16_reg16(cpu_or, _OR_RM16_reg16)
DEFAULT_reg8_RM8(cpu_or, _OR_reg8_RM8)
DEFAULT_reg16_RM16(cpu_or, _OR_reg16_RM16)
DEFAULT_RM8_imm8(cpu_or, _OR_RM8_imm8)
DEFAULT_RM16_imm16(cpu_or, _OR_RM16_imm16)
DEFAULT_RM16_imm8(cpu_or, _OR_RM16_imm8)
DEFAULT_AL_imm8(cpu_or, _OR_AL_imm8)
DEFAULT_AX_imm16(cpu_or, _OR_AX_imm16)

READONLY_RM8_reg8(cpu_and, _TEST_RM8_reg8)
READONLY_RM16_reg16(cpu_and, _TEST_RM16_reg16)
READONLY_reg8_RM8(cpu_and, _TEST_reg8_RM8)
READONLY_reg16_RM16(cpu_and, _TEST_reg16_RM16)
READONLY_RM8_imm8(cpu_and, _TEST_RM8_imm8)
READONLY_RM16_imm16(cpu_and, _TEST_RM16_imm16)
READONLY_RM16_imm8(cpu_and, _TEST_RM16_imm8)
READONLY_AL_imm8(cpu_and, _TEST_AL_imm8)
READONLY_AX_imm16(cpu_and, _TEST_AX_imm16)

void _CBW(VCpu* cpu)
{
    cpu->regs.W.AX = vomit_signExtend(cpu->regs.B.AL);
}

void _CWD(VCpu* cpu)
{
    if (cpu->regs.B.AH & 0x80)
        cpu->regs.W.DX = 0xFFFF;
    else
        cpu->regs.W.DX = 0x0000;
}

void _SALC(VCpu* cpu)
{
    cpu->regs.B.AL = cpu->getCF() ? 0xFF : 0x00;
}

BYTE cpu_or8(VCpu* cpu, BYTE dest, BYTE src)
{
    BYTE result = dest | src;
    cpu->updateFlags8(result);
    cpu->setOF(0);
    cpu->setCF(0);
    return result;
}

WORD cpu_or16(VCpu* cpu, WORD dest, WORD src)
{
    WORD result = dest | src;
    cpu->updateFlags16(result);
    cpu->setOF(0);
    cpu->setCF(0);
    return result;
}

BYTE cpu_xor8(VCpu* cpu, BYTE dest, BYTE src)
{
    BYTE result = dest ^ src;
    cpu->updateFlags8(result);
    cpu->setOF(0);
    cpu->setCF(0);
    return result;
}

WORD cpu_xor16(VCpu* cpu, WORD dest, WORD src)
{
    WORD result = dest ^ src;
    cpu->updateFlags16(result);
    cpu->setOF(0);
    cpu->setCF(0);
    return result;
}

BYTE cpu_and8(VCpu* cpu, BYTE dest, BYTE src)
{
    BYTE result = dest & src;
    cpu->updateFlags8(result);
    cpu->setOF(0);
    cpu->setCF(0);
    return result;
}

WORD cpu_and16(VCpu* cpu, WORD dest, WORD src)
{
    WORD result = dest & src;
    cpu->updateFlags16(result);
    cpu->setOF(0);
    cpu->setCF(0);
    return result;
}

DWORD cpu_shl(VCpu* cpu, WORD data, BYTE steps, BYTE bits)
{
    DWORD result = (DWORD)data;

    steps &= 0x1F;

    if (bits == 8) {
        for (BYTE i = 0; i < steps; ++i) {
            cpu->setCF((result >> 7) & 1);
            result <<= 1;
        }
    } else {
        for (BYTE i = 0; i < steps; ++i) {
            cpu->setCF((result >> 15) & 1);
            result <<= 1;
        }
    }

    if (steps == 1)
        cpu->setOF((data >> (bits - 1)) ^ cpu->getCF());

    cpu->updateFlags(result, bits);
    return result;
}

DWORD cpu_shr(VCpu* cpu, WORD data, BYTE steps, BYTE bits)
{
    DWORD result = (DWORD)data;

    steps &= 0x1F;

    if (bits == 8) {
        for (BYTE i = 0; i < steps; ++i) {
            cpu->setCF(result & 1);
            result >>= 1;
        }
    } else {
        for (BYTE i = 0; i < steps; ++i) {
            cpu->setCF(result & 1);
            result >>= 1;
        }
    }

    if (steps == 1)
        cpu->setOF((data >> (bits - 1)) & 1);

    cpu->updateFlags(result, bits);
    return result;
}

DWORD cpu_sar(VCpu* cpu, WORD data, BYTE steps, BYTE bits)
{
    DWORD result = (DWORD)data;
    WORD n;

    steps &= 0x1F;

    if (bits == 8) {
        for (BYTE i = 0; i < steps; ++i) {
            n = result;
            result = (result>>1) | (n&0x80);
            cpu->setCF(n & 1);
        }
    } else {
        for (BYTE i = 0; i < steps; ++i) {
            n = result;
            result = (result>>1) | (n&0x8000);
            cpu->setCF(n & 1);
        }
    }

    if (steps == 1)
        cpu->setOF(0);

    cpu->updateFlags(result, bits);
    return result;
}

DWORD cpu_rol(VCpu* cpu, WORD data, BYTE steps, BYTE bits)
{
    DWORD result = (DWORD)data;

    steps &= 0x1F;

    if (bits == 8) {
        for (BYTE i = 0; i < steps; ++i) {
            cpu->setCF((result>>7) & 1);
            result = (result<<1) | cpu->getCF();
        }
    } else {
        for (BYTE i = 0; i < steps; ++i) {
            cpu->setCF((result>>15) & 1);
            result = (result<<1) | cpu->getCF();
        }
    }

    if (steps == 1)
        cpu->setOF(((result >> (bits - 1)) & 1) ^ cpu->getCF());

    return result;
}

DWORD cpu_ror(VCpu* cpu, WORD data, BYTE steps, BYTE bits)
{
    DWORD result = (DWORD)data;

    steps &= 0x1F;

    if (bits == 8) {
        for (BYTE i = 0; i < steps; ++i) {
            cpu->setCF(result & 1);
            result = (result>>1) | (cpu->getCF()<<7);
        }
    } else {
        for (BYTE i = 0; i < steps; ++i) {
            cpu->setCF(result & 1);
            result = (result>>1) | (cpu->getCF()<<15);
        }
    }

    if (steps == 1)
        cpu->setOF((result >> (bits - 1)) ^ ((result >> (bits - 2) & 1)));

    return result;
}

DWORD cpu_rcl(VCpu* cpu, WORD data, BYTE steps, BYTE bits)
{
    DWORD result = (DWORD)data;
    WORD n;

    steps &= 0x1F;

    if (bits == 8) {
        for (BYTE i = 0; i < steps; ++i) {
            n = result;
            result = ((result<<1) & 0xFF) | cpu->getCF();
            cpu->setCF((n>>7) & 1);
        }
    } else {
        for (BYTE i = 0; i < steps; ++i) {
            n = result;
            result = ((result<<1) & 0xFFFF) | cpu->getCF();
            cpu->setCF((n>>15) & 1);
        }
    }

    if (steps == 1)
        cpu->setOF((result >> (bits - 1)) ^ cpu->getCF());

    return result;
}

DWORD cpu_rcr(VCpu* cpu, WORD data, BYTE steps, BYTE bits)
{
    DWORD result = (DWORD)data;
    WORD n;

    steps &= 0x1F;

    if (bits == 8) {
        for (BYTE i = 0; i < steps; ++i) {
            n = result;
            result = (result>>1) | (cpu->getCF()<<7);
            cpu->setCF(n & 1);
        }
    } else {
        for (BYTE i = 0; i < steps; ++i) {
            n = result;
            result = (result>>1) | (cpu->getCF()<<15);
            cpu->setCF(n & 1);
        }
    }

    if (steps == 1)
        cpu->setOF((result >> (bits - 1)) ^ ((result >> (bits - 2) & 1)));

    return result;
}

void _NOT_RM8(VCpu* cpu)
{
    BYTE value = cpu->readModRM8(cpu->rmbyte);
    cpu->updateModRM8(~value);
}

void _NOT_RM16(VCpu* cpu)
{
    WORD value = cpu->readModRM16(cpu->rmbyte);
    cpu->updateModRM16(~value);
}

void _NEG_RM8(VCpu* cpu)
{
    BYTE value = cpu->readModRM8(cpu->rmbyte);
    BYTE old = value;
    value = -value;
    cpu->updateModRM8(value);
    cpu->setCF(old != 0);
    cpu->updateFlags8(value);
    cpu->setOF(((
        ((0)^(old)) &
        ((0)^(value))
        )>>(7))&1);
    vomit_cpu_setAF(cpu, value, 0, old);
}

void _NEG_RM16(VCpu* cpu)
{
    WORD value = cpu->readModRM16(cpu->rmbyte);
    WORD old = value;
    value = -value;
    cpu->updateModRM16(value);
    cpu->setCF(old != 0);
    cpu->updateFlags16(value);
    cpu->setOF(((
        ((0)^(old)) &
        ((0)^(value))
        )>>(15))&1);
    vomit_cpu_setAF(cpu, value, 0, old);
}
