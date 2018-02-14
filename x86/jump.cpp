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

void CPU::_JCXZ_imm8(Instruction& insn)
{
    SIGNED_BYTE imm = insn.imm8();
    bool shouldJump;
    if (a16())
        shouldJump = getCX() == 0;
    else
        shouldJump = getECX() == 0;
    if (shouldJump)
        jumpRelative8(imm);
}

void CPU::_JMP_imm16(Instruction& insn)
{
    SIGNED_WORD imm = insn.imm16();
    jumpRelative16(imm);
}

void CPU::_JMP_imm32(Instruction& insn)
{
    SIGNED_DWORD imm = insn.imm32();
    jumpRelative32(imm);
}

void CPU::_JMP_imm16_imm16(Instruction& insn)
{
    WORD newCS = insn.imm16_1();
    WORD newIP = insn.imm16_2();
    jump16(newCS, newIP);
}

void CPU::_JMP_imm16_imm32(Instruction& insn)
{
    WORD newCS = insn.imm16_1();
    DWORD newEIP = insn.imm32_2();
    jump32(newCS, newEIP);
}

void CPU::_JMP_short_imm8(Instruction& insn)
{
    SIGNED_BYTE imm = insn.imm8();
    jumpRelative8(imm);
}

void CPU::_JMP_RM16(Instruction& insn)
{
    jumpAbsolute16(insn.modrm().read16());
}

void CPU::_JMP_RM32(Instruction& insn)
{
    jumpAbsolute32(insn.modrm().read32());
}

void CPU::_JMP_FAR_mem16(Instruction& insn)
{
    WORD* ptr = static_cast<WORD*>(insn.modrm().memoryPointer());
    jump16(ptr[1], ptr[0]);
}

void CPU::_JMP_FAR_mem32(Instruction& insn)
{
    WORD* ptr = static_cast<WORD*>(insn.modrm().memoryPointer());
    jump32(ptr[2], makeDWORD(ptr[1], ptr[0]));
}

ALWAYS_INLINE bool CPU::evaluateCondition(BYTE cc) const
{
    switch (cc) {
    case 0: return getOF();
    case 1: return !getOF();
    case 2: return getCF();
    case 3: return !getCF();
    case 4: return getZF();
    case 5: return !getZF();
    case 6: return getCF() | getZF();
    case 7: return !(getCF() | getZF());
    case 8: return getSF();
    case 9: return !getSF();
    case 10: return getPF();
    case 11: return !getPF();
    case 12: return getSF() ^ getOF();
    case 13: return !(getSF() ^ getOF());
    case 14: return (getSF() ^ getOF()) | getZF();
    case 15: return !((getSF() ^ getOF()) | getZF());
    }
    VM_ASSERT(false);
    return false;
}

void CPU::_SETcc_RM8(Instruction& insn)
{
    insn.modrm().write8(evaluateCondition(insn.cc()));
}

void CPU::_Jcc_imm8(Instruction& insn)
{
    if (evaluateCondition(insn.cc()))
        jumpRelative8(insn.imm8());
}

void CPU::_Jcc_NEAR_imm(Instruction& insn)
{
    if (!evaluateCondition(insn.cc()))
        return;
    if (a16())
        jumpRelative16(insn.imm16());
    else
        jumpRelative32(insn.imm32());
}

void CPU::_CALL_imm16(Instruction& insn)
{
    SIGNED_WORD imm = insn.imm16();
    pushInstructionPointer();
    jumpRelative16(imm);
}

void CPU::_CALL_imm32(Instruction& insn)
{
    SIGNED_DWORD imm = insn.imm32();
    pushInstructionPointer();
    jumpRelative32(imm);
}

void CPU::_CALL_imm16_imm16(Instruction& insn)
{
    push(getCS());
    pushInstructionPointer();
    jump16(insn.imm16_1(), insn.imm16_2());
}

void CPU::_CALL_imm16_imm32(Instruction& insn)
{
    push(getCS());
    pushInstructionPointer();
    jump32(insn.imm16_1(), insn.imm32_2());
}

void CPU::_CALL_FAR_mem16(Instruction& insn)
{
    WORD* ptr = static_cast<WORD*>(insn.modrm().memoryPointer());
    push(getCS());
    pushInstructionPointer();
    jump16(ptr[1], ptr[0]);
}

void CPU::_CALL_FAR_mem32(Instruction&)
{
    // FIXME: Implement!
    VM_ASSERT(false);
}

void CPU::_CALL_RM16(Instruction& insn)
{
    pushInstructionPointer();
    jumpAbsolute16(insn.modrm().read16());
}

void CPU::_CALL_RM32(Instruction& insn)
{
    pushInstructionPointer();
    jumpAbsolute32(insn.modrm().read32());
}

void CPU::_RET(Instruction&)
{
    if (o32())
        jumpAbsolute32(pop32());
    else
        jumpAbsolute16(pop());
}

void CPU::_RET_imm16(Instruction& insn)
{
    if (o32()) {
        jumpAbsolute32(pop32());
        regs.D.ESP += insn.imm16();
    } else {
        jumpAbsolute16(pop());
        regs.W.SP += insn.imm16();
    }
}

void CPU::_RETF(Instruction&)
{
    if (o32()) {
        DWORD nip = pop32();
        jump32(pop(), nip);
    } else {
        WORD nip = pop();
        jump16(pop(), nip);
    }
}

void CPU::_RETF_imm16(Instruction& insn)
{
    if (o32()) {
        DWORD nip = pop32();
        jump32(pop(), nip);
        regs.D.ESP += insn.imm16();
    } else {
        WORD nip = pop();
        jump16(pop(), nip);
        regs.W.SP += insn.imm16();
    }
}
