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

DEFAULT_RM8_imm8(doROL, _ROL_RM8_imm8)
DEFAULT_RM16_imm8(doROL, _ROL_RM16_imm8)
DEFAULT_RM32_imm8(doROL, _ROL_RM32_imm8)
DEFAULT_RM8_1(doROL, _ROL_RM8_1)
DEFAULT_RM16_1(doROL, _ROL_RM16_1)
DEFAULT_RM32_1(doROL, _ROL_RM32_1)
DEFAULT_RM8_CL(doROL, _ROL_RM8_CL)
DEFAULT_RM16_CL(doROL, _ROL_RM16_CL)
DEFAULT_RM32_CL(doROL, _ROL_RM32_CL)

DEFAULT_RM8_imm8(doROR, _ROR_RM8_imm8)
DEFAULT_RM16_imm8(doROR, _ROR_RM16_imm8)
DEFAULT_RM32_imm8(doROR, _ROR_RM32_imm8)
DEFAULT_RM8_1(doROR, _ROR_RM8_1)
DEFAULT_RM16_1(doROR, _ROR_RM16_1)
DEFAULT_RM32_1(doROR, _ROR_RM32_1)
DEFAULT_RM8_CL(doROR, _ROR_RM8_CL)
DEFAULT_RM16_CL(doROR, _ROR_RM16_CL)
DEFAULT_RM32_CL(doROR, _ROR_RM32_CL)

DEFAULT_RM8_imm8(doSHL, _SHL_RM8_imm8)
DEFAULT_RM16_imm8(doSHL, _SHL_RM16_imm8)
DEFAULT_RM32_imm8(doSHL, _SHL_RM32_imm8)
DEFAULT_RM8_1(doSHL, _SHL_RM8_1)
DEFAULT_RM16_1(doSHL, _SHL_RM16_1)
DEFAULT_RM32_1(doSHL, _SHL_RM32_1)
DEFAULT_RM8_CL(doSHL, _SHL_RM8_CL)
DEFAULT_RM16_CL(doSHL, _SHL_RM16_CL)
DEFAULT_RM32_CL(doSHL, _SHL_RM32_CL)

DEFAULT_RM8_imm8(doSHR, _SHR_RM8_imm8)
DEFAULT_RM16_imm8(doSHR, _SHR_RM16_imm8)
DEFAULT_RM32_imm8(doSHR, _SHR_RM32_imm8)
DEFAULT_RM8_1(doSHR, _SHR_RM8_1)
DEFAULT_RM16_1(doSHR, _SHR_RM16_1)
DEFAULT_RM32_1(doSHR, _SHR_RM32_1)
DEFAULT_RM8_CL(doSHR, _SHR_RM8_CL)
DEFAULT_RM16_CL(doSHR, _SHR_RM16_CL)
DEFAULT_RM32_CL(doSHR, _SHR_RM32_CL)

DEFAULT_RM8_imm8(doSAR, _SAR_RM8_imm8)
DEFAULT_RM16_imm8(doSAR, _SAR_RM16_imm8)
DEFAULT_RM32_imm8(doSAR, _SAR_RM32_imm8)
DEFAULT_RM8_1(doSAR, _SAR_RM8_1)
DEFAULT_RM16_1(doSAR, _SAR_RM16_1)
DEFAULT_RM32_1(doSAR, _SAR_RM32_1)
DEFAULT_RM8_CL(doSAR, _SAR_RM8_CL)
DEFAULT_RM16_CL(doSAR, _SAR_RM16_CL)
DEFAULT_RM32_CL(doSAR, _SAR_RM32_CL)

DEFAULT_RM8_imm8(doRCL, _RCL_RM8_imm8)
DEFAULT_RM16_imm8(doRCL, _RCL_RM16_imm8)
DEFAULT_RM32_imm8(doRCL, _RCL_RM32_imm8)
DEFAULT_RM8_1(doRCL, _RCL_RM8_1)
DEFAULT_RM16_1(doRCL, _RCL_RM16_1)
DEFAULT_RM32_1(doRCL, _RCL_RM32_1)
DEFAULT_RM8_CL(doRCL, _RCL_RM8_CL)
DEFAULT_RM16_CL(doRCL, _RCL_RM16_CL)
DEFAULT_RM32_CL(doRCL, _RCL_RM32_CL)

