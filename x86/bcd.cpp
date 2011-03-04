/*
 * Copyright (C) 2003-2011 Andreas Kling <kling@webkit.org>
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

void _AAA(VCpu* cpu)
{
    if (((cpu->regs.B.AL & 0x0F)>9) || cpu->getAF()) {
        cpu->regs.B.AL += 6;
        cpu->regs.B.AH += 1;
        cpu->setAF(1);
        cpu->setCF(1);
    } else {
        cpu->setAF(0);
        cpu->setCF(0);
    }
    cpu->regs.B.AL &= 0x0F;
}

void _AAM(VCpu* cpu)
{
    BYTE imm = cpu->fetchOpcodeByte();

    if (imm == 0) {
        cpu->exception(0);
        return;
    }

    BYTE tempAL = cpu->regs.B.AL;
    cpu->regs.B.AH = tempAL / imm;
    cpu->regs.B.AL = tempAL % imm;
    cpu->updateFlags8(cpu->regs.B.AL);
}

void _AAD(VCpu* cpu)
{
    BYTE tempAL = cpu->regs.B.AL;
    BYTE tempAH = cpu->regs.B.AH;
    BYTE imm = cpu->fetchOpcodeByte();

    cpu->regs.B.AL = (tempAL + (tempAH * imm)) & 0xFF;
    cpu->regs.B.AH = 0x00;
    cpu->updateFlags8(cpu->regs.B.AL);
}

void _AAS(VCpu* cpu)
{
    if (((cpu->regs.B.AL & 0x0F) > 9) || cpu->getAF()) {
        cpu->regs.B.AL -= 6;
        cpu->regs.B.AH -= 1;
        cpu->setAF(1);
        cpu->setCF(1);
    } else {
        cpu->setAF(0);
        cpu->setCF(0);
    }
}

void _DAS(VCpu* cpu)
{
    bool oldCF = cpu->getCF();
    BYTE oldAL = cpu->regs.B.AL;

    cpu->setCF(0);

    if (((cpu->regs.B.AL & 0x0F) > 0x09) || cpu->getAF()) {
        cpu->setCF(((cpu->regs.B.AL - 6) >> 8) & 1);
        cpu->regs.B.AL -= 0x06;
        cpu->setCF(oldCF | cpu->getCF());
        cpu->setAF(1);
    } else {
        cpu->setAF(0);
    }

    if (oldAL > 0x99 || oldCF == 1) {
        cpu->regs.B.AL -= 0x60;
        cpu->setCF(1);
    } else {
        cpu->setCF(0);
    }
}

void _DAA(VCpu* cpu)
{
    bool oldCF = cpu->getCF();
    BYTE oldAL = cpu->regs.B.AL;

    cpu->setCF(0);

    if (((cpu->regs.B.AL & 0x0F) > 0x09) || cpu->getAF()) {
        cpu->setCF(((cpu->regs.B.AL + 6) >> 8) & 1);
        cpu->regs.B.AL += 6;
        cpu->setCF(oldCF | cpu->getCF());
        cpu->setAF(1);
    } else {
        cpu->setAF(0);
    }

    if (oldAL > 0x99 || oldCF == 1) {
        cpu->regs.B.AL += 0x60;
        cpu->setCF(1);
    } else {
        cpu->setCF(0);
    }
}
