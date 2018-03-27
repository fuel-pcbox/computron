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
#include "debug.h"

void CPU::pushSegmentRegisterValue(WORD value)
{
    if (o16()) {
        push16(value);
        return;
    }
    writeMemory16(SegmentRegisterIndex::SS, currentStackPointer() - 4, value);
    adjustStackPointer(-4);
    if (UNLIKELY(options.stacklog))
        vlog(LogCPU, "push32: %04x (at esp=%08x, special 16-bit write for segment registers)", value, getESP());
}

void CPU::push32(DWORD value)
{
    writeMemory32(SegmentRegisterIndex::SS, currentStackPointer() - 4, value);
    adjustStackPointer(-4);
    if (UNLIKELY(options.stacklog))
        vlog(LogCPU, "push32: %08x (at esp=%08x)", value, getESP());
}

void CPU::push16(WORD value)
{
    writeMemory16(SegmentRegisterIndex::SS, currentStackPointer() - 2, value);
    adjustStackPointer(-2);
    if (UNLIKELY(options.stacklog))
        vlog(LogCPU, "push16: %04x (at esp=%08x)", value, getESP());
}

DWORD CPU::pop32()
{
    DWORD data = readMemory32(SegmentRegisterIndex::SS, currentStackPointer());
    if (UNLIKELY(options.stacklog))
        vlog(LogCPU, "pop32: %08x (from esp=%08x)", data, getESP());
    adjustStackPointer(4);
    return data;
}

WORD CPU::pop16()
{
    WORD data = readMemory16(SegmentRegisterIndex::SS, currentStackPointer());
    if (UNLIKELY(options.stacklog))
        vlog(LogCPU, "pop16: %04x (from esp=%08x)", data, getESP());
    adjustStackPointer(2);
    return data;
}

void CPU::_PUSH_reg16(Instruction& insn)
{
    push16(insn.reg16());
}

void CPU::_PUSH_reg32(Instruction& insn)
{
    push32(insn.reg32());
}

void CPU::_POP_reg16(Instruction& insn)
{
    insn.reg16() = pop16();
}

void CPU::_POP_reg32(Instruction& insn)
{
    insn.reg32() = pop32();
}

void CPU::_PUSH_RM16(Instruction& insn)
{
    push16(insn.modrm().read16());
}

void CPU::_PUSH_RM32(Instruction& insn)
{
    push32(insn.modrm().read32());
}

// From the IA-32 manual:
// "If the ESP register is used as a base register for addressing a destination operand in memory,
// the POP instruction computes the effective address of the operand after it increments the ESP register."

void CPU::_POP_RM16(Instruction& insn)
{
    // See comment above.
    auto data = pop16();
    insn.modrm().resolve(*this);
    insn.modrm().write16(data);
}

void CPU::_POP_RM32(Instruction& insn)
{
    // See comment above.
    auto data = pop32();
    insn.modrm().resolve(*this);
    insn.modrm().write32(data);
}

void CPU::_PUSH_CS(Instruction&)
{
    pushSegmentRegisterValue(getCS());
}

void CPU::_PUSH_DS(Instruction&)
{
    pushSegmentRegisterValue(getDS());
}

void CPU::_PUSH_ES(Instruction&)
{
    pushSegmentRegisterValue(getES());
}

void CPU::_PUSH_SS(Instruction&)
{
    pushSegmentRegisterValue(getSS());
}

void CPU::_PUSH_FS(Instruction&)
{
    pushSegmentRegisterValue(getFS());
}

void CPU::_PUSH_GS(Instruction&)
{
    pushSegmentRegisterValue(getGS());
}

void CPU::_POP_DS(Instruction&)
{
    if (o16())
        setDS(pop16());
    else
        setDS(pop32());
}

void CPU::_POP_ES(Instruction&)
{
    if (o16())
        setES(pop16());
    else
        setES(pop32());
}

void CPU::_POP_SS(Instruction&)
{
    if (o16())
        setSS(pop16());
    else
        setSS(pop32());
    makeNextInstructionUninterruptible();
}

void CPU::_POP_FS(Instruction&)
{
    if (o16())
        setFS(pop16());
    else
        setFS(pop32());
}

void CPU::_POP_GS(Instruction&)
{
    if (o16())
        setGS(pop16());
    else
        setGS(pop32());
}

void CPU::_PUSHFD(Instruction&)
{
    if (!getPE() || (getPE() && ((!getVM() || (getVM() && getIOPL() == 3)))))
        push32(getEFlags() & 0x00FCFFFF);
    else
        throw GeneralProtectionFault(0, "PUSHFD");
}

void CPU::_PUSH_imm32(Instruction& insn)
{
    push32(insn.imm32());
}

void CPU::_PUSHF(Instruction&)
{
    if (!getPE() || (getPE() && ((!getVM() || (getVM() && getIOPL() == 3)))))
        push16(getFlags());
    else
        throw GeneralProtectionFault(0, "PUSHF");
}

void CPU::_POPF(Instruction&)
{
    setEFlagsRespectfully(pop16());
}

void CPU::_POPFD(Instruction&)
{
    setEFlagsRespectfully(pop32());
}

void CPU::setEFlagsRespectfully(DWORD newFlags)
{
    RELEASE_ASSERT(!getVM());
    DWORD oldFlags = getEFlags();
    DWORD flagsToKeep = Flag::VIP | Flag::VIF | Flag::RF;
    if (o16())
        flagsToKeep |= 0xffff0000;
    if (getPE() && getCPL() != 0) {
        flagsToKeep |= Flag::IOPL;
        if (getCPL() > getIOPL()) {
            flagsToKeep |= Flag::IF;
        }
    }
    newFlags &= ~flagsToKeep;
    newFlags |= oldFlags & flagsToKeep;
    newFlags &= ~Flag::RF;
    setEFlags(newFlags);
}
