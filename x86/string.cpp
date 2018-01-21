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

void VCpu::_LODSB()
{
    if (a16()) {
        regs.B.AL = readMemory8(currentSegment(), getSI());
        nextSI(1);
    } else {
        regs.B.AL = readMemory8(currentSegment(), getESI());
        nextESI(1);
    }
}

void VCpu::_LODSW()
{
    if (a16()) {
        regs.W.AX = readMemory16(currentSegment(), getSI());
        nextSI(2);
    } else {
        regs.W.AX = readMemory16(currentSegment(), getESI());
        nextESI(2);
    }
}

void VCpu::_LODSD()
{
    if (a16()) {
        regs.D.EAX = readMemory32(currentSegment(), getSI());
        nextSI(4);
    } else {
        regs.D.EAX = readMemory32(currentSegment(), getESI());
        nextESI(4);
    }
}

void VCpu::_STOSB()
{
    if (a16()) {
        writeMemory8(getES(), getDI(), getAL());
        nextDI(1);
    } else {
        writeMemory8(getES(), getEDI(), getAL());
        nextEDI(1);
    }
}

void VCpu::_STOSW()
{
    if (a16()) {
        writeMemory16(getES(), getDI(), getAX());
        nextDI(2);
    } else {
        writeMemory16(getES(), getEDI(), getAX());
        nextEDI(2);
    }
}

void VCpu::_STOSD()
{
    if (a16()) {
        writeMemory32(getES(), getDI(), getEAX());
        nextDI(4);
    } else {
        writeMemory32(getES(), getEDI(), getEAX());
        nextEDI(4);
    }
}

void VCpu::_CMPSB()
{
    BYTE src;
    BYTE dest;

    if (a16()) {
        src = readMemory8(currentSegment(), getSI());
        dest = readMemory8(getES(), getDI());
        nextSI(1);
        nextDI(1);
    } else {
        src = readMemory8(currentSegment(), getESI());
        dest = readMemory8(getES(), getEDI());
        nextESI(1);
        nextEDI(1);
    }

    cmpFlags8(src - dest, src, dest);
}

void VCpu::_CMPSW()
{
    WORD src;
    WORD dest;

    if (a16()) {
        src = readMemory16(currentSegment(), getSI());
        dest = readMemory16(getES(), getDI());
        nextSI(2);
        nextDI(2);
    } else {
        src = readMemory16(currentSegment(), getESI());
        dest = readMemory16(getES(), getEDI());
        nextESI(2);
        nextEDI(2);
    }

    cmpFlags16(src - dest, src, dest);
}

void VCpu::_CMPSD()
{
    DWORD src;
    DWORD dest;

    if (a16()) {
        src = readMemory32(currentSegment(), getSI());
        dest = readMemory32(getES(), getDI());
        nextSI(4);
        nextDI(4);
    } else {
        src = readMemory32(currentSegment(), getESI());
        dest = readMemory32(getES(), getEDI());
        nextESI(4);
        nextEDI(4);
    }

    cmpFlags32(src - dest, src, dest);
}

void VCpu::_SCASB()
{
    BYTE dest;

    if (a16()) {
        dest = readMemory8(getES(), getDI());
        nextDI(1);
    } else {
        dest = readMemory8(getES(), getEDI());
        nextEDI(1);
    }

    cmpFlags8(getAL() - dest, dest, getAL());
}

void VCpu::_SCASW()
{
    WORD dest;

    if (a16()) {
        dest = readMemory16(getES(), getDI());
        nextDI(2);
    } else {
        dest = readMemory16(getES(), getEDI());
        nextEDI(2);
    }

    cmpFlags16(getAX() - dest, dest, getAX());
}

void VCpu::_SCASD()
{
    DWORD dest;

    if (a16()) {
        dest = readMemory32(getES(), getDI());
        nextDI(4);
    } else {
        dest = readMemory32(getES(), getEDI());
        nextEDI(4);
    }

    cmpFlags16(getEAX() - dest, dest, getEAX());
}

void VCpu::_MOVSB()
{
    if (a16()) {
        BYTE tmpb = readMemory8(currentSegment(), getSI());
        writeMemory8(ES, getDI(), tmpb);
        nextSI(1);
        nextDI(1);
    } else {
        BYTE tmpb = readMemory8(currentSegment(), getESI());
        writeMemory8(ES, getEDI(), tmpb);
        nextESI(1);
        nextEDI(1);
    }
}

void VCpu::_MOVSW()
{
    if (a16()) {
        WORD tmpw = readMemory16(currentSegment(), getSI());
        writeMemory16(ES, getDI(), tmpw);
        nextSI(2);
        nextDI(2);
    } else {
        WORD tmpw = readMemory16(currentSegment(), getESI());
        writeMemory16(ES, getEDI(), tmpw);
        nextESI(2);
        nextEDI(2);
    }
}

void VCpu::_MOVSD()
{
    if (a16()) {
        DWORD tmpw = readMemory32(currentSegment(), regs.W.SI);
        writeMemory32(ES, regs.W.DI, tmpw);
        nextSI(4);
        nextDI(4);
    } else {
        DWORD tmpw = readMemory32(currentSegment(), regs.D.ESI);
        writeMemory32(ES, regs.D.EDI, tmpw);
        nextESI(4);
        nextEDI(4);
    }
}
