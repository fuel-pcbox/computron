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

#include "iodevice.h"
#include "debug.h"
#include "pic.h"
#include "machine.h"
#include <QList>

QSet<WORD> IODevice::s_ignorePorts;

IODevice::IODevice(const char* name, Machine& machine, int irq)
    : m_machine(machine)
    , m_name(name)
    , m_irq(irq)
{
    m_machine.registerDevice(IODevicePass(), *this);
}

IODevice::~IODevice()
{
    m_machine.unregisterDevice(IODevicePass(), *this);
}

void IODevice::listen(WORD port, ListenMask mask)
{
    if (mask & ReadOnly)
        machine().registerInputDevice(IODevicePass(), port, *this);

    if (mask & WriteOnly)
        machine().registerOutputDevice(IODevicePass(), port, *this);

    m_ports.append(port);
}

QList<WORD> IODevice::ports() const
{
    return m_ports;
}

const char* IODevice::name() const
{
    return m_name;
}

void IODevice::out8(WORD port, BYTE data)
{
    vlog(LogIO, "FIXME: IODevice[%s]::out8(%04X, %02X)", m_name, port, data);
}

BYTE IODevice::in8(WORD port)
{
    vlog(LogIO, "FIXME: IODevice[%s]::in8(%04X)", m_name, port);
    return IODevice::JunkValue;
}

void IODevice::ignorePort(WORD port)
{
    s_ignorePorts.insert(port);
}

bool IODevice::shouldIgnorePort(WORD port)
{
    return s_ignorePorts.contains(port);
}

void IODevice::raiseIRQ()
{
    ASSERT(m_irq != -1);
    ASSERT(m_irq < 256);
    PIC::raiseIRQ(machine(), m_irq);
}

void IODevice::lowerIRQ()
{
    ASSERT(m_irq != -1);
    ASSERT(m_irq < 256);
    PIC::lowerIRQ(machine(), m_irq);
}

bool IODevice::isIRQRaised() const
{
    ASSERT(m_irq != -1);
    ASSERT(m_irq < 256);
    return PIC::isIRQRaised(machine(), m_irq);
}