DEFAULT_RM8_imm8(doRCR, _RCR_RM8_imm8)
DEFAULT_RM16_imm8(doRCR, _RCR_RM16_imm8)
DEFAULT_RM32_imm8(doRCR, _RCR_RM32_imm8)
DEFAULT_RM8_1(doRCR, _RCR_RM8_1)
DEFAULT_RM16_1(doRCR, _RCR_RM16_1)
DEFAULT_RM32_1(doRCR, _RCR_RM32_1)
DEFAULT_RM8_CL(doRCR, _RCR_RM8_CL)
DEFAULT_RM16_CL(doRCR, _RCR_RM16_CL)
DEFAULT_RM32_CL(doRCR, _RCR_RM32_CL)

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

template<typename T>
T CPU::doROL(T data, int steps)
{
    T result = data;
    steps &= 0x1f;
    if (!steps)
        return data;

    steps &= BitSizeOfType<T>::bits - 1;
    result = (data << steps) | (data >> (BitSizeOfType<T>::bits - steps));
    setCF(result & 1);
    setOF(((result >> (BitSizeOfType<T>::bits - 1)) & 1) ^ getCF());

    return result;
}

template<typename T>
T CPU::doROR(T data, int steps)
{
    steps &= 0x1f;
    if (!steps)
        return data;

    T result = data;
    steps &= BitSizeOfType<T>::bits - 1;
    result = (data >> steps) | (data << (BitSizeOfType<T>::bits - steps));
    setCF((result >> (BitSizeOfType<T>::bits - 1)) & 1);
    setOF((result >> (BitSizeOfType<T>::bits - 1)) ^ ((result >> (BitSizeOfType<T>::bits - 2) & 1)));
    return result;
}

template<typename T>
T CPU::doSHR(T data, int steps)
{
    T result = data;
    steps &= 0x1F;
    if (!steps)
        return data;

    if (steps <= BitSizeOfType<T>::bits) {
        setCF((result >> (steps - 1)) & 1);
        setOF((data >> (BitSizeOfType<T>::bits - 1)) & 1);
    }
    result >>= steps;

    updateFlags(result, BitSizeOfType<T>::bits);
    return result;
}

template<typename T>
T CPU::doSHL(T data, int steps)
{
    T result = data;
    steps &= 0x1F;
    if (!steps)
        return data;

    if (steps <= BitSizeOfType<T>::bits) {
        setCF(result >> (BitSizeOfType<T>::bits - steps) & 1);
    }
    result <<= steps;
    setOF((result >> (BitSizeOfType<T>::bits - 1)) ^ getCF());
    updateFlags(result, BitSizeOfType<T>::bits);
    return result;
}

template<typename T>
T CPU::doSAR(T data, int steps)
{
    // FIXME: This is painfully unoptimized.
    steps &= 0x1f;
    if (!steps)
        return data;

    T result = data;
    T mask = 1 << (BitSizeOfType<T>::bits - 1);

    for (int i = 0; i < steps; ++i) {
        T n = result;
        result = (result >> 1) | (n & mask);
        setCF(n & 1);
    }
    setOF(0);
    updateFlags(result, BitSizeOfType<T>::bits);
    return result;
}

template<typename T>
inline T allOnes()
{
    if (BitSizeOfType<T>::bits == 8)
        return 0xff;
    if (BitSizeOfType<T>::bits == 16)
        return 0xffff;
    if (BitSizeOfType<T>::bits == 32)
        return 0xffffffff;
}

template<typename T>
T CPU::doRCL(T data, int steps)
{
    // FIXME: This is painfully unoptimized.
    T result = data;
    T mask = allOnes<T>();
    steps &= 0x1f;
    if (!steps)
        return data;

    for (int i = 0; i < steps; ++i) {
        T n = result;
        result = ((result << 1) & mask) | getCF();
        setCF((n >> (BitSizeOfType<T>::bits - 1)) & 1);
    }
    setOF((result >> (BitSizeOfType<T>::bits - 1)) ^ getCF());
    return result;
}

template<typename T>
T CPU::doRCR(T data, int steps)
{
    // FIXME: This is painfully unoptimized.
    T result = data;
    steps &= 0x1f;
    if (!steps)
        return data;

    for (int i = 0; i < steps; ++i) {
        T n = result;
        result = (result >> 1) | (getCF() << (BitSizeOfType<T>::bits - 1));
        setCF(n & 1);
    }
    setOF((result >> (BitSizeOfType<T>::bits - 1)) ^ ((result >> (BitSizeOfType<T>::bits - 2) & 1)));
    return result;
}

template<typename T>
void CPU::doNOT(Instruction& insn)
{
    insn.modrm().write<T>(~insn.modrm().read<T>());
}

