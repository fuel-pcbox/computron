/*
 * Copyright (C) 2003-2011 Andreas Kling <kling@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ANDREAS KLING ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANDREAS KLING OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "vcpu.h"
#include "templates.h"

DEFAULT_RM8_reg8(cpu_and, _AND_RM8_reg8)
DEFAULT_RM16_reg16(cpu_and, _AND_RM16_reg16)
DEFAULT_RM32_reg32(cpu_and, _AND_RM32_reg32)
DEFAULT_reg8_RM8(cpu_and, _AND_reg8_RM8)
DEFAULT_reg16_RM16(cpu_and, _AND_reg16_RM16)
DEFAULT_reg32_RM32(cpu_and, _AND_reg32_RM32)
DEFAULT_RM8_imm8(cpu_and, _AND_RM8_imm8)
DEFAULT_RM16_imm16(cpu_and, _AND_RM16_imm16)
DEFAULT_RM32_imm32(cpu_and, _AND_RM32_imm32)
DEFAULT_RM32_imm8(cpu_and, _AND_RM32_imm8)
DEFAULT_RM16_imm8(cpu_and, _AND_RM16_imm8)
DEFAULT_AL_imm8(cpu_and, _AND_AL_imm8)
DEFAULT_AX_imm16(cpu_and, _AND_AX_imm16)
DEFAULT_EAX_imm32(cpu_and, _AND_EAX_imm32)

DEFAULT_RM8_reg8(cpu_xor, _XOR_RM8_reg8)
DEFAULT_RM16_reg16(cpu_xor, _XOR_RM16_reg16)
DEFAULT_RM32_reg32(cpu_xor, _XOR_RM32_reg32)
DEFAULT_reg8_RM8(cpu_xor, _XOR_reg8_RM8)
DEFAULT_reg16_RM16(cpu_xor, _XOR_reg16_RM16)
DEFAULT_reg32_RM32(cpu_xor, _XOR_reg32_RM32)
DEFAULT_RM8_imm8(cpu_xor, _XOR_RM8_imm8)
DEFAULT_RM16_imm16(cpu_xor, _XOR_RM16_imm16)
DEFAULT_RM32_imm32(cpu_xor, _XOR_RM32_imm32)
DEFAULT_RM16_imm8(cpu_xor, _XOR_RM16_imm8)
DEFAULT_RM32_imm8(cpu_xor, _XOR_RM32_imm8)
DEFAULT_AL_imm8(cpu_xor, _XOR_AL_imm8)
DEFAULT_AX_imm16(cpu_xor, _XOR_AX_imm16)
DEFAULT_EAX_imm32(cpu_xor, _XOR_EAX_imm32)

DEFAULT_RM8_reg8(cpu_or, _OR_RM8_reg8)
DEFAULT_RM16_reg16(cpu_or, _OR_RM16_reg16)
DEFAULT_RM32_reg32(cpu_or, _OR_RM32_reg32)
DEFAULT_reg8_RM8(cpu_or, _OR_reg8_RM8)
DEFAULT_reg16_RM16(cpu_or, _OR_reg16_RM16)
DEFAULT_reg32_RM32(cpu_or, _OR_reg32_RM32)
DEFAULT_RM8_imm8(cpu_or, _OR_RM8_imm8)
DEFAULT_RM16_imm16(cpu_or, _OR_RM16_imm16)
DEFAULT_RM32_imm32(cpu_or, _OR_RM32_imm32)
DEFAULT_RM16_imm8(cpu_or, _OR_RM16_imm8)
DEFAULT_RM32_imm8(cpu_or, _OR_RM32_imm8)
DEFAULT_AL_imm8(cpu_or, _OR_AL_imm8)
DEFAULT_AX_imm16(cpu_or, _OR_AX_imm16)
DEFAULT_EAX_imm32(cpu_or, _OR_EAX_imm32)

READONLY_RM8_reg8(cpu_and, _TEST_RM8_reg8)
READONLY_RM16_reg16(cpu_and, _TEST_RM16_reg16)
READONLY_RM32_reg32(cpu_and, _TEST_RM32_reg32)
READONLY_RM8_imm8(cpu_and, _TEST_RM8_imm8)
READONLY_RM16_imm16(cpu_and, _TEST_RM16_imm16)
READONLY_RM32_imm32(cpu_and, _TEST_RM32_imm32)
READONLY_AL_imm8(cpu_and, _TEST_AL_imm8)
READONLY_AX_imm16(cpu_and, _TEST_AX_imm16)
READONLY_EAX_imm32(cpu_and, _TEST_EAX_imm32)

void VCpu::_CBW()
{
    if (getAL() & 0x80)
        setAH(0xFF);
    else
        setAH(0x00);
}

void VCpu::_CWD()
{
    if (getAX() & 0x8000)
        setDX(0xFFFF);
    else
        setDX(0x0000);
}

void VCpu::_CWDE()
{
    if (getAX() & 0x8000)
        regs.W.__EAX_high_word = 0xFFFF;
    else
        regs.W.__EAX_high_word = 0x0000;
}

void VCpu::_CDQ()
{
    if (getEAX() & 0x80000000)
        setEDX(0xFFFFFFFF);
    else
        setEDX(0x00000000);
}

void VCpu::_SALC()
{
    setAL(getCF() ? 0xFF : 0);
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

DWORD cpu_or32(VCpu* cpu, DWORD dest, DWORD src)
{
    DWORD result = dest | src;
    cpu->updateFlags32(result);
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

DWORD cpu_xor32(VCpu* cpu, DWORD dest, DWORD src)
{
    DWORD result = dest ^ src;
    cpu->updateFlags32(result);
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

DWORD cpu_and32(VCpu* cpu, DWORD dest, DWORD src)
{
    DWORD result = dest & src;
    cpu->updateFlags32(result);
    cpu->setOF(0);
    cpu->setCF(0);
    return result;
}

DWORD cpu_sar(VCpu* cpu, WORD data, BYTE steps, BYTE bits)
{
    DWORD result = data;
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
    DWORD result = data;

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
    DWORD result = data;
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

void VCpu::_NOT_RM8()
{
    BYTE value = readModRM8(rmbyte);
    updateModRM8(~value);
}

void VCpu::_NOT_RM16()
{
    WORD value = readModRM16(rmbyte);
    updateModRM16(~value);
}

void VCpu::_NEG_RM8()
{
    BYTE value = readModRM8(rmbyte);
    BYTE old = value;
    value = -value;
    updateModRM8(value);
    setCF(old != 0);
    updateFlags8(value);
    setOF(((
        ((0)^(old)) &
        ((0)^(value))
        )>>(7))&1);
    adjustFlag32(value, 0, old);
}

void VCpu::_NEG_RM16()
{
    WORD value = readModRM16(rmbyte);
    WORD old = value;
    value = -value;
    updateModRM16(value);
    setCF(old != 0);
    updateFlags16(value);
    setOF(((
        ((0)^(old)) &
        ((0)^(value))
        )>>(15))&1);
    adjustFlag32(value, 0, old);
}
