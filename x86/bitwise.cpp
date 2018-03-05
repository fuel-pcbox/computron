/*
 * Copyright (C) 2003-2018 Andreas Kling <awesomekling@gmail.com>
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

#include "CPU.h"
#include "templates.h"

DEFAULT_RM8_reg8(doAnd, _AND_RM8_reg8)
DEFAULT_RM16_reg16(doAnd, _AND_RM16_reg16)
DEFAULT_RM32_reg32(doAnd, _AND_RM32_reg32)
DEFAULT_reg8_RM8(doAnd, _AND_reg8_RM8)
DEFAULT_reg16_RM16(doAnd, _AND_reg16_RM16)
DEFAULT_reg32_RM32(doAnd, _AND_reg32_RM32)
DEFAULT_RM8_imm8(doAnd, _AND_RM8_imm8)
DEFAULT_RM16_imm16(doAnd, _AND_RM16_imm16)
DEFAULT_RM32_imm32(doAnd, _AND_RM32_imm32)
DEFAULT_RM32_imm8(doAnd, _AND_RM32_imm8)
DEFAULT_RM16_imm8(doAnd, _AND_RM16_imm8)
DEFAULT_AL_imm8(doAnd, _AND_AL_imm8)
DEFAULT_AX_imm16(doAnd, _AND_AX_imm16)
DEFAULT_EAX_imm32(doAnd, _AND_EAX_imm32)

DEFAULT_RM8_reg8(doXor, _XOR_RM8_reg8)
DEFAULT_RM16_reg16(doXor, _XOR_RM16_reg16)
DEFAULT_RM32_reg32(doXor, _XOR_RM32_reg32)
DEFAULT_reg8_RM8(doXor, _XOR_reg8_RM8)
DEFAULT_reg16_RM16(doXor, _XOR_reg16_RM16)
DEFAULT_reg32_RM32(doXor, _XOR_reg32_RM32)
DEFAULT_RM8_imm8(doXor, _XOR_RM8_imm8)
DEFAULT_RM16_imm16(doXor, _XOR_RM16_imm16)
DEFAULT_RM32_imm32(doXor, _XOR_RM32_imm32)
DEFAULT_RM16_imm8(doXor, _XOR_RM16_imm8)
DEFAULT_RM32_imm8(doXor, _XOR_RM32_imm8)
DEFAULT_AL_imm8(doXor, _XOR_AL_imm8)
DEFAULT_AX_imm16(doXor, _XOR_AX_imm16)
DEFAULT_EAX_imm32(doXor, _XOR_EAX_imm32)

DEFAULT_RM8_reg8(doOr, _OR_RM8_reg8)
DEFAULT_RM16_reg16(doOr, _OR_RM16_reg16)
DEFAULT_RM32_reg32(doOr, _OR_RM32_reg32)
DEFAULT_reg8_RM8(doOr, _OR_reg8_RM8)
DEFAULT_reg16_RM16(doOr, _OR_reg16_RM16)
DEFAULT_reg32_RM32(doOr, _OR_reg32_RM32)
DEFAULT_RM8_imm8(doOr, _OR_RM8_imm8)
DEFAULT_RM16_imm16(doOr, _OR_RM16_imm16)
DEFAULT_RM32_imm32(doOr, _OR_RM32_imm32)
DEFAULT_RM16_imm8(doOr, _OR_RM16_imm8)
DEFAULT_RM32_imm8(doOr, _OR_RM32_imm8)
DEFAULT_AL_imm8(doOr, _OR_AL_imm8)
DEFAULT_AX_imm16(doOr, _OR_AX_imm16)
DEFAULT_EAX_imm32(doOr, _OR_EAX_imm32)

READONLY_RM8_reg8(doAnd, _TEST_RM8_reg8)
READONLY_RM16_reg16(doAnd, _TEST_RM16_reg16)
READONLY_RM32_reg32(doAnd, _TEST_RM32_reg32)
READONLY_RM8_imm8(doAnd, _TEST_RM8_imm8)
READONLY_RM16_imm16(doAnd, _TEST_RM16_imm16)
READONLY_RM32_imm32(doAnd, _TEST_RM32_imm32)
READONLY_AL_imm8(doAnd, _TEST_AL_imm8)
READONLY_AX_imm16(doAnd, _TEST_AX_imm16)
READONLY_EAX_imm32(doAnd, _TEST_EAX_imm32)

void CPU::_CBW(Instruction&)
{
    if (getAL() & 0x80)
        setAH(0xFF);
    else
        setAH(0x00);
}

void CPU::_CWD(Instruction&)
{
    if (getAX() & 0x8000)
        setDX(0xFFFF);
    else
        setDX(0x0000);
}

void CPU::_CWDE(Instruction&)
{
    if (getAX() & 0x8000)
        regs.W.__EAX_high_word = 0xFFFF;
    else
        regs.W.__EAX_high_word = 0x0000;
}

void CPU::_CDQ(Instruction&)
{
    if (getEAX() & 0x80000000)
        setEDX(0xFFFFFFFF);
    else
        setEDX(0x00000000);
}

void CPU::_SALC(Instruction&)
{
    setAL(getCF() ? 0xFF : 0);
}

// FIXME: Move this method into CPU.
template<typename T>
inline void updateCpuFlags(CPU& cpu, T result)
{
    if (BitSizeOfType<T>::bits == 8)
        cpu.updateFlags8(result);
    else if (BitSizeOfType<T>::bits == 16)
        cpu.updateFlags16(result);
    else if (BitSizeOfType<T>::bits == 32)
        cpu.updateFlags32(result);
}

template <typename T>
T CPU::doOr(T dest, T src)
{
    T result = dest | src;
    updateCpuFlags(*this, result);
    setOF(0);
    setCF(0);
    return result;
}

template<typename T>
T CPU::doXor(T dest, T src)
{
    T result = dest ^ src;
    updateCpuFlags(*this, result);
    setOF(0);
    setCF(0);
    return result;
}

template<typename T>
T CPU::doAnd(T dest, T src)
{
    T result = dest & src;
    updateCpuFlags(*this, result);
    setOF(0);
    setCF(0);
    return result;
}

inline DWORD allOnes(unsigned bits)
{
    if (bits == 8)
        return 0xFF;
    if (bits == 16)
        return 0xFFFF;
    ASSERT(bits == 32);
    return 0xFFFFFFFF;
}

DWORD cpu_sar(CPU& cpu, DWORD data, BYTE steps, BYTE bits)
{
    DWORD result = data;
    DWORD n;
    DWORD mask = 1 << (bits - 1);

    steps &= 0x1F;
    if (!steps)
        return data;

    for (BYTE i = 0; i < steps; ++i) {
        n = result;
        result = (result >> 1) | (n & mask);
        cpu.setCF(n & 1);
    }

    if (steps == 1)
        cpu.setOF(0);
    cpu.updateFlags(result, bits);
    return result;
}

DWORD cpu_rcl(CPU& cpu, DWORD data, BYTE steps, BYTE bits)
{
    DWORD result = data;
    DWORD n;
    DWORD mask = allOnes(bits);

    steps &= 0x1F;
    if (!steps)
        return data;

    for (BYTE i = 0; i < steps; ++i) {
        n = result;
        result = ((result<<1) & mask) | cpu.getCF();
        cpu.setCF((n>>(bits-1)) & 1);
    }

    if (steps == 1)
        cpu.setOF((result >> (bits - 1)) ^ cpu.getCF());

    return result;
}

DWORD cpu_rcr(CPU& cpu, DWORD data, BYTE steps, BYTE bits)
{
    DWORD result = data;
    DWORD n;

    steps &= 0x1F;
    if (!steps)
        return data;

    for (BYTE i = 0; i < steps; ++i) {
        n = result;
        result = (result>>1) | (cpu.getCF()<<(bits-1));
        cpu.setCF(n & 1);
    }

    if (steps == 1)
        cpu.setOF((result >> (bits - 1)) ^ ((result >> (bits - 2) & 1)));

    return result;
}

void CPU::_NOT_RM8(Instruction& insn)
{
    insn.modrm().write8(~insn.modrm().read8());
}

void CPU::_NOT_RM16(Instruction& insn)
{
    insn.modrm().write16(~insn.modrm().read16());
}

void CPU::_NOT_RM32(Instruction& insn)
{
    insn.modrm().write32(~insn.modrm().read32());
}

DEFAULT_RM16_imm8(doBt, _BT_RM16_imm8)
DEFAULT_RM32_imm8(doBt, _BT_RM32_imm8)
DEFAULT_RM16_reg16(doBt, _BT_RM16_reg16)
DEFAULT_RM32_reg32(doBt, _BT_RM32_reg32)
DEFAULT_RM16_imm8(doBtr, _BTR_RM16_imm8)
DEFAULT_RM32_imm8(doBtr, _BTR_RM32_imm8)
DEFAULT_RM16_reg16(doBtr, _BTR_RM16_reg16)
DEFAULT_RM32_reg32(doBtr, _BTR_RM32_reg32)
DEFAULT_RM16_imm8(doBtc, _BTC_RM16_imm8)
DEFAULT_RM32_imm8(doBtc, _BTC_RM32_imm8)
DEFAULT_RM16_reg16(doBtc, _BTC_RM16_reg16)
DEFAULT_RM32_reg32(doBtc, _BTC_RM32_reg32)
DEFAULT_RM16_imm8(doBts, _BTS_RM16_imm8)
DEFAULT_RM32_imm8(doBts, _BTS_RM32_imm8)
DEFAULT_RM16_reg16(doBts, _BTS_RM16_reg16)
DEFAULT_RM32_reg32(doBts, _BTS_RM32_reg32)

template<typename T, typename U>
T CPU::doBt(T src, U bitIndex)
{
    T bitMask = 1 << bitIndex;
    setCF((src & bitMask) != 0);
    return src;
}

template<typename T, typename U>
T CPU::doBtr(T dest, U bitIndex)
{
    T bitMask = 1 << bitIndex;
    T result = dest & ~bitMask;
    setCF((dest & bitMask) != 0);
    return result;
}

template<typename T, typename U>
T CPU::doBts(T dest, U bitIndex)
{
    T bitMask = 1 << bitIndex;
    T result = dest | bitMask;
    setCF((dest & bitMask) != 0);
    return result;
}

template<typename T, typename U>
T CPU::doBtc(T dest, U bitIndex)
{
    T bitMask = 1 << bitIndex;
    T result;
    if (dest & bitMask)
        result = dest & ~bitMask;
    else
        result = dest | bitMask;
    setCF((dest & bitMask) != 0);
    return result;
}

template<typename T>
T CPU::doBSF(T src)
{
    ASSERT(src != 0);
    setZF(0);
    for (int i = 0; i < BitSizeOfType<T>::bits; ++i) {
        T mask = 1 << i;
        if (src & mask)
            return i;
    }
    ASSERT_NOT_REACHED();
    return 0;
}

template<typename T>
T CPU::doBSR(T src)
{
    ASSERT(src != 0);
    setZF(0);
    for (int i = BitSizeOfType<T>::bits - 1; i >= 0; --i) {
        T mask = 1 << i;
        if (src & mask)
            return i;
    }
    ASSERT_NOT_REACHED();
    return 0;
}

void CPU::_BSF_reg16_RM16(Instruction& insn)
{
    auto value = insn.modrm().read16();
    if (!value) {
        setZF(1);
        return;
    }
    insn.reg16() = doBSF(value);
}

void CPU::_BSF_reg32_RM32(Instruction& insn)
{
    auto value = insn.modrm().read32();
    if (!value) {
        setZF(1);
        return;
    }
    insn.reg32() = doBSF(value);
}

void CPU::_BSR_reg16_RM16(Instruction& insn)
{
    auto value = insn.modrm().read16();
    if (!value) {
        setZF(1);
        return;
    }
    insn.reg16() = doBSR(value);
}

void CPU::_BSR_reg32_RM32(Instruction& insn)
{
    auto value = insn.modrm().read32();
    if (!value) {
        setZF(1);
        return;
    }
    insn.reg32() = doBSR(value);
}

void CPU::_SHLD_RM16_reg16_imm8(Instruction& insn)
{
    DWORD value = makeDWORD(insn.modrm().read16(), insn.reg16());
    insn.modrm().write16(leftShift(value, insn.imm8()) >> 16);
}

void CPU::_SHLD_RM32_reg32_imm8(Instruction& insn)
{
    QWORD value = makeQWORD(insn.modrm().read32(), insn.reg32());
    insn.modrm().write32(leftShift(value, insn.imm8()) >> 32);
}

void CPU::_SHLD_RM16_reg16_CL(Instruction& insn)
{
    DWORD value = makeDWORD(insn.modrm().read16(), insn.reg16());
    insn.modrm().write16(leftShift(value, getCL()) >> 16);
}

void CPU::_SHLD_RM32_reg32_CL(Instruction& insn)
{
    QWORD value = makeQWORD(insn.modrm().read32(), insn.reg32());
    insn.modrm().write32(leftShift(value, getCL()) >> 32);
}

void CPU::_SHRD_RM16_reg16_imm8(Instruction& insn)
{
    DWORD value = makeDWORD(insn.reg16(), insn.modrm().read16());
    insn.modrm().write16(rightShift(value, insn.imm8()));
}

void CPU::_SHRD_RM32_reg32_imm8(Instruction& insn)
{
    QWORD value = makeQWORD(insn.reg32(), insn.modrm().read32());
    insn.modrm().write32(rightShift(value, insn.imm8()));
}

void CPU::_SHRD_RM16_reg16_CL(Instruction& insn)
{
    DWORD value = makeDWORD(insn.reg16(), insn.modrm().read16());
    insn.modrm().write16(rightShift(value, getCL()));
}

void CPU::_SHRD_RM32_reg32_CL(Instruction& insn)
{
    QWORD value = makeQWORD(insn.reg32(), insn.modrm().read32());
    insn.modrm().write32(rightShift(value, getCL()));
}
