// Computron x86 PC Emulator
// Copyright (C) 2003-2018 Andreas Kling <awesomekling@gmail.com>
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY ANDREAS KLING ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANDREAS KLING OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "CPU.h"
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
QWORD CPU::doADD(T dest, T src)
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
QWORD CPU::doADC(T dest, T src)
{
    QWORD result = (QWORD)dest + (QWORD)src + (QWORD)getCF();

    updateCpuMathFlags(*this, result, dest, src);
    setOF(((
          ((result)^(dest)) &
          ((result)^(src))
         )>>(BitSizeOfType<T>::bits - 1))&1);
    return result;
}

template<typename T>
QWORD CPU::doSUB(T dest, T src)
{
    QWORD result = (QWORD)dest - (QWORD)src;
    cmpFlags<T>(result, dest, src);
    return result;
}

template<typename T>
QWORD CPU::doSBB(T dest, T src)
{
    QWORD result = (QWORD)dest - (QWORD)src - (QWORD)getCF();
    cmpFlags<T>(result, dest, src);
    return result;
}

template<typename T>
SIGNED_QWORD CPU::doIMUL(T acc, T multi)
{
    // FIXME: This function should protect against T being an unsigned type.
    SIGNED_QWORD result = (SIGNED_QWORD)acc * (SIGNED_QWORD)multi;
    return result;
}

DEFINE_INSTRUCTION_HANDLERS_GRP1(ADD)
DEFINE_INSTRUCTION_HANDLERS_GRP1(ADC)
DEFINE_INSTRUCTION_HANDLERS_GRP1(SUB)
DEFINE_INSTRUCTION_HANDLERS_GRP1(SBB)
DEFINE_INSTRUCTION_HANDLERS_GRP4_READONLY(SUB, CMP)

template<typename T>
void CPU::doMUL(T f1, T f2, T& resultHigh, T& resultLow)
{
    typedef typename TypeDoubler<T>::type DT;
    DT result = (DT)f1 * (DT)f2;
    resultLow = result & MasksForType<T>::allBits;
    resultHigh = (result >> BitSizeOfType<T>::bits) & MasksForType<T>::allBits;

    if (resultHigh == 0) {
        setCF(0);
        setOF(0);
    } else {
        setCF(1);
        setOF(1);
    }
}

void CPU::_MUL_RM8(Instruction& insn)
{
    doMUL<BYTE>(getAL(), insn.modrm().read8(), regs.B.AH, regs.B.AL);
}

void CPU::_MUL_RM16(Instruction& insn)
{
    doMUL<WORD>(getAX(), insn.modrm().read16(), regs.W.DX, regs.W.AX);
}

void CPU::_MUL_RM32(Instruction& insn)
{
    doMUL<DWORD>(getEAX(), insn.modrm().read32(), regs.D.EDX, regs.D.EAX);
}

void CPU::_IMUL_RM8(Instruction& insn)
{
    SIGNED_BYTE value = insn.modrm().read8();
    SIGNED_WORD result = doIMUL(static_cast<SIGNED_BYTE>(getAL()), value);
    regs.W.AX = result;

    if (result > 0x7F || result < -0x80) {
        setCF(1);
        setOF(1);
    } else {
        setCF(0);
        setOF(0);
    }
}

void CPU::_IMUL_reg32_RM32_imm8(Instruction& insn)
{
    SIGNED_DWORD value = insn.modrm().read32();
    SIGNED_QWORD result = doIMUL(value, signExtend<SIGNED_DWORD>(insn.imm8()));

    insn.reg32() = result;

    if (result > 0x7FFFFFFFLL || result < -0x80000000LL) {
        setCF(1);
        setOF(1);
    } else {
        setCF(0);
        setOF(0);
    }
}

void CPU::_IMUL_reg32_RM32_imm32(Instruction& insn)
{
    SIGNED_DWORD value = insn.modrm().read32();
    SIGNED_QWORD result = doIMUL(value, static_cast<SIGNED_DWORD>(insn.imm32()));

    insn.reg32() = result;

    if (result > 0x7FFFFFFFLL || result < -0x80000000LL) {
        setCF(1);
        setOF(1);
    } else {
        setCF(0);
        setOF(0);
    }
}

