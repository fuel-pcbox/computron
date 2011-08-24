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

static const BYTE parity_table[0x100] = {
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
};

void VCpu::adjustFlag32(DWORD result, WORD src, WORD dest)
{
    setAF((((result ^ (src ^ dest)) & 0x10) >> 4) & 1);
}

void VCpu::updateFlags32(DWORD data)
{
    setPF(parity_table[data & 0xFF]);
    setSF((data & 0x80000000) != 0);
    setZF(data == 0);
}

void VCpu::updateFlags16(WORD data)
{
    setPF(parity_table[data & 0xFF]);
    setSF(data & 0x8000);
    setZF(data == 0);
}

void VCpu::updateFlags8(BYTE data)
{
    setPF(parity_table[data]);
    setSF(data & 0x80);
    setZF(data == 0);
}

void VCpu::updateFlags(WORD data, BYTE bits)
{
    if (bits == 8) {
        data &= 0xFF;
        setSF(data & 0x80);
    } else
        setSF(data & 0x8000);

    setPF(parity_table[data & 0xFF]);
    setZF(data == 0);
}

void VCpu::_STC()
{
    setCF(1);
}

void VCpu::_STD()
{
    setDF(1);
}

void VCpu::_STI()
{
    if (!getPE()) {
        setIF(1);
        return;
    }

    if (!getVM()) {
        if (getIOPL() >= getCPL()) {
            setIF(1);
        } else {
            if ((getIOPL() < getCPL()) && getCPL() == 3 && !getVIP())
                setVIF(1);
            else
                GP(0);
        }
    } else {
        if (getIOPL() == 3) {
            setIF(0);
        } else {
            if (getIOPL() < 3 && !getVIP() && getVME()) {
                setVIF(1);
            } else {
                GP(0);
            }
        }
    }
}

void VCpu::_CLI()
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
                GP(0);
        }
    } else {
        if (getIOPL() == 3) {
            setIF(0);
        } else {
            if (getIOPL() < 3 && getVME()) {
                setVIF(0);
            } else {
                GP(0);
            }
        }
    }
}

void VCpu::_CLC()
{
    setCF(0);
}

void VCpu::_CLD()
{
    setDF(0);
}

void VCpu::_CMC()
{
    setCF(!getCF());
}

void VCpu::_LAHF()
{
    regs.B.AH = getCF() | (getPF() * 4) | (getAF() * 16) | (getZF() * 64) | (getSF() * 128) | 2;
}

void VCpu::_SAHF()
{
    setCF(regs.B.AH & 0x01);
    setPF(regs.B.AH & 0x04);
    setAF(regs.B.AH & 0x10);
    setZF(regs.B.AH & 0x40);
    setSF(regs.B.AH & 0x80);
}

void VCpu::mathFlags8(WORD result, BYTE dest, BYTE src)
{
    setCF(result & 0x0100);
    setSF(result & 0x0080);
    setZF((result & 0x00FF) == 0);
    setPF(parity_table[result & 0xFF]);
    adjustFlag32(result, dest, src);
}

void VCpu::mathFlags16(DWORD result, WORD dest, WORD src)
{
    setCF(result & 0x10000);
    setSF(result & 0x8000);
    setZF((result & 0xFFFF) == 0);
    setPF(parity_table[result & 0xFF]);
    adjustFlag32(result, dest, src);
}

void VCpu::mathFlags32(QWORD result, DWORD dest, DWORD src)
{
    setCF(result & 0x100000000ULL);
    setSF(result & 0x80000000);
    setZF((result & 0xFFFFFFFF) == 0);
    setPF(parity_table[result & 0xFF]);
    adjustFlag32(result, dest, src);
}

void VCpu::cmpFlags8(DWORD result, BYTE dest, BYTE src)
{
    mathFlags8(result, dest, src);
    setOF(((
        ((src)^(dest)) &
        ((src)^(src-dest))
        )>>(7))&1);
}

void VCpu::cmpFlags16(DWORD result, WORD dest, WORD src)
{
    mathFlags16(result, dest, src);
    setOF(((
        ((src)^(dest)) &
        ((src)^(src-dest))
        )>>(15))&1);
}

void VCpu::cmpFlags32(QWORD result, DWORD dest, DWORD src)
{
    mathFlags32(result, dest, src);
    setOF(((
        ((src)^(dest)) &
        ((src)^(src-dest))
        )>>(31))&1);
}

void VCpu::setFlags(WORD flags)
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

WORD VCpu::getFlags() const
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
        | (getIOPL() << 13)
        | (getNT() << 14);
}

void VCpu::setEFlags(DWORD eflags)
{
    setFlags(eflags & 0xFFFF);

    this->RF = (eflags & 0x10000) != 0;
    this->VM = (eflags & 0x20000) != 0;
    //this->AC = (eflags & 0x40000) != 0;
    this->VIF = (eflags & 0x80000) != 0;
    this->VIP = (eflags & 0x100000) != 0;
    this->ID = (eflags & 0x200000) != 0;
}

DWORD VCpu::getEFlags() const
{
    DWORD eflags = getFlags()
         | (this->RF << 16)
         | (this->VM << 17)
//       | (this->AC << 18)
         | (this->VIF << 19)
         | (this->VIP << 20)
         | (this->ID << 21);
    return eflags;
}
