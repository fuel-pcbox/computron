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

#include "vcpu.h"
#include "templates.h"

template<typename T>
inline void updateCpuMathFlags(CPU& cpu, QWORD result, T dest, T src)
{
    if (BitSizeOfType<T>::bits == 8)
        cpu.mathFlags8(result, dest, src);
    else if (BitSizeOfType<T>::bits == 16)
        cpu.mathFlags16(result, dest, src);
    else if (BitSizeOfType<T>::bits == 32)
        cpu.mathFlags32(result, dest, src);
}

template<typename T>
inline void updateCpuCmpFlags(CPU& cpu, QWORD result, T dest, T src)
{
    if (BitSizeOfType<T>::bits == 8)
        cpu.cmpFlags8(result, dest, src);
    else if (BitSizeOfType<T>::bits == 16)
        cpu.cmpFlags16(result, dest, src);
    else if (BitSizeOfType<T>::bits == 32)
        cpu.cmpFlags32(result, dest, src);
}

template<typename T>
QWORD CPU::doAdd(T dest, T src)
{
    QWORD result = (QWORD)dest + (QWORD)src;
    updateCpuMathFlags(*this, result, dest, src);
    setOF(((
          ((result)^(dest)) &
          ((result)^(src))
         )>>(BitSizeOfType<T>::bits - 1))&1);
    return result;
}

template<typename T>
QWORD CPU::doAdc(T dest, T src)
{
    QWORD result = (QWORD)dest + (QWORD)src + (QWORD)getCF();

    updateCpuMathFlags(*this, result, dest, src);
    setOF(((
          ((result)^(dest)) &
          ((result)^(src))
         )>>(BitSizeOfType<T>::bits))&1);
    return result;
}

template<typename T>
QWORD CPU::doSub(T dest, T src)
{
    QWORD result = (QWORD)dest - (QWORD)src;
    updateCpuCmpFlags(*this, result, dest, src);
    return result;
}

template<typename T>
QWORD CPU::doSbb(T dest, T src)
{
    QWORD result = (QWORD)dest - (QWORD)src - (QWORD)getCF();
    updateCpuCmpFlags(*this, result, dest, src);
    return result;
}

template<typename T>
QWORD CPU::doMul(T acc, T multi)
{
    QWORD result = (QWORD)acc * (QWORD)multi;
    updateCpuMathFlags(*this, result, acc, multi);
    return result;
}

template<typename T>
SIGNED_QWORD CPU::doImul(T acc, T multi)
{
    // FIXME: This function should protect against T being an unsigned type.
    SIGNED_QWORD result = (SIGNED_QWORD)acc * (SIGNED_QWORD)multi;
    return result;
}

DEFAULT_RM8_reg8(doAdd, _ADD_RM8_reg8)
DEFAULT_RM16_reg16(doAdd, _ADD_RM16_reg16)
DEFAULT_RM32_reg32(doAdd, _ADD_RM32_reg32)
DEFAULT_reg8_RM8(doAdd, _ADD_reg8_RM8)
DEFAULT_reg16_RM16(doAdd, _ADD_reg16_RM16)
DEFAULT_reg32_RM32(doAdd, _ADD_reg32_RM32)
DEFAULT_RM8_imm8(doAdd, _ADD_RM8_imm8)
DEFAULT_RM16_imm16(doAdd, _ADD_RM16_imm16)
DEFAULT_RM32_imm32(doAdd, _ADD_RM32_imm32)
DEFAULT_RM16_imm8(doAdd, _ADD_RM16_imm8)
DEFAULT_RM32_imm8(doAdd, _ADD_RM32_imm8)
DEFAULT_AL_imm8(doAdd, _ADD_AL_imm8)
DEFAULT_AX_imm16(doAdd, _ADD_AX_imm16)
DEFAULT_EAX_imm32(doAdd, _ADD_EAX_imm32)