void CPU::_NOT_RM8(Instruction& insn)
{
    doNOT<BYTE>(insn);
}

void CPU::_NOT_RM16(Instruction& insn)
{
    doNOT<WORD>(insn);
}

void CPU::_NOT_RM32(Instruction& insn)
{
    doNOT<DWORD>(insn);
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

template<typename T>
T CPU::doBt(T src, int bitIndex)
{
    bitIndex &= BitSizeOfType<T>::bits - 1;
    setCF((src >> bitIndex) & 1);
    return src;
}

template<typename T>
T CPU::doBtr(T dest, int bitIndex)
{
    bitIndex &= BitSizeOfType<T>::bits - 1;
    T bitMask = 1 << bitIndex;
    T result = dest & ~bitMask;
    setCF((dest & bitMask) != 0);
    return result;
}

template<typename T>
T CPU::doBts(T dest, int bitIndex)
{
    bitIndex &= BitSizeOfType<T>::bits - 1;
    T bitMask = 1 << bitIndex;
    T result = dest | bitMask;
    setCF((dest & bitMask) != 0);
    return result;
}

template<typename T>
T CPU::doBtc(T dest, int bitIndex)
{
    bitIndex &= BitSizeOfType<T>::bits - 1;
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

template<typename T>
T CPU::doSHLD(T leftData, T rightData, int steps)
{
    steps &= 31;
    if (!steps)
        return leftData;

    T result;

    if (steps > BitSizeOfType<T>::bits) {
        result = (leftData >> ((BitSizeOfType<T>::bits * 2) - steps) | (rightData << (steps - BitSizeOfType<T>::bits)));
        setCF((rightData >> ((BitSizeOfType<T>::bits * 2) - steps)) & 1);
    } else {
        result = (leftData << steps) | (rightData >> (BitSizeOfType<T>::bits - steps));
        setCF((leftData >> (BitSizeOfType<T>::bits - steps)) & 1);
    }

    setOF(getCF() ^ (result >> (BitSizeOfType<T>::bits - 1) & 1));
    updateFlags(result, BitSizeOfType<T>::bits);
    return result;
}

void CPU::_SHLD_RM16_reg16_imm8(Instruction& insn)
{
    insn.modrm().write16(doSHLD(insn.modrm().read16(), insn.reg16(), insn.imm8()));
}

void CPU::_SHLD_RM32_reg32_imm8(Instruction& insn)
{
    insn.modrm().write32(doSHLD(insn.modrm().read32(), insn.reg32(), insn.imm8()));
}

void CPU::_SHLD_RM16_reg16_CL(Instruction& insn)
{
    insn.modrm().write16(doSHLD(insn.modrm().read16(), insn.reg16(), getCL()));
}

void CPU::_SHLD_RM32_reg32_CL(Instruction& insn)
{
    insn.modrm().write32(doSHLD(insn.modrm().read32(), insn.reg32(), getCL()));
}

template<typename T>
T CPU::doSHRD(T leftData, T rightData, int steps)
{
    steps &= 31;
    if (!steps)
        return rightData;

    T result;
    if (steps > BitSizeOfType<T>::bits) {
        result = (rightData << (32 - steps)) | (leftData >> (steps - BitSizeOfType<T>::bits));
        setCF((leftData >> (steps - (BitSizeOfType<T>::bits + 1))) & 1);
    } else {
        result = (rightData >> steps) | (leftData << (BitSizeOfType<T>::bits - steps));
        setCF((rightData >> (steps - 1)) & 1);
    }

    setOF((result ^ rightData) >> (BitSizeOfType<T>::bits - 1) & 1);
    updateFlags(result, BitSizeOfType<T>::bits);
    return result;
}

void CPU::_SHRD_RM16_reg16_imm8(Instruction& insn)
{
    insn.modrm().write16(doSHRD(insn.reg16(), insn.modrm().read16(), insn.imm8()));
}

void CPU::_SHRD_RM32_reg32_imm8(Instruction& insn)
{
    insn.modrm().write32(doSHRD(insn.reg32(), insn.modrm().read32(), insn.imm8()));
}

void CPU::_SHRD_RM16_reg16_CL(Instruction& insn)
{
    insn.modrm().write16(doSHRD(insn.reg16(), insn.modrm().read16(), getCL()));
}

void CPU::_SHRD_RM32_reg32_CL(Instruction& insn)
{
    insn.modrm().write32(doSHRD(insn.reg32(), insn.modrm().read32(), getCL()));
}