void CPU::_IMUL_reg16_RM16_imm16(Instruction& insn)
{
    SIGNED_WORD value = insn.modrm().read16();
    SIGNED_DWORD result = doIMUL(value, static_cast<SIGNED_WORD>(insn.imm16()));

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
    SIGNED_DWORD result = doIMUL(dest, src);

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
    SIGNED_QWORD result = doIMUL(dest, src);

    insn.reg32() = result;

    if (result > 0x7FFFFFFFLL || result < -0x80000000LL) {
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
    SIGNED_DWORD result = doIMUL(value, signExtend<SIGNED_WORD>(insn.imm8()));

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
    SIGNED_DWORD result = doIMUL(static_cast<SIGNED_WORD>(getAX()), value);
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

void CPU::_IMUL_RM32(Instruction& insn)
{
    SIGNED_DWORD value = insn.modrm().read32();
    SIGNED_QWORD result = doIMUL(static_cast<SIGNED_DWORD>(getEAX()), value);
    regs.D.EAX = result;
    regs.D.EDX = result >> 32;

    if (result > 0x7FFFFFFFLL || result < -0x80000000LL) {
        setCF(1);
        setOF(1);
    } else {
        setCF(0);
        setOF(0);
    }
}

template<typename T>
void CPU::doDIV(T dividendHigh, T dividendLow, T divisor, T& quotient, T& remainder)
{
    if (divisor == 0) {
        throw DivideError("Divide by zero");
    }

    typedef typename TypeDoubler<T>::type DT;
    DT dividend = weld<DT>(dividendHigh, dividendLow);

    DT result = dividend / divisor;
    if (result > MasksForType<T>::allBits) {
        throw DivideError(QString("Unsigned divide overflow (%1 / %2 = %3)").arg(dividend).arg(divisor).arg(result));
    }

    quotient = result;
    remainder = dividend % divisor;
}

void CPU::_DIV_RM8(Instruction& insn)
{
    doDIV<BYTE>(getAH(), getAL(), insn.modrm().read8(), regs.B.AL, regs.B.AH);
}

void CPU::_DIV_RM16(Instruction& insn)
{
    doDIV<WORD>(getDX(), getAX(), insn.modrm().read16(), regs.W.AX, regs.W.DX);
}

void CPU::_DIV_RM32(Instruction& insn)
{
    doDIV<DWORD>(getEDX(), getEAX(), insn.modrm().read32(), regs.D.EAX, regs.D.EDX);
}

void CPU::_IDIV_RM8(Instruction& insn)
{
    SIGNED_BYTE value = (SIGNED_BYTE)insn.modrm().read8();
    SIGNED_WORD tAX = (SIGNED_WORD)regs.W.AX;

    if (value == 0) {
        throw DivideError("Divide by zero");
    }

    SIGNED_WORD result = tAX / value;
    if (result > 0x7f || result < -0x80) {
        throw DivideError(QString("Signed divide overflow (%1 / %2 = %3)").arg(tAX).arg(value).arg(result));
    }

    regs.B.AL = (SIGNED_BYTE)(result);
    regs.B.AH = (SIGNED_BYTE)(tAX % value);
}

void CPU::_IDIV_RM16(Instruction& insn)
{
    SIGNED_WORD value = insn.modrm().read16();
    SIGNED_DWORD tDXAX = (regs.W.AX + (regs.W.DX << 16));

    if (value == 0) {
        throw DivideError("Divide by zero");
    }

    SIGNED_DWORD result = tDXAX / value;
    if (result > 0x7fff || result < -0x8000) {
        throw DivideError(QString("Signed divide overflow (%1 / %2 = %3)").arg(tDXAX).arg(value).arg(result));
    }

    regs.W.AX = (SIGNED_WORD)(result);
    regs.W.DX = (SIGNED_WORD)(tDXAX % value);
}

void CPU::_IDIV_RM32(Instruction& insn)
{
    SIGNED_DWORD value = insn.modrm().read32();
    SIGNED_QWORD tEDXEAX = ((QWORD)regs.D.EAX + ((QWORD)regs.D.EDX << 32));

    if (value == 0) {
        throw DivideError("Divide by zero");
    }

    SIGNED_QWORD result = tEDXEAX / value;
    if (result > 0x7fffffffLL || result < -0x80000000LL) {
        throw DivideError(QString("Signed divide overflow (%1 / %2 = %3)").arg(tEDXEAX).arg(value).arg(result));
    }

    regs.D.EAX = (SIGNED_DWORD)(result);
    regs.D.EDX = (SIGNED_DWORD)(tEDXEAX % value);
}

template<typename T>
void CPU::doNEG(Instruction& insn)
{
    insn.modrm().write<T>(doSUB((T)0, insn.modrm().read<T>()));
}

void CPU::_NEG_RM8(Instruction& insn)
{
    doNEG<BYTE>(insn);
}

void CPU::_NEG_RM16(Instruction& insn)
{
    doNEG<WORD>(insn);
}

void CPU::_NEG_RM32(Instruction& insn)
{
    doNEG<DWORD>(insn);
}
