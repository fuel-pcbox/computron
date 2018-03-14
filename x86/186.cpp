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

#include "Common.h"
#include "CPU.h"
#include "debug.h"

#define DEBUG_BOUND

void CPU::_BOUND(Instruction& insn)
{
    QString reason;
    bool isWithinBounds;
    if (o32()) {
        SIGNED_DWORD arrayIndex = insn.reg32();
        SIGNED_DWORD* bounds = static_cast<SIGNED_DWORD*>(insn.modrm().memoryPointer());
        isWithinBounds = arrayIndex >= bounds[0] && arrayIndex <= bounds[1];
#ifdef DEBUG_BOUND
        vlog(LogCPU, "BOUND32 checking if %d is within [%d, %d]: %s",
            arrayIndex,
            bounds[0],
            bounds[1],
            isWithinBounds ? "yes" : "no");
#endif
        if (!isWithinBounds) {
            reason = QString("%1 not within [%2, %3]").arg(arrayIndex).arg(bounds[0]).arg(bounds[1]);
        }
    } else {
        SIGNED_WORD arrayIndex = insn.reg16();
        SIGNED_WORD* bounds = static_cast<SIGNED_WORD*>(insn.modrm().memoryPointer());
        isWithinBounds = arrayIndex >= bounds[0] && arrayIndex <= bounds[1];
#ifdef DEBUG_BOUND
        vlog(LogCPU, "BOUND16 checking if %d is within [%d, %d]: %s",
            arrayIndex,
            bounds[0],
            bounds[1],
            isWithinBounds ? "yes" : "no");
#endif
        if (!isWithinBounds) {
            reason = QString("%1 not within [%2, %3]").arg(arrayIndex).arg(bounds[0]).arg(bounds[1]);
        }
    }
    if (!isWithinBounds) {
        throw BoundRangeExceeded(reason);
    }
}

void CPU::_PUSH_imm8(Instruction& insn)
{
    if (o32())
        push32(signExtend<DWORD>(insn.imm8()));
    else
        push16(signExtend<WORD>(insn.imm8()));
}

void CPU::_PUSH_imm16(Instruction& insn)
{
    push16(insn.imm16());
}

void CPU::_ENTER_16(Instruction& insn)
{
    ASSERT(o16());
    ASSERT(a16());
    //ASSERT(s16());

    WORD size = insn.imm16_2();
    BYTE nestingLevel = insn.imm8_1() & 31;
    push16(getBP());
    WORD frameTemp = getSP();
    if (nestingLevel > 0) {
        WORD tmpEBP = getEBP();
        for (WORD i = 1; i < nestingLevel - 1; ++i) {
            tmpEBP -= 2;
            push16(readMemory16(SegmentRegisterIndex::SS, getEBP()));
        }
        push16(frameTemp);
    }
    setBP(frameTemp);
    setSP(getSP() - size);
}


void CPU::_ENTER_32(Instruction& insn)
{
    ASSERT(o32());
    ASSERT(a32());
    //ASSERT(s32());

    WORD size = insn.imm16_2();
    BYTE nestingLevel = insn.imm8_1() & 31;
    push32(getEBP());
    WORD frameTemp = getESP();
    if (nestingLevel > 0) {
        WORD tmpEBP = getEBP();
        for (WORD i = 1; i < nestingLevel - 1; ++i) {
            tmpEBP -= 4;
            push32(readMemory32(SegmentRegisterIndex::SS, getEBP()));
        }
        push32(frameTemp);
    }
    setEBP(frameTemp);
    setESP(getESP() - size);
}

void CPU::_LEAVE(Instruction&)
{
    if (s16()) {
        setSP(getBP());
    } else {
        setESP(getEBP());
    }

    if (o16()) {
        setBP(pop16());
    } else {
        setEBP(pop32());
    }
}

void CPU::_PUSHA(Instruction&)
{
    WORD oldSP = getSP();
    push16(getAX());
    push16(getCX());
    push16(getDX());
    push16(getBX());
    push16(oldSP);
    push16(getBP());
    push16(getSI());
    push16(getDI());
}

void CPU::_PUSHAD(Instruction&)
{
    DWORD oldESP = getESP();
    push32(getEAX());
    push32(getECX());
    push32(getEDX());
    push32(getEBX());
    push32(oldESP);
    push32(getEBP());
    push32(getESI());
    push32(getEDI());
}

void CPU::_POPA(Instruction&)
{
    setDI(pop16());
    setSI(pop16());
    setBP(pop16());
    (void) pop16();
    setBX(pop16());
    setDX(pop16());
    setCX(pop16());
    setAX(pop16());
}

void CPU::_POPAD(Instruction&)
{
    setEDI(pop32());
    setESI(pop32());
    setEBP(pop32());
    (void) pop32();
    setEBX(pop32());
    setEDX(pop32());
    setECX(pop32());
    setEAX(pop32());
}