DEFAULT_RM8_reg8(doAdc, _ADC_RM8_reg8)
DEFAULT_RM16_reg16(doAdc, _ADC_RM16_reg16)
DEFAULT_RM32_reg32(doAdc, _ADC_RM32_reg32)
DEFAULT_reg8_RM8(doAdc, _ADC_reg8_RM8)
DEFAULT_reg16_RM16(doAdc, _ADC_reg16_RM16)
DEFAULT_reg32_RM32(doAdc, _ADC_reg32_RM32)
DEFAULT_RM8_imm8(doAdc, _ADC_RM8_imm8)
DEFAULT_RM16_imm16(doAdc, _ADC_RM16_imm16)
DEFAULT_RM32_imm32(doAdc, _ADC_RM32_imm32)
DEFAULT_RM16_imm8(doAdc, _ADC_RM16_imm8)
DEFAULT_RM32_imm8(doAdc, _ADC_RM32_imm8)
DEFAULT_AL_imm8(doAdc, _ADC_AL_imm8)
DEFAULT_AX_imm16(doAdc, _ADC_AX_imm16)
DEFAULT_EAX_imm32(doAdc, _ADC_EAX_imm32)

DEFAULT_RM8_reg8(doSub, _SUB_RM8_reg8)
DEFAULT_RM16_reg16(doSub, _SUB_RM16_reg16)
DEFAULT_RM32_reg32(doSub, _SUB_RM32_reg32)
DEFAULT_reg8_RM8(doSub, _SUB_reg8_RM8)
DEFAULT_reg16_RM16(doSub, _SUB_reg16_RM16)
DEFAULT_reg32_RM32(doSub, _SUB_reg32_RM32)
DEFAULT_RM8_imm8(doSub, _SUB_RM8_imm8)
DEFAULT_RM16_imm16(doSub, _SUB_RM16_imm16)
DEFAULT_RM32_imm32(doSub, _SUB_RM32_imm32)
DEFAULT_RM16_imm8(doSub, _SUB_RM16_imm8)
DEFAULT_RM32_imm8(doSub, _SUB_RM32_imm8)
DEFAULT_AL_imm8(doSub, _SUB_AL_imm8)
DEFAULT_AX_imm16(doSub, _SUB_AX_imm16)
DEFAULT_EAX_imm32(doSub, _SUB_EAX_imm32)

DEFAULT_RM8_reg8(doSbb, _SBB_RM8_reg8)
DEFAULT_RM16_reg16(doSbb, _SBB_RM16_reg16)
DEFAULT_RM32_reg32(doSbb, _SBB_RM32_reg32)
DEFAULT_reg8_RM8(doSbb, _SBB_reg8_RM8)
DEFAULT_reg16_RM16(doSbb, _SBB_reg16_RM16)
DEFAULT_reg32_RM32(doSbb, _SBB_reg32_RM32)
DEFAULT_RM8_imm8(doSbb, _SBB_RM8_imm8)
DEFAULT_RM16_imm16(doSbb, _SBB_RM16_imm16)
DEFAULT_RM32_imm32(doSbb, _SBB_RM32_imm32)
DEFAULT_RM16_imm8(doSbb, _SBB_RM16_imm8)
DEFAULT_RM32_imm8(doSbb, _SBB_RM32_imm8)
DEFAULT_AL_imm8(doSbb, _SBB_AL_imm8)
DEFAULT_AX_imm16(doSbb, _SBB_AX_imm16)
DEFAULT_EAX_imm32(doSbb, _SBB_EAX_imm32)

READONLY_RM8_reg8(doSub, _CMP_RM8_reg8)
READONLY_RM16_reg16(doSub, _CMP_RM16_reg16)
READONLY_RM32_reg32(doSub, _CMP_RM32_reg32)
READONLY_reg8_RM8(doSub, _CMP_reg8_RM8)
READONLY_reg16_RM16(doSub, _CMP_reg16_RM16)
READONLY_reg32_RM32(doSub, _CMP_reg32_RM32)
READONLY_RM8_imm8(doSub, _CMP_RM8_imm8)
READONLY_RM16_imm16(doSub, _CMP_RM16_imm16)
READONLY_RM32_imm32(doSub, _CMP_RM32_imm32)
READONLY_RM16_imm8(doSub, _CMP_RM16_imm8)
READONLY_RM32_imm8(doSub, _CMP_RM32_imm8)
READONLY_AL_imm8(doSub, _CMP_AL_imm8)
READONLY_AX_imm16(doSub, _CMP_AX_imm16)
READONLY_EAX_imm32(doSub, _CMP_EAX_imm32)

