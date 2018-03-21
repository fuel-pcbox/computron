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

bool CPU::getPF() const
{
    if (m_dirtyFlags & Flag::PF) {
        PF = 0x9669 << 2 >> ((m_lastResult ^ m_lastResult >> 4) & 0xf) & Flag::PF;
        m_dirtyFlags &= ~Flag::PF;
    }
    return PF;
}

bool CPU::getZF() const
{
    if (m_dirtyFlags & Flag::ZF) {
        ZF = (~m_lastResult & (m_lastResult - 1)) >> (m_lastOpSize - 1) & 1;
        m_dirtyFlags &= ~Flag::ZF;
    }
    return ZF;
}

bool CPU::getSF() const
{
    if (m_dirtyFlags & Flag::SF) {
        SF = (m_lastResult >> (m_lastOpSize - 1)) & 1;
        m_dirtyFlags &= ~Flag::SF;
    }
    return SF;
}

void CPU::adjustFlag32(QWORD result, DWORD src, DWORD dest)
{
    setAF((((result ^ (src ^ dest)) & 0x10) >> 4) & 1);
}

void CPU::updateFlags32(DWORD data)
{
    m_dirtyFlags |= Flag::PF | Flag::ZF | Flag::SF;
    m_lastResult = data;
    m_lastOpSize = DWordSize;
}

void CPU::updateFlags16(WORD data)
{
    m_dirtyFlags |= Flag::PF | Flag::ZF | Flag::SF;
    m_lastResult = data;
    m_lastOpSize = WordSize;
}

void CPU::updateFlags8(BYTE data)
{
    m_dirtyFlags |= Flag::PF | Flag::ZF | Flag::SF;
    m_lastResult = data;
    m_lastOpSize = ByteSize;
}

void CPU::updateFlags(DWORD data, BYTE bits)
{
    m_dirtyFlags |= Flag::PF | Flag::ZF | Flag::SF;
    m_lastResult = data;
    m_lastOpSize = bits;
}

void CPU::_STC(Instruction&)
{
    setCF(1);
}

void CPU::_STD(Instruction&)
{
    setDF(1);
}

void CPU::_STI(Instruction&)
{
    if (!getPE()) {
        makeNextInstructionUninterruptible();
        setIF(1);
        return;
    }

    if (!getVM()) {
        if (getIOPL() >= getCPL()) {
            makeNextInstructionUninterruptible();
            setIF(1);
        } else {
            if ((getIOPL() < getCPL()) && getCPL() == 3 && !getVIP())
                setVIF(1);
            else
                throw GeneralProtectionFault(0, "STI with VM=0");
        }
    } else {
        if (getIOPL() == 3) {
            setIF(0);
        } else {
            if (getIOPL() < 3 && !getVIP() && getVME()) {
                setVIF(1);
            } else {
                throw GeneralProtectionFault(0, "STI with VM=1");
            }
        }
    }
}

void CPU::_CLI(Instruction&)
{
    if (!getPE()) {
        setIF(0);
        return;
    }

    if (!getVM()) {
        if (getIOPL() >= getCPL()) {
            setIF(0);
        } else {
            if ((getIOPL() < getCPL()) && getCPL() == 3 && getPVI())
                setVIF(0);
            else
                throw GeneralProtectionFault(0, "CLI with VM=0");
        }
    } else {
        if (getIOPL() == 3) {
            setIF(0);
        } else {
            if (getIOPL() < 3 && getVME()) {
                setVIF(0);
            } else {
                throw GeneralProtectionFault(0, "CLI with VM=1");
            }
        }
    }
}

void CPU::_CLC(Instruction&)
{
    setCF(0);
}

void CPU::_CLD(Instruction&)
{
    setDF(0);
}

void CPU::_CMC(Instruction&)
{
    setCF(!getCF());
}

void CPU::_LAHF(Instruction&)
{
    regs.B.AH = getCF() | (getPF() * 4) | (getAF() * 16) | (getZF() * 64) | (getSF() * 128) | 2;
}

void CPU::_SAHF(Instruction&)
{
    setCF(regs.B.AH & 0x01);
    setPF(regs.B.AH & 0x04);
    setAF(regs.B.AH & 0x10);
    setZF(regs.B.AH & 0x40);
    setSF(regs.B.AH & 0x80);
}

void CPU::mathFlags8(WORD result, BYTE dest, BYTE src)
{
    m_dirtyFlags |= Flag::PF | Flag::ZF | Flag::SF;
    m_lastResult = result;
    m_lastOpSize = ByteSize;

    setCF(result & 0xFF00);
    adjustFlag32(result, dest, src);
}

void CPU::mathFlags16(DWORD result, WORD dest, WORD src)
{
    m_dirtyFlags |= Flag::PF | Flag::ZF | Flag::SF;
    m_lastResult = result;
    m_lastOpSize = WordSize;

    setCF(result & 0xFFFF0000);
    setSF(result & 0x8000);
    adjustFlag32(result, dest, src);
}

void CPU::mathFlags32(QWORD result, DWORD dest, DWORD src)
{
    m_dirtyFlags |= Flag::PF | Flag::ZF | Flag::SF;
    m_lastResult = result;
    m_lastOpSize = DWordSize;

    setCF(result & 0xFFFFFFFF00000000ULL);
    adjustFlag32(result, dest, src);
}

void CPU::cmpFlags8(DWORD result, BYTE dest, BYTE src)
{
    mathFlags8(result, dest, src);
    setOF(((
        ((result)^(dest)) &
        ((src)^(dest))
        )>>(7))&1);
}

void CPU::cmpFlags16(DWORD result, WORD dest, WORD src)
{
    mathFlags16(result, dest, src);
    setOF(((
        ((result)^(dest)) &
        ((src)^(dest))
        )>>(15))&1);
}

void CPU::cmpFlags32(QWORD result, DWORD dest, DWORD src)
{
    mathFlags32(result, dest, src);
    setOF(((
        ((result)^(dest)) &
        ((src)^(dest))
        )>>(31))&1);
}

void CPU::setFlags(WORD flags)
{
    setCF(flags & 0x0001);
    setPF(flags & 0x0004);
    setAF(flags & 0x0010);
    setZF(flags & 0x0040);
    setSF(flags & 0x0080);
    setTF(flags & 0x0100);
    setIF(flags & 0x0200);
    setDF(flags & 0x0400);
    setOF(flags & 0x0800);
    setIOPL((flags & 0x3000) >> 12);
    setNT(flags & 0x4000);
}

WORD CPU::getFlags() const
{
    return 0x0002
        | (getCF() << 0)
        | (getPF() << 2)
        | (getAF() << 4)
        | (getZF() << 6)
        | (getSF() << 7)
        | (getTF() << 8)
        | (getIF() << 9)
        | (getDF() << 10)
        | (getOF() << 11)
        | (getIOPL() << 12)
        | (getNT() << 14);
}

void CPU::setEFlags(DWORD eflags)
{
    setFlags(eflags & 0xFFFF);

    this->RF = (eflags & 0x10000) != 0;
    this->VM = (eflags & 0x20000) != 0;
//    this->AC = (eflags & 0x40000) != 0;
//    this->VIF = (eflags & 0x80000) != 0;
//    this->VIP = (eflags & 0x100000) != 0;
//    this->ID = (eflags & 0x200000) != 0;
}

DWORD CPU::getEFlags() const
{
    DWORD eflags = getFlags()
         | (this->RF << 16)
         | (this->VM << 17)
//         | (this->AC << 18)
//         | (this->VIF << 19)
//         | (this->VIP << 20)
//         | (this->ID << 21);
         ;
    return eflags;
}
