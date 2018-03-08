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

#include "debugger.h"

void CPU::_LOOP_imm8(Instruction& insn)
{
    SIGNED_BYTE displacement = insn.imm8();
    if (a32()) {
        --regs.D.ECX;
        if (regs.D.ECX)
            jumpRelative8(displacement);
    } else {
        --regs.W.CX;
        if (regs.W.CX)
            jumpRelative8(displacement);
    }
}

void CPU::_LOOPE_imm8(Instruction& insn)
{
    SIGNED_BYTE displacement = insn.imm8();
    if (a32()) {
        --regs.D.ECX;
        if (regs.D.ECX && getZF())
            jumpRelative8(displacement);
    } else {
        --regs.W.CX;
        if (regs.W.CX && getZF())
            jumpRelative8(displacement);
    }
}

void CPU::_LOOPNE_imm8(Instruction& insn)
{
    SIGNED_BYTE displacement = insn.imm8();
    if (a32()) {
        --regs.D.ECX;
        if (regs.D.ECX && !getZF())
            jumpRelative8(displacement);
    } else {
        --regs.W.CX;
        if (regs.W.CX && !getZF())
            jumpRelative8(displacement);
    }
}

#define CALL_HANDLER(handler16, handler32) if (o16()) { handler16(insn); } else { handler32(insn); }

#define DO_REP_NEW(o16Handler, o32Handler) do { \
    if (a32()) \
        for (; regs.D.ECX; --regs.D.ECX) { CALL_HANDLER(o16Handler, o32Handler); } \
    else \
        for (; regs.W.CX; --regs.W.CX) { CALL_HANDLER(o16Handler, o32Handler); } \
    } while(0)

#define DO_REP(func) do { \
    if (a32()) \
        for (; regs.D.ECX; --regs.D.ECX) { func(insn); } \
    else \
        for (; regs.W.CX; --regs.W.CX) { func(insn); } \
    } while(0)

#define DO_REPZ(func) do { \
    if (a32()) \
        for (setZF(shouldEqual); regs.D.ECX && (getZF() == shouldEqual); --regs.D.ECX) { func(insn); } \
    else \
        for (setZF(shouldEqual); regs.W.CX && (getZF() == shouldEqual); --regs.W.CX) { func(insn); } \
    } while (0)

#define DO_REPZ_NEW(o16Handler, o32Handler) do { \
    if (a32()) \
        for (setZF(shouldEqual); regs.D.ECX && (getZF() == shouldEqual); --regs.D.ECX) { CALL_HANDLER(o16Handler, o32Handler); } \
    else \
        for (setZF(shouldEqual); regs.W.CX && (getZF() == shouldEqual); --regs.W.CX) { CALL_HANDLER(o16Handler, o32Handler); } \
    } while (0)

void CPU::handleRepeatOpcode(Instruction&& insn, bool shouldEqual)
{
    switch(insn.op()) {
    case 0x26: setSegmentPrefix(SegmentRegisterIndex::ES); break;
    case 0x2E: setSegmentPrefix(SegmentRegisterIndex::CS); break;
    case 0x36: setSegmentPrefix(SegmentRegisterIndex::SS); break;
    case 0x3E: setSegmentPrefix(SegmentRegisterIndex::DS); break;
    case 0x64: setSegmentPrefix(SegmentRegisterIndex::FS); break;
    case 0x65: setSegmentPrefix(SegmentRegisterIndex::GS); break;

    case 0x66: {
        m_operandSize32 = !m_operandSize32;
        auto newInsn = Instruction::fromStream(*this, o32(), a32());
        handleRepeatOpcode(std::move(newInsn), shouldEqual);
        m_operandSize32 = !m_operandSize32;
        return;
    }

    case 0x67: {
        m_addressSize32 = !m_addressSize32;
        auto newInsn = Instruction::fromStream(*this, o32(), a32());
        handleRepeatOpcode(std::move(newInsn), shouldEqual);
        m_addressSize32 = !m_addressSize32;
        return;
    }

    case 0x6E: DO_REP(_OUTSB); return;
    case 0x6F: DO_REP_NEW(_OUTSW, _OUTSD); return;

    case 0xA4: DO_REP(_MOVSB); return;
    case 0xA5: DO_REP_NEW(_MOVSW, _MOVSD); return;
    case 0xAA: DO_REP(_STOSB); return;
    case 0xAB: DO_REP_NEW(_STOSW, _STOSD); return;
    case 0xAC: DO_REP(_LODSB); return;
    case 0xAD: DO_REP_NEW(_LODSW, _LODSD); return;

    case 0xA6: DO_REPZ(_CMPSB); return;
    case 0xA7: DO_REPZ_NEW(_CMPSW, _CMPSD); return;
    case 0xAE: DO_REPZ(_SCASB); return;
    case 0xAF: DO_REPZ_NEW(_SCASW, _SCASD); return;

    case 0x90:
        // PAUSE / REP NOP
        execute(std::move(insn));
        return;

    default:
        debugger().enter();
        vlog(LogAlert, "SUSPICIOUS: Opcode %02X used with REP* prefix", insn.op());
        execute(std::move(insn));
        return;
    }

    // Recurse if this opcode was a segment prefix.
    // FIXME: Infinite recursion IS possible here.
    auto newInsn = Instruction::fromStream(*this, o32(), a32());
    handleRepeatOpcode(std::move(newInsn), shouldEqual);
}

void CPU::_REP(Instruction&)
{
    auto newInsn = Instruction::fromStream(*this, o32(), a32());
    handleRepeatOpcode(std::move(newInsn), true);
    resetSegmentPrefix();
}

void CPU::_REPNE(Instruction&)
{
    auto newInsn = Instruction::fromStream(*this, o32(), a32());
    handleRepeatOpcode(std::move(newInsn), false);
    resetSegmentPrefix();
}
