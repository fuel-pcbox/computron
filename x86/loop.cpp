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

#define DO_REP do { \
    if (a32()) \
        for (; regs.D.ECX; --regs.D.ECX) { execute(insn); } \
    else \
        for (; regs.W.CX; --regs.W.CX) { execute(insn); } \
    } while(0)

#define DO_REPZ do { \
    if (a32()) { \
        if (getECX() == 0) \
            return; \
        for (setZF(shouldEqual); regs.D.ECX && (getZF() == shouldEqual); --regs.D.ECX) { execute(insn); } \
    } else { \
        if (getCX() == 0) \
            return; \
        for (setZF(shouldEqual); regs.W.CX && (getZF() == shouldEqual); --regs.W.CX) { execute(insn); } \
    } } while (0)

void CPU::handleRepeatOpcode(Instruction& insn, bool shouldEqual)
{
    switch(insn.op()) {
    case 0x66: {
        m_operandSize32 = !m_operandSize32;
        auto newInsn = Instruction::fromStream(*this, o32(), a32());
        handleRepeatOpcode(newInsn, shouldEqual);
        m_operandSize32 = !m_operandSize32;
        return;
    }

    case 0x67: {
        m_addressSize32 = !m_addressSize32;
        auto newInsn = Instruction::fromStream(*this, o32(), a32());
        handleRepeatOpcode(newInsn, shouldEqual);
        m_addressSize32 = !m_addressSize32;
        return;
    }

    case 0x6C: DO_REP; return; // INSB
    case 0x6D: DO_REP; return; // INSW/INSD
    case 0x6E: DO_REP; return; // OUTSB
    case 0x6F: DO_REP; return; // OUTSW/OUTSD
    case 0xA4: DO_REP; return; // MOVSB
    case 0xA5: DO_REP; return; // MOVSW/MOVSD
    case 0xAA: DO_REP; return; // STOSB
    case 0xAB: DO_REP; return; // STOSW/STOSD
    case 0xAC: DO_REP; return; // LODSB
    case 0xAD: DO_REP; return; // LODSW/LODSD
    case 0xA6: DO_REPZ; return; // CMPSB
    case 0xA7: DO_REPZ; return; // CMPSW/CMPSD
    case 0xAE: DO_REPZ; return; // SCASB
    case 0xAF: DO_REPZ; return; // SCASW/SCASD

    case 0x90:
        // PAUSE / REP NOP
        execute(insn);
        return;

    default:
        throw InvalidOpcode(QString("Opcode %1 used with REP* prefix").arg(insn.op(), 2, 16, QLatin1Char('0')));
    }
}

void CPU::_REP(Instruction&)
{
    auto newInsn = Instruction::fromStream(*this, o32(), a32());
    handleRepeatOpcode(newInsn, true);
}

void CPU::_REPNE(Instruction&)
{
    auto newInsn = Instruction::fromStream(*this, o32(), a32());
    handleRepeatOpcode(newInsn, false);
}
