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

void CPU::_JCXZ_imm8(Instruction& insn)
{
    bool shouldJump;
    if (a16())
        shouldJump = getCX() == 0;
    else
        shouldJump = getECX() == 0;
    if (shouldJump)
        jumpRelative8(insn.imm8());
}

void CPU::_JMP_imm16(Instruction& insn)
{
    jumpRelative16(insn.imm16());
}

void CPU::_JMP_imm32(Instruction& insn)
{
    jumpRelative32(insn.imm32());
}

void CPU::_JMP_imm16_imm16(Instruction& insn)
{
    WORD newCS = insn.imm16_1();
    WORD newIP = insn.imm16_2();
    jump16(newCS, newIP, JumpType::JMP);
}

void CPU::_JMP_imm16_imm32(Instruction& insn)
{
    WORD newCS = insn.imm16_1();
    DWORD newEIP = insn.imm32_2();
    jump32(newCS, newEIP, JumpType::JMP);
}

void CPU::_JMP_short_imm8(Instruction& insn)
{
    jumpRelative8(insn.imm8());
}

void CPU::_JMP_RM16(Instruction& insn)
{
    jumpAbsolute16(insn.modrm().read16());
}

void CPU::_JMP_RM32(Instruction& insn)
{
    jumpAbsolute32(insn.modrm().read32());
}

template<typename T>
void CPU::doFarJump(Instruction& insn, JumpType jumpType)
{
    T offset = readMemory<T>(insn.modrm().segment(), insn.modrm().offset());
    WORD selector = readMemory16(insn.modrm().segment(), insn.modrm().offset() + sizeof(T));
    jump32(selector, offset, jumpType);
}

void CPU::_JMP_FAR_mem16(Instruction& insn)
{
    doFarJump<WORD>(insn, JumpType::JMP);
}

void CPU::_JMP_FAR_mem32(Instruction& insn)
{
    doFarJump<DWORD>(insn, JumpType::JMP);
}

void CPU::_CALL_FAR_mem16(Instruction& insn)
{
    doFarJump<WORD>(insn, JumpType::CALL);
}

void CPU::_CALL_FAR_mem32(Instruction& insn)
{
    doFarJump<DWORD>(insn, JumpType::CALL);
}

void CPU::_SETcc_RM8(Instruction& insn)
{
    insn.modrm().write8(evaluate(insn.cc()));
}

void CPU::_Jcc_imm8(Instruction& insn)
{
    if (evaluate(insn.cc()))
        jumpRelative8(insn.imm8());
}

void CPU::_Jcc_NEAR_imm(Instruction& insn)
{
    if (!evaluate(insn.cc()))
        return;
    if (a16())
        jumpRelative16(insn.imm16());
    else
        jumpRelative32(insn.imm32());
}

void CPU::_CALL_imm16(Instruction& insn)
{
    push16(getIP());
    jumpRelative16(insn.imm16());
}

void CPU::_CALL_imm32(Instruction& insn)
{
    push32(getEIP());
    jumpRelative32(insn.imm32());
}

void CPU::_CALL_imm16_imm16(Instruction& insn)
{
    jump16(insn.imm16_1(), insn.imm16_2(), JumpType::CALL);
}

void CPU::_CALL_imm16_imm32(Instruction& insn)
{
    jump32(insn.imm16_1(), insn.imm32_2(), JumpType::CALL);
}

void CPU::_CALL_RM16(Instruction& insn)
{
    push16(getIP());
    jumpAbsolute16(insn.modrm().read16());
}

void CPU::_CALL_RM32(Instruction& insn)
{
    push32(getEIP());
    jumpAbsolute32(insn.modrm().read32());
}

void CPU::_RET(Instruction&)
{
    if (o32())
        jumpAbsolute32(pop32());
    else
        jumpAbsolute16(pop16());
}

void CPU::_RET_imm16(Instruction& insn)
{
    if (o32())
        jumpAbsolute32(pop32());
    else
        jumpAbsolute16(pop16());
    adjustStackPointer(insn.imm16());
}

void CPU::_RETF(Instruction&)
{
    if (o32()) {
        DWORD nip = pop32();
        WORD ncs = pop32();
        jump32(ncs, nip, JumpType::RETF);
    } else {
        WORD nip = pop16();
        WORD ncs = pop16();
        jump16(ncs, nip, JumpType::RETF);
    }
}

void CPU::_RETF_imm16(Instruction& insn)
{
    BYTE originalCPL = getCPL();
    if (o32()) {
        DWORD nip = pop32();
        WORD ncs = pop32();
        adjustStackPointer(insn.imm16());
        jump32(ncs, nip, JumpType::RETF);
    } else {
        WORD nip = pop16();
        WORD ncs = pop16();
        adjustStackPointer(insn.imm16());
        jump16(ncs, nip, JumpType::RETF);
    }
    if (getCPL() != originalCPL) {
        ASSERT_NOT_REACHED();
    }
}

void CPU::doLOOP(Instruction& insn, bool condition)
{
    if (!decrementCXForAddressSize() && condition)
        jumpRelative8(static_cast<SIGNED_BYTE>(insn.imm8()));
}

void CPU::_LOOP_imm8(Instruction& insn)
{
    doLOOP(insn, true);
}

void CPU::_LOOPZ_imm8(Instruction& insn)
{
    doLOOP(insn, getZF());
}

void CPU::_LOOPNZ_imm8(Instruction& insn)
{
    doLOOP(insn, !getZF());
}
