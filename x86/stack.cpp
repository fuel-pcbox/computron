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
#include "debug.h"

void VCpu::push32(DWORD value)
{
    if (a16()) {
        this->regs.W.SP -= 4;
        writeMemory32(getSS(), this->getSP(), value);
    } else {
        this->regs.D.ESP -= 4;
        writeMemory32(getSS(), this->getESP(), value);
    }
}

void VCpu::push(WORD value)
{
    if (a16()) {
        this->regs.W.SP -= 2;
        writeMemory16(getSS(), this->getSP(), value);
    } else {
        this->regs.D.ESP -= 2;
        writeMemory16(getSS(), this->getESP(), value);
    }
}

DWORD VCpu::pop32()
{
    DWORD d;
    if (a16()) {
        d = readMemory32(getSS(), this->getSP());
        this->regs.W.SP += 4;
    } else {
        d = readMemory32(getSS(), this->getESP());
        this->regs.D.ESP += 4;
    }
    return d;
}

WORD VCpu::pop()
{
    WORD w;
    if (a16()) {
        w = readMemory16(getSS(), this->getSP());
        this->regs.W.SP += 2;
    } else {
        w = readMemory16(getSS(), this->getESP());
        this->regs.D.ESP += 2;
    }
    return w;
}

void VCpu::_PUSH_reg16()
{
    push(*treg16[this->opcode & 7]);
}

void VCpu::_PUSH_reg32()
{
    push32(*treg32[this->opcode & 7]);
}


void VCpu::_POP_reg16()
{
    *treg16[this->opcode & 7] = pop();
}

void VCpu::_POP_reg32()
{
    *treg32[this->opcode & 7] = pop32();
}

void VCpu::_PUSH_RM16()
{
    push(readModRM16(rmbyte));
}

void VCpu::_POP_RM16()
{
    writeModRM16(rmbyte, pop());
}

void VCpu::_POP_RM32()
{
    writeModRM32(rmbyte, pop32());
}

void VCpu::_PUSH_CS()
{
    push(getCS());
}

void VCpu::_PUSH_DS()
{
    push(getDS());
}

void VCpu::_PUSH_ES()
{
    push(getES());
}

void VCpu::_PUSH_SS()
{
    push(getSS());
}

void VCpu::_PUSH_FS()
{
    push(getFS());
}

void VCpu::_PUSH_GS()
{
    push(getGS());
}

void VCpu::_POP_DS()
{
    DS = pop();
}

void VCpu::_POP_ES()
{
    ES = pop();
}

void VCpu::_POP_SS()
{
    SS = pop();
}

void VCpu::_POP_FS()
{
    FS = pop();
}

void VCpu::_POP_GS()
{
    GS = pop();
}

void VCpu::_PUSHFD()
{
    if (!getPE() || (getPE() && ((!getVM() || (getVM() && getIOPL() == 3)))))
        push32(getEFlags() & 0x00FCFFFF);
    else
        GP(0);
}

void VCpu::_PUSH_imm32()
{
    push32(fetchOpcodeDWord());
}

void VCpu::_PUSHF()
{
    if (!getPE() || (getPE() && ((!getVM() || (getVM() && getIOPL() == 3)))))
        push(getFlags());
    else
        GP(0);
}

void VCpu::_POPF()
{
    if (!getVM()) {
        if (getCPL() == 0)
            setFlags(pop());
        else {
            bool oldIOPL = getIOPL();
            setFlags(pop());
            setIOPL(oldIOPL);
        }
    } else {
        if (getIOPL() == 3) {
            bool oldIOPL = getIOPL();
            setFlags(pop());
            setIOPL(oldIOPL);
        } else
            GP(0);
    }
}

void VCpu::_POPFD()
{
    if (!getVM()) {
        if (getCPL() == 0)
            setEFlags(pop32());
        else {
            bool oldIOPL = getIOPL();
            setEFlags(pop32());
            setIOPL(oldIOPL);
        }
    } else {
        if (getIOPL() == 3) {
            bool oldIOPL = getIOPL();
            setEFlags(pop32());
            setIOPL(oldIOPL);
        } else
            GP(0);
    }
}
