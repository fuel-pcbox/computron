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
#include "debug.h"

void VCpu::push32(DWORD value)
{
    //vlog(LogCPU, "push32: %08X", value);
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
    //vlog(LogCPU, "push16: %04X", value);
    if (a16()) {
        if (o16())
            this->regs.W.SP -= 2;
        else
            this->regs.W.SP -= 4;
        writeMemory16(getSS(), this->getSP(), value);
    } else {
        if (o16())
            this->regs.D.ESP -= 2;
        else
            this->regs.D.ESP -= 4;
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
    //vlog(LogCPU, "pop32: %08X", d);
    return d;
}

WORD VCpu::pop()
{
    WORD w;
    if (a16()) {
        w = readMemory16(getSS(), this->getSP());
        if (o16())
            this->regs.W.SP += 2;
        else
            this->regs.W.SP += 4;
    } else {
        w = readMemory16(getSS(), this->getESP());
        if (o16())
            this->regs.D.ESP += 2;
        else
            this->regs.D.ESP += 4;
    }
    //vlog(LogCPU, "pop16: %08X", w);
    return w;
}

void VCpu::_PUSH_reg16(Instruction& insn)
{
    push(insn.reg16());
}

void VCpu::_PUSH_reg32(Instruction& insn)
{
    push32(insn.reg32());
}

void VCpu::_POP_reg16(Instruction& insn)
{
    insn.reg16() = pop();
}

void VCpu::_POP_reg32(Instruction& insn)
{
    insn.reg32() = pop32();
}

void VCpu::_PUSH_RM16(Instruction& insn)
{
    push(insn.modrm().read16());
}

void VCpu::_PUSH_RM32(Instruction& insn)
{
    push32(insn.modrm().read32());
}

void VCpu::_POP_RM16(Instruction& insn)
{
    insn.modrm().write16(pop());
}

void VCpu::_POP_RM32(Instruction& insn)
{
    insn.modrm().write32(pop32());
}

void VCpu::_PUSH_CS(Instruction&)
{
    push(getCS());
}

void VCpu::_PUSH_DS(Instruction&)
{
    push(getDS());
}

void VCpu::_PUSH_ES(Instruction&)
{
    push(getES());
}

void VCpu::_PUSH_SS(Instruction&)
{
    push(getSS());
}

void VCpu::_PUSH_FS(Instruction&)
{
    push(getFS());
}

void VCpu::_PUSH_GS(Instruction&)
{
    push(getGS());
}

void VCpu::_POP_DS(Instruction&)
{
    setDS(pop());
}

void VCpu::_POP_ES(Instruction&)
{
    setES(pop());
}

void VCpu::_POP_SS(Instruction&)
{
    setSS(pop());
}

void VCpu::_POP_FS(Instruction&)
{
    setFS(pop());
}

void VCpu::_POP_GS(Instruction&)
{
    setGS(pop());
}

void VCpu::_PUSHFD(Instruction&)
{
    if (!getPE() || (getPE() && ((!getVM() || (getVM() && getIOPL() == 3)))))
        push32(getEFlags() & 0x00FCFFFF);
    else
        GP(0);
}

void VCpu::_PUSH_imm32(Instruction& insn)
{
    push32(insn.imm32());
}

void VCpu::_PUSHF(Instruction&)
{
    if (!getPE() || (getPE() && ((!getVM() || (getVM() && getIOPL() == 3)))))
        push(getFlags());
    else
        GP(0);
}

void VCpu::_POPF(Instruction&)
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

void VCpu::_POPFD(Instruction&)
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