void CPU::_MUL_RM8(Instruction& insn)
{
    regs.W.AX = doMul(regs.B.AL, insn.modrm().read8());

    if (regs.B.AH == 0x00) {
        setCF(0);
        setOF(0);
    } else {
        setCF(1);
        setOF(1);
    }
}

void CPU::_MUL_RM16(Instruction& insn)
{
    DWORD result = doMul(regs.W.AX, insn.modrm().read16());
    regs.W.AX = result & 0xFFFF;
    regs.W.DX = (result >> 16) & 0xFFFF;

    if (regs.W.DX == 0x0000) {
        setCF(0);
        setOF(0);
    } else {
        setCF(1);
        setOF(1);
    }
}

void CPU::_MUL_RM32(Instruction& insn)
{
    QWORD result = doMul(regs.D.EAX, insn.modrm().read32());
    setEAX(result & 0xFFFFFFFF);
    setEDX((result >> 32) & 0xFFFFFFFF);

    if (getEDX() == 0x00000000) {
        setCF(0);
        setOF(0);
    } else {
        setCF(1);
        setOF(1);
    }
}

void CPU::_IMUL_RM8(Instruction& insn)
{
    SIGNED_BYTE value = insn.modrm().read8();
    SIGNED_WORD result = doImul(static_cast<SIGNED_BYTE>(getAL()), value);
    regs.W.AX = result;

    if (result > 0x7F || result < -0x80) {
        setCF(1);
        setOF(1);
    } else {
        setCF(0);
        setOF(0);
    }
}

void CPU::_IMUL_reg32_RM32_imm8(Instruction&)
{
    vlog(LogCPU, "Not implemented: IMUL reg32,rm32,imm8");
    vomit_exit(1);
}

void CPU::_IMUL_reg32_RM32_imm32(Instruction&)
{
    vlog(LogCPU, "Not implemented: IMUL reg32,rm32,imm32");
    vomit_exit(1);
}

void CPU::_IMUL_reg16_RM16_imm16(Instruction& insn)
{
    SIGNED_WORD value = insn.modrm().read16();
    SIGNED_DWORD result = doImul(value, static_cast<SIGNED_WORD>(insn.imm16()));

    insn.reg16() = result;

    if (result > 0x7FFF || result < -0x8000) {
        setCF(1);
        setOF(1);
    } else {
        setCF(0);
        setOF(0);
    }
}

void CPU::_IMUL_reg16_RM16(Instruction& insn)
{
    SIGNED_WORD src = insn.modrm().read16();
    SIGNED_WORD dest = insn.reg16();
    SIGNED_DWORD result = doImul(dest, src);

    insn.reg16() = result;

    if (result > 0x7FFF || result < -0x8000) {
        setCF(1);
        setOF(1);
    } else {
        setCF(0);
        setOF(0);
    }
}

void CPU::_IMUL_reg32_RM32(Instruction& insn)
{
    SIGNED_DWORD src = insn.modrm().read32();
    SIGNED_DWORD dest = insn.reg32();
    SIGNED_QWORD result = doImul(dest, src);

    insn.reg32() = result;

    if (result > 0x7FFFFFFF || result < -0x80000000) {
        setCF(1);
        setOF(1);
    } else {
        setCF(0);
        setOF(0);
    }
}

void CPU::_IMUL_reg16_RM16_imm8(Instruction& insn)
{
    SIGNED_WORD value = insn.modrm().read16();
    SIGNED_DWORD result = doImul(value, static_cast<SIGNED_WORD>(insn.imm8()));

    insn.reg16() = result;

    if (result > 0x7FFF || result < -0x8000) {
        setCF(1);
        setOF(1);
    } else {
        setCF(0);
        setOF(0);
    }
}

