// Computron x86 PC Emulator
// Copyright (C) 2003-2018 Andreas Kling <awesomekling@gmail.com>
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY ANDREAS KLING ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANDREAS KLING OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "CPU.h"
#include "pic.h"

template<typename F>
void CPU::doOnceOrRepeatedly(Instruction& insn, bool careAboutZF, F func)
{
    if (!insn.hasRepPrefix()) {
        func();
        return;
    }
    while (readRegisterForAddressSize(RegisterCX)) {
        if (getIF() && PIC::hasPendingIRQ() && !PIC::isIgnoringAllIRQs()) {
            throw HardwareInterruptDuringREP();
        }
        func();
        ++m_cycle;
        decrementCXForAddressSize();
        if (careAboutZF) {
            if (insn.repPrefix() == Prefix::REPZ && !getZF())
                break;
            if (insn.repPrefix() == Prefix::REPNZ && getZF())
                break;
        }
    }
}

template<typename T>
void CPU::doLODS(Instruction& insn)
{
    doOnceOrRepeatedly(insn, false, [this] () {
        writeRegister<T>(RegisterAL, readMemory<T>(currentSegment(), readRegisterForAddressSize(RegisterSI)));
        stepRegisterForAddressSize(RegisterSI, sizeof(T));
    });
}

void CPU::_LODSB(Instruction& insn)
{
    doLODS<BYTE>(insn);
}

void CPU::_LODSW(Instruction& insn)
{
    doLODS<WORD>(insn);
}

void CPU::_LODSD(Instruction& insn)
{
    doLODS<DWORD>(insn);
}

template<typename T>
void CPU::doSTOS(Instruction& insn)
{
    doOnceOrRepeatedly(insn, false, [this] () {
        writeMemory<T>(SegmentRegisterIndex::ES, readRegisterForAddressSize(RegisterDI), readRegister<T>(RegisterAL));
        stepRegisterForAddressSize(RegisterDI, sizeof(T));
    });
}

void CPU::_STOSB(Instruction& insn)
{
    doSTOS<BYTE>(insn);
}

void CPU::_STOSW(Instruction& insn)
{
    doSTOS<WORD>(insn);
}

void CPU::_STOSD(Instruction& insn)
{
    doSTOS<DWORD>(insn);
}

template<typename T, typename U>
void CPU::doCMPS(Instruction& insn)
{
    static_assert(sizeof(U) == sizeof(T) * 2, "U should be a super-sized T");
    doOnceOrRepeatedly(insn, true, [this] () {
        U src = readMemory<T>(currentSegment(), readRegisterForAddressSize(RegisterSI));
        U dest = readMemory<T>(SegmentRegisterIndex::ES, readRegisterForAddressSize(RegisterDI));
        stepRegisterForAddressSize(RegisterSI, sizeof(T));
        stepRegisterForAddressSize(RegisterDI, sizeof(T));
        cmpFlags<T>(src - dest, src, dest);
    });
}

void CPU::_CMPSB(Instruction& insn)
{
    doCMPS<BYTE, WORD>(insn);
}

void CPU::_CMPSW(Instruction& insn)
{
    doCMPS<WORD, DWORD>(insn);
}

void CPU::_CMPSD(Instruction& insn)
{
    doCMPS<DWORD, QWORD>(insn);
}

template<typename T, typename U>
void CPU::doSCAS(Instruction& insn)
{
    static_assert(sizeof(U) == sizeof(T) * 2, "U should be a super-sized T");
    doOnceOrRepeatedly(insn, true, [this] () {
        U dest = readMemory<T>(SegmentRegisterIndex::ES, readRegisterForAddressSize(RegisterDI));
        stepRegisterForAddressSize(RegisterDI, sizeof(T));
        cmpFlags<T>(readRegister<T>(RegisterAL) - dest, readRegister<T>(RegisterAL), dest);
    });
}

void CPU::_SCASB(Instruction& insn)
{
    doSCAS<BYTE, WORD>(insn);
}

void CPU::_SCASW(Instruction& insn)
{
    doSCAS<WORD, DWORD>(insn);
}

void CPU::_SCASD(Instruction& insn)
{
    doSCAS<DWORD, QWORD>(insn);
}

template<typename T>
void CPU::doMOVS(Instruction& insn)
{
    doOnceOrRepeatedly(insn, false, [this] () {
        T tmp = readMemory<T>(currentSegment(), readRegisterForAddressSize(RegisterSI));
        writeMemory<T>(SegmentRegisterIndex::ES, readRegisterForAddressSize(RegisterDI), tmp);
        stepRegisterForAddressSize(RegisterSI, sizeof(T));
        stepRegisterForAddressSize(RegisterDI, sizeof(T));
    });
}

void CPU::_MOVSB(Instruction& insn)
{
    doMOVS<BYTE>(insn);
}

void CPU::_MOVSW(Instruction& insn)
{
    doMOVS<WORD>(insn);
}

void CPU::_MOVSD(Instruction& insn)
{
    doMOVS<DWORD>(insn);
}

template<typename T>
void CPU::doOUTS(Instruction& insn)
{
    doOnceOrRepeatedly(insn, false, [this] () {
        T data = readMemory<T>(currentSegment(), readRegisterForAddressSize(RegisterSI));
        out<T>(getDX(), data);
        stepRegisterForAddressSize(RegisterSI, sizeof(T));
    });
}

void CPU::_OUTSB(Instruction& insn)
{
    doOUTS<BYTE>(insn);
}

void CPU::_OUTSW(Instruction& insn)
{
    doOUTS<WORD>(insn);
}

void CPU::_OUTSD(Instruction& insn)
{
    doOUTS<DWORD>(insn);
}

template<typename T>
void CPU::doINS(Instruction& insn)
{
    doOnceOrRepeatedly(insn, false, [this] () {
        // FIXME: Should this really read the port without knowing that the destination memory is writable?
        T data = in<T>(getDX());
        writeMemory<T>(SegmentRegisterIndex::ES, readRegisterForAddressSize(RegisterDI), data);
        stepRegisterForAddressSize(RegisterDI, sizeof(T));
    });
}

void CPU::_INSB(Instruction& insn)
{
    doINS<BYTE>(insn);
}

void CPU::_INSW(Instruction& insn)
{
    doINS<WORD>(insn);
}

void CPU::_INSD(Instruction& insn)
{
    doINS<DWORD>(insn);
}
