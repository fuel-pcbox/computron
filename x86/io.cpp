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

#include "Common.h"
#include "CPU.h"
#include "debug.h"
#include "iodevice.h"
#include "machine.h"

void CPU::_OUT_imm8_AL(Instruction& insn)
{
    out8(insn.imm8(), getAL());
}

void CPU::_OUT_imm8_AX(Instruction& insn)
{
    out16(insn.imm8(), getAX());
}

void CPU::_OUT_imm8_EAX(Instruction& insn)
{
    out32(insn.imm8(), getEAX());
}

void CPU::_OUT_DX_AL(Instruction&)
{
    out8(getDX(), getAL());
}

void CPU::_OUT_DX_AX(Instruction&)
{
    out16(getDX(), getAX());
}

void CPU::_OUT_DX_EAX(Instruction&)
{
    out32(getDX(), getEAX());
}

void CPU::_OUTSB(Instruction&)
{
    BYTE data;

    if (a16()) {
        data = readMemory8(currentSegment(), getSI());
        nextSI(1);
    } else {
        data = readMemory8(currentSegment(), getESI());
        nextESI(1);
    }

    out8(getDX(), data);
}

void CPU::_OUTSW(Instruction&)
{
    WORD data;

    if (a16()) {
        data = readMemory16(currentSegment(), regs.W.SI);
        nextSI(2);
    } else {
        data = readMemory16(currentSegment(), regs.D.ESI);
        nextESI(2);
    }

    out16(getDX(), data);
}

void CPU::_OUTSD(Instruction&)
{
    DWORD data;

    if (a16()) {
        data = readMemory32(currentSegment(), regs.W.SI);
        nextSI(4);
    } else {
        data = readMemory32(currentSegment(), regs.D.ESI);
        nextESI(4);
    }

    out32(getDX(), data);
}

void CPU::_IN_AL_imm8(Instruction& insn)
{
    setAL(in8(insn.imm8()));
}

void CPU::_IN_AX_imm8(Instruction& insn)
{
    setAX(in16(insn.imm8()));
}

void CPU::_IN_EAX_imm8(Instruction& insn)
{
    setEAX(in32(insn.imm8()));
}

void CPU::_IN_AL_DX(Instruction&)
{
    setAL(in8(getDX()));
}

void CPU::_IN_AX_DX(Instruction&)
{
    setAX(in16(getDX()));
}

void CPU::_IN_EAX_DX(Instruction&)
{
    setEAX(in32(getDX()));
}

// Important note from IA32 manual, regarding string I/O instructions:
// "These instructions may read from the I/O port without writing to the memory location if an exception or VM exit
// occurs due to the write (e.g. #PF). If this would be problematic, for example because the I/O port read has side-
// effects, software should ensure the write to the memory location does not cause an exception or VM exit."

void CPU::_INSB(Instruction&)
{
    // FIXME: Should this really read the port without knowing that the destination memory is writable?
    BYTE data = in8(getDX());
    if (a16()) {
        writeMemory8(SegmentRegisterIndex::ES, getDI(), data);
        nextDI(1);
    } else {
        writeMemory8(SegmentRegisterIndex::ES, getEDI(), data);
        nextEDI(1);
    }
}

void CPU::_INSW(Instruction&)
{
    WORD data = in16(getDX());
    if (a16()) {
        writeMemory16(SegmentRegisterIndex::ES, getDI(), data);
        nextDI(2);
    } else {
        writeMemory16(SegmentRegisterIndex::ES, getEDI(), data);
        nextEDI(2);
    }
}

void CPU::_INSD(Instruction&)
{
    DWORD data = in32(getDX());
    if (a16()) {
        writeMemory32(SegmentRegisterIndex::ES, getDI(), data);
        nextDI(4);
    } else {
        writeMemory32(SegmentRegisterIndex::ES, getEDI(), data);
        nextEDI(4);
    }
}

void CPU::out8(WORD port, BYTE value)
{
    if (getPE() && getCPL() > getIOPL()) {
        throw GeneralProtectionFault(0, QString("I/O write attempt with CPL(%1) > IOPL(%2)").arg(getCPL()).arg(getIOPL()));
    }

    if (options.iopeek) {
        if (port != 0x00E6 && port != 0x0020 && port != 0x3D4 && port != 0x03d5 && port != 0xe2 && port != 0xe0 && port != 0x92) {
            vlog(LogIO, "CPU::out8: %02X --> %04X", value, port);
        }
    }

    if (auto* device = machine().outputDeviceForPort(port)) {
        device->out8(port, value);
        return;
    }

    if (!IODevice::shouldIgnorePort(port))
        vlog(LogAlert, "Unhandled I/O write to port %04X, data %02X", port, value);
}

