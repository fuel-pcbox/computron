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

template<typename T>
void CPU::doLODS()
{
    writeRegister<T>(RegisterAL, readMemory<T>(currentSegment(), readRegisterForAddressSize(RegisterSI)));
    stepRegisterForAddressSize(RegisterSI, sizeof(T));
}

void CPU::_LODSB(Instruction&)
{
    doLODS<BYTE>();
}

void CPU::_LODSW(Instruction&)
{
    doLODS<WORD>();
}

void CPU::_LODSD(Instruction&)
{
    doLODS<DWORD>();
}

template<typename T>
void CPU::doSTOS()
{
    writeMemory<T>(SegmentRegisterIndex::ES, readRegisterForAddressSize(RegisterDI), readRegister<T>(RegisterAL));
    stepRegisterForAddressSize(RegisterDI, sizeof(T));
}

void CPU::_STOSB(Instruction&)
{
    doSTOS<BYTE>();
}

void CPU::_STOSW(Instruction&)
{
    doSTOS<WORD>();
}

void CPU::_STOSD(Instruction&)
{
    doSTOS<DWORD>();
}

template<typename T, typename U>
void CPU::doCMPS()
{
    static_assert(sizeof(U) == sizeof(T) * 2);
    U src = readMemory<T>(currentSegment(), readRegisterForAddressSize(RegisterSI));
    U dest = readMemory<T>(SegmentRegisterIndex::ES, readRegisterForAddressSize(RegisterDI));
    stepRegisterForAddressSize(RegisterSI, sizeof(T));
    stepRegisterForAddressSize(RegisterDI, sizeof(T));
    cmpFlags<T>(src - dest, src, dest);
}

void CPU::_CMPSB(Instruction&)
{
    doCMPS<BYTE, WORD>();
}

void CPU::_CMPSW(Instruction&)
{
    doCMPS<WORD, DWORD>();
}

void CPU::_CMPSD(Instruction&)
{
    doCMPS<DWORD, QWORD>();
}

template<typename T, typename U>
void CPU::doSCAS()
{
    U dest = readMemory<T>(SegmentRegisterIndex::ES, readRegisterForAddressSize(RegisterDI));
    stepRegisterForAddressSize(RegisterDI, sizeof(T));
    cmpFlags<T>(readRegister<T>(RegisterAL) - dest, readRegister<T>(RegisterAL), dest);
}

void CPU::_SCASB(Instruction&)
{
    doSCAS<BYTE, WORD>();
}

void CPU::_SCASW(Instruction&)
{
    doSCAS<WORD, DWORD>();
}

void CPU::_SCASD(Instruction&)
{
    doSCAS<DWORD, QWORD>();
}

template<typename T>
void CPU::doMOVS()
{
    T tmp = readMemory<T>(currentSegment(), readRegisterForAddressSize(RegisterSI));
    writeMemory<T>(SegmentRegisterIndex::ES, readRegisterForAddressSize(RegisterDI), tmp);
    stepRegisterForAddressSize(RegisterSI, sizeof(T));
    stepRegisterForAddressSize(RegisterDI, sizeof(T));
}

void CPU::_MOVSB(Instruction&)
{
    doMOVS<BYTE>();
}

void CPU::_MOVSW(Instruction&)
{
    doMOVS<WORD>();
}

void CPU::_MOVSD(Instruction&)
{
    doMOVS<DWORD>();
}
