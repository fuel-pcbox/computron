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

void CPU::_AAA(Instruction&)
{
    if (((regs.B.AL & 0x0F)>9) || getAF()) {
        regs.B.AL += 6;
        regs.B.AH += 1;
        setAF(1);
        setCF(1);
    } else {
        setAF(0);
        setCF(0);
    }
    regs.B.AL &= 0x0F;
}

void CPU::_AAM(Instruction& insn)
{
    if (insn.imm8() == 0) {
        exception(0);
        return;
    }

    BYTE tempAL = regs.B.AL;
    regs.B.AH = tempAL / insn.imm8();
    regs.B.AL = tempAL % insn.imm8();
    updateFlags8(regs.B.AL);
}

void CPU::_AAD(Instruction& insn)
{
    BYTE tempAL = regs.B.AL;
    BYTE tempAH = regs.B.AH;

    regs.B.AL = (tempAL + (tempAH * insn.imm8())) & 0xFF;
    regs.B.AH = 0x00;
    updateFlags8(regs.B.AL);
}

void CPU::_AAS(Instruction&)
{
    if (((regs.B.AL & 0x0F) > 9) || getAF()) {
        regs.B.AL -= 6;
        regs.B.AH -= 1;
        setAF(1);
        setCF(1);
    } else {
        setAF(0);
        setCF(0);
    }
}

void CPU::_DAS(Instruction&)
{
    bool oldCF = getCF();
    BYTE oldAL = regs.B.AL;

    setCF(0);

    if (((regs.B.AL & 0x0F) > 0x09) || getAF()) {
        setCF(((regs.B.AL - 6) >> 8) & 1);
        regs.B.AL -= 0x06;
        setCF(oldCF | getCF());
        setAF(1);
    } else {
        setAF(0);
    }

    if (oldAL > 0x99 || oldCF == 1) {
        regs.B.AL -= 0x60;
        setCF(1);
    } else {
        setCF(0);
    }
}

void CPU::_DAA(Instruction&)
{
    bool oldCF = getCF();
    BYTE oldAL = regs.B.AL;

    setCF(0);

    if (((regs.B.AL & 0x0F) > 0x09) || getAF()) {
        setCF(((regs.B.AL + 6) >> 8) & 1);
        regs.B.AL += 6;
        setCF(oldCF | getCF());
        setAF(1);
    } else {
        setAF(0);
    }

    if (oldAL > 0x99 || oldCF == 1) {
        regs.B.AL += 0x60;
        setCF(1);
    } else {
        setCF(0);
    }
}
