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

void CPU::doREP(Instruction& insn)
{
    while (readRegisterForAddressSize(RegisterCX) != 0) {
        execute(insn);
        decrementCXForAddressSize();
    }
}

void CPU::doREPZ(Instruction& insn, bool wantedZF)
{
    while (readRegisterForAddressSize(RegisterCX) != 0) {
        execute(insn);
        decrementCXForAddressSize();
        if (getZF() != wantedZF)
            break;
    }
}

void CPU::handleRepeatOpcode(Instruction& insn, bool wantedZF)
{
    switch(insn.op()) {
    case 0x6C: doREP(insn); return; // INSB
    case 0x6D: doREP(insn); return; // INSW/INSD
    case 0x6E: doREP(insn); return; // OUTSB
    case 0x6F: doREP(insn); return; // OUTSW/OUTSD
    case 0xA4: doREP(insn); return; // MOVSB
    case 0xA5: doREP(insn); return; // MOVSW/MOVSD
    case 0xAA: doREP(insn); return; // STOSB
    case 0xAB: doREP(insn); return; // STOSW/STOSD
    case 0xAC: doREP(insn); return; // LODSB
    case 0xAD: doREP(insn); return; // LODSW/LODSD
    case 0xA6: doREPZ(insn, wantedZF); return; // CMPSB
    case 0xA7: doREPZ(insn, wantedZF); return; // CMPSW/CMPSD
    case 0xAE: doREPZ(insn, wantedZF); return; // SCASB
    case 0xAF: doREPZ(insn, wantedZF); return; // SCASW/SCASD

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
    auto newInsn = Instruction::fromStream(*this, m_operandSize32, m_addressSize32);
    handleRepeatOpcode(newInsn, true);
}

void CPU::_REPNZ(Instruction&)
{
    auto newInsn = Instruction::fromStream(*this, m_operandSize32, m_addressSize32);
    handleRepeatOpcode(newInsn, false);
}
