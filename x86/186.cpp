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

void CPU::_ENTER16(Instruction& insn)
{
    WORD size = insn.imm16_2();
    BYTE nestingLevel = insn.imm8_1() & 31;
    push16(getBP());
    WORD frameTemp = getSP();

    if (nestingLevel > 0) {
        DWORD tmpEBP = currentBasePointer();
        for (WORD i = 1; i < nestingLevel - 1; ++i) {
            tmpEBP -= 2;
            push16(readMemory16(SegmentRegisterIndex::SS, currentBasePointer()));
        }
        push16(frameTemp);
    }
    setBP(frameTemp);
    adjustStackPointer(-size);
    snoop(SegmentRegisterIndex::SS, currentStackPointer(), MemoryAccessType::Write);
}

void CPU::_ENTER32(Instruction& insn)
{
    WORD size = insn.imm16_2();
    BYTE nestingLevel = insn.imm8_1() & 31;
    push32(getEBP());
    DWORD frameTemp = getESP();

    if (nestingLevel > 0) {
        DWORD tmpEBP = currentBasePointer();
        for (WORD i = 1; i < nestingLevel - 1; ++i) {
            tmpEBP -= 2;
            push32(readMemory32(SegmentRegisterIndex::SS, currentBasePointer()));
        }
        push32(frameTemp);
    }
    setEBP(frameTemp);
    adjustStackPointer(-size);
    snoop(SegmentRegisterIndex::SS, currentStackPointer(), MemoryAccessType::Write);
}

void CPU::_LEAVE16(Instruction&)
{
    WORD newBP = readMemory16(SegmentRegisterIndex::SS, currentBasePointer());
    setCurrentStackPointer(currentBasePointer() + 2);
    setCurrentBasePointer(newBP);
}

void CPU::_LEAVE32(Instruction&)
{
    DWORD newBP = readMemory32(SegmentRegisterIndex::SS, currentBasePointer());
    setCurrentStackPointer(currentBasePointer() + 4);
    setCurrentBasePointer(newBP);
}

void CPU::_PUSHA(Instruction&)
{
    snoop(SegmentRegisterIndex::SS, currentStackPointer(), MemoryAccessType::Write);
    snoop(SegmentRegisterIndex::SS, currentStackPointer() - 16, MemoryAccessType::Write);

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
    snoop(SegmentRegisterIndex::SS, currentStackPointer(), MemoryAccessType::Write);
    snoop(SegmentRegisterIndex::SS, currentStackPointer() - 32, MemoryAccessType::Write);

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
    snoop(SegmentRegisterIndex::SS, currentStackPointer(), MemoryAccessType::Read);
    snoop(SegmentRegisterIndex::SS, currentStackPointer() + 16, MemoryAccessType::Read);

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
    snoop(SegmentRegisterIndex::SS, currentStackPointer(), MemoryAccessType::Read);
    snoop(SegmentRegisterIndex::SS, currentStackPointer() + 32, MemoryAccessType::Read);

    setEDI(pop32());
    setESI(pop32());
    setEBP(pop32());
    (void) pop32();
    setEBX(pop32());
    setEDX(pop32());
    setECX(pop32());
    setEAX(pop32());
}
