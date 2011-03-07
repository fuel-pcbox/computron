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

#include "vomit.h"
#include "debug.h"
#include "iodevice.h"

static QSet<WORD> s_ignorePorts;

void _OUT_imm8_AL(VCpu* cpu)
{
    cpu->out(cpu->fetchOpcodeByte(), cpu->regs.B.AL);
}

void _OUT_imm8_AX(VCpu* cpu)
{
    WORD port = cpu->fetchOpcodeByte();
    cpu->out(port, cpu->regs.B.AL);
    cpu->out(port + 1, cpu->regs.B.AH);
}

void _OUT_imm8_EAX(VCpu* cpu)
{
    WORD port = cpu->fetchOpcodeByte();
    cpu->out(port, cpu->regs.B.AL);
    cpu->out(port + 1, cpu->regs.B.AH);
    cpu->out(port + 2, LSB(cpu->regs.W.__EAX_high_word));
    cpu->out(port + 3, MSB(cpu->regs.W.__EAX_high_word));
}

void _OUT_DX_AL(VCpu* cpu)
{
    cpu->out(cpu->getDX(), cpu->regs.B.AL);
}

void _OUT_DX_AX(VCpu* cpu)
{
    cpu->out(cpu->getDX(), cpu->regs.B.AL);
    cpu->out(cpu->getDX() + 1, cpu->regs.B.AH);
}

void _OUT_DX_EAX(VCpu* cpu)
{
    cpu->out(cpu->getDX(), cpu->regs.B.AL);
    cpu->out(cpu->getDX() + 1, cpu->regs.B.AH);
    cpu->out(cpu->getDX() + 2, LSB(cpu->regs.W.__EAX_high_word));
    cpu->out(cpu->getDX() + 3, MSB(cpu->regs.W.__EAX_high_word));
}

void _OUTSB(VCpu* cpu)
{
    BYTE data;

    if (cpu->a16()) {
        data = cpu->readMemory8(cpu->currentSegment(), cpu->getSI());
        cpu->nextSI(1);
    } else {
        data = cpu->readMemory8(cpu->currentSegment(), cpu->getESI());
        cpu->nextESI(1);
    }

    cpu->out(cpu->getDX(), data);
}

void _OUTSW(VCpu* cpu)
{
    BYTE lsb;
    BYTE msb;

    if (cpu->a16()) {
        lsb = cpu->readMemory8(cpu->currentSegment(), cpu->regs.W.SI);
        msb = cpu->readMemory8(cpu->currentSegment(), cpu->regs.W.SI + 1);
        cpu->nextSI(2);
    } else {
        lsb = cpu->readMemory8(cpu->currentSegment(), cpu->getSI());
        msb = cpu->readMemory8(cpu->currentSegment(), cpu->getESI() + 1);
        cpu->nextESI(2);
    }

    cpu->out(cpu->getDX(), lsb);
    cpu->out(cpu->getDX() + 1, msb);
}

void _IN_AL_imm8(VCpu* cpu)
{
    cpu->regs.B.AL = cpu->in(cpu->fetchOpcodeByte());
}

void _IN_AX_imm8(VCpu* cpu)
{
    WORD port = cpu->fetchOpcodeByte();
    cpu->regs.B.AL = cpu->in(port);
    cpu->regs.B.AH = cpu->in(port + 1);
}

void _IN_EAX_imm8(VCpu* cpu)
{
    WORD port = cpu->fetchOpcodeByte();
    cpu->regs.B.AL = cpu->in(port);
    cpu->regs.B.AH = cpu->in(port + 1);
    cpu->regs.W.__EAX_high_word = MAKEWORD(cpu->in(port + 2), cpu->in(port + 3));
}

void _IN_AL_DX(VCpu* cpu)
{
    cpu->regs.B.AL = cpu->in(cpu->getDX());
}

void _IN_AX_DX(VCpu* cpu)
{
    cpu->regs.B.AL = cpu->in(cpu->getDX());
    cpu->regs.B.AH = cpu->in(cpu->getDX() + 1);
}

void _IN_EAX_DX(VCpu* cpu)
{
    cpu->regs.B.AL = cpu->in(cpu->getDX());
    cpu->regs.B.AH = cpu->in(cpu->getDX() + 1);
    cpu->regs.W.__EAX_high_word = MAKEWORD(cpu->in(cpu->getDX() + 2), cpu->in(cpu->getDX() + 3));
}

void VCpu::out(WORD port, BYTE value)
{
#ifdef VOMIT_DEBUG
    if (options.iopeek)
        vlog(VM_IOMSG, "[%04X:%08X] VCpu::out: %02X --> %04X", getBaseCS(), getBaseEIP(), value, port);
#endif

    if (IODevice::writeDevices().contains(port)) {
        IODevice::writeDevices()[port]->out8(port, value);
        return;
    }

    if (!s_ignorePorts.contains(port))
        vlog(VM_ALERT, "Unhandled I/O write to port %04X, data %02X", port, value);
}

BYTE VCpu::in(WORD port)
{
#ifdef VOMIT_DEBUG
    if (options.iopeek)
        vlog(VM_IOMSG, "[%04X:%08X] VCpu::in: %04X", getBaseCS(), getBaseEIP(), port);
#endif

    if (IODevice::readDevices().contains(port))
        return IODevice::readDevices()[port]->in8(port);

    if (!s_ignorePorts.contains(port))
        vlog(VM_ALERT, "Unhandled I/O read from port %04X", port);

    return IODevice::JunkValue;
}

void vomit_ignore_io_port(WORD port)
{
    s_ignorePorts.insert(port);
}