void CPU::out16(WORD port, WORD value)
{
    if (getPE() && getCPL() > getIOPL()) {
        throw GeneralProtectionFault(0, QString("I/O write attempt with CPL(%1) > IOPL(%2)").arg(getCPL()).arg(getIOPL()));
    }

    if (options.iopeek) {
        if (port != 0x00E6 && port != 0x0020 && port != 0x3D4 && port != 0x03d5 && port != 0xe2 && port != 0xe0 && port != 0x92) {
            vlog(LogIO, "CPU::out16: %02X --> %04X", value, port);
        }
    }

    if (auto* device = machine().outputDeviceForPort(port)) {
        device->out16(port, value);
        return;
    }

    if (!IODevice::shouldIgnorePort(port))
        vlog(LogAlert, "Unhandled I/O write to port %04X, data %02X", port, value);
}

void CPU::out32(WORD port, DWORD value)
{
    if (getPE() && getCPL() > getIOPL()) {
        throw GeneralProtectionFault(0, QString("I/O write attempt with CPL(%1) > IOPL(%2)").arg(getCPL()).arg(getIOPL()));
    }

    if (options.iopeek) {
        if (port != 0x00E6 && port != 0x0020 && port != 0x3D4 && port != 0x03d5 && port != 0xe2 && port != 0xe0 && port != 0x92) {
            vlog(LogIO, "CPU::out32: %02X --> %04X", value, port);
        }
    }

    if (auto* device = machine().outputDeviceForPort(port)) {
        device->out32(port, value);
        return;
    }

    if (!IODevice::shouldIgnorePort(port))
        vlog(LogAlert, "Unhandled I/O write to port %04X, data %02X", port, value);
}

BYTE CPU::in8(WORD port)
{
    if (getPE() && getCPL() > getIOPL()) {
        throw GeneralProtectionFault(0, QString("I/O read attempt with CPL(%1) > IOPL(%2)").arg(getCPL()).arg(getIOPL()));
    }

    BYTE value;
    if (auto* device = machine().inputDeviceForPort(port)) {
        value = device->in8(port);
    } else {
        if (!IODevice::shouldIgnorePort(port))
            vlog(LogAlert, "Unhandled I/O read from port %04X", port);
        value = IODevice::JunkValue;
    }

    if (options.iopeek) {
        if (port != 0x00E6 && port != 0x0020 && port != 0x3D4 && port != 0x03D5 && port != 0x3DA && port != 0x92) {
            vlog(LogIO, "CPU::in8: %04X = %02X", port, value);
        }
    }
    return value;
}

WORD CPU::in16(WORD port)
{
    if (getPE() && getCPL() > getIOPL()) {
        throw GeneralProtectionFault(0, QString("I/O read attempt with CPL(%1) > IOPL(%2)").arg(getCPL()).arg(getIOPL()));
    }

    WORD value;
    if (auto* device = machine().inputDeviceForPort(port)) {
        value = device->in16(port);
    } else {
        if (!IODevice::shouldIgnorePort(port))
            vlog(LogAlert, "Unhandled 16-bit I/O read from port %04X", port);
        value = IODevice::JunkValue;
    }

    if (options.iopeek) {
        vlog(LogIO, "CPU::in16: %04X = %02X", port, value);
    }
    return value;
}

DWORD CPU::in32(WORD port)
{
    if (getPE() && getCPL() > getIOPL()) {
        throw GeneralProtectionFault(0, QString("I/O read attempt with CPL(%1) > IOPL(%2)").arg(getCPL()).arg(getIOPL()));
    }

    DWORD value;
    if (auto* device = machine().inputDeviceForPort(port)) {
        value = device->in32(port);
    } else {
        if (!IODevice::shouldIgnorePort(port))
            vlog(LogAlert, "Unhandled 32-bit I/O read from port %04X", port);
        value = IODevice::JunkValue;
    }

    if (options.iopeek) {
        vlog(LogIO, "CPU::in32: %04X = %02X", port, value);
    }
    return value;
}