void CPU::_IMUL_RM16(Instruction& insn)
{
    SIGNED_WORD value = insn.modrm().read16();
    SIGNED_DWORD result = doImul(static_cast<SIGNED_WORD>(getAX()), value);
    regs.W.AX = result;
    regs.W.DX = result >> 16;

    if (result > 0x7FFF || result < -0x8000) {
        setCF(1);
        setOF(1);
    } else {
        setCF(0);
        setOF(0);
    }
}

void CPU::_IMUL_RM32(Instruction&)
{
    VM_ASSERT(false);
}

void CPU::_DIV_RM8(Instruction& insn)
{
    auto value = insn.modrm().read8();
    WORD tAX = regs.W.AX;

    if (value == 0) {
        exception(0);
        return;
    }

    // FIXME: divide error if result overflows
    regs.B.AL = (BYTE)(tAX / value); // Quote
    regs.B.AH = (BYTE)(tAX % value); // Remainder
}

void CPU::_DIV_RM16(Instruction& insn)
{
    auto value = insn.modrm().read16();
    DWORD tDXAX = regs.W.AX + (regs.W.DX << 16);

    if (value == 0) {
        exception(0);
        return;
    }

    // FIXME: divide error if result overflows
    regs.W.AX = (WORD)(tDXAX / value); // Quote
    regs.W.DX = (WORD)(tDXAX % value); // Remainder
}

void CPU::_DIV_RM32(Instruction& insn)
{
    DWORD value = insn.modrm().read32();
    QWORD tEDXEAX = getEAX() | ((QWORD)getEDX() << 32);

    if (value == 0) {
        exception(0);
        return;
    }

    // FIXME: divide error if result overflows
    setEAX(tEDXEAX / value); // Quote
    setEDX(tEDXEAX % value); // Remainder
}

void CPU::_IDIV_RM8(Instruction& insn)
{
    SIGNED_BYTE value = (SIGNED_BYTE)insn.modrm().read8();
    SIGNED_WORD tAX = (SIGNED_WORD)regs.W.AX;

    if (value == 0) {
        exception(0);
        return;
    }

    // FIXME: divide error if result overflows
    regs.B.AL = (SIGNED_BYTE)(tAX / value); // Quote
    regs.B.AH = (SIGNED_BYTE)(tAX % value); // Remainder
}

void CPU::_IDIV_RM16(Instruction& insn)
{
    SIGNED_WORD value = insn.modrm().read16();
    SIGNED_DWORD tDXAX = (regs.W.AX + (regs.W.DX << 16));

    if (value == 0) {
        exception(0);
        return;
    }

    // FIXME: divide error if result overflows
    regs.W.AX = (SIGNED_WORD)(tDXAX / value); // Quote
    regs.W.DX = (SIGNED_WORD)(tDXAX % value); // Remainder
}

void CPU::_IDIV_RM32(Instruction& insn)
{
    SIGNED_DWORD value = insn.modrm().read32();
    SIGNED_QWORD tEDXEAX = ((QWORD)regs.D.EAX + ((QWORD)regs.D.EDX << 32));

    if (value == 0) {
        exception(0);
        return;
    }

    // FIXME: divide error if result overflows
    regs.D.EAX = (SIGNED_DWORD)(tEDXEAX / value); // Quote
    regs.D.EDX = (SIGNED_DWORD)(tEDXEAX % value); // Remainder
}

void CPU::_NEG_RM8(Instruction& insn)
{
    auto& modrm = insn.modrm();
    modrm.write8(doSub((BYTE)0, modrm.read8()));
}

void CPU::_NEG_RM16(Instruction& insn)
{
    auto& modrm = insn.modrm();
    modrm.write16(doSub((WORD)0, modrm.read16()));
}

void CPU::_NEG_RM32(Instruction& insn)
{
    auto& modrm = insn.modrm();
    modrm.write32(doSub((DWORD)0, modrm.read32()));
}
