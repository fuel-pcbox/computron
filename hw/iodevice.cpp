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

#include "iodevice.h"
#include "debug.h"
#include <QList>

QList<IODevice*>& IODevice::devices()
{
    static QList<IODevice*> s_devices;
    return s_devices;
}

QHash<uint16_t, IODevice*>& IODevice::readDevices()
{
    static QHash<uint16_t, IODevice*> s_readDevices;
    return s_readDevices;
}

QHash<uint16_t, IODevice*>& IODevice::writeDevices()
{
    static QHash<uint16_t, IODevice*> s_writeDevices;
    return s_writeDevices;
}

IODevice::IODevice(const char* name)
    : m_name(name)
{
    devices().append(this);
}

IODevice::~IODevice()
{
    devices().removeAll(this);
}

void IODevice::listen(uint16_t port, ListenMask mask)
{
    if (mask & ReadOnly)
        readDevices()[port] = this;

    if (mask & WriteOnly)
        writeDevices()[port] = this;

    m_ports.append(port);
}

QList<uint16_t> IODevice::ports() const
{
    return m_ports;
}

const char* IODevice::name() const
{
    return m_name;
}

void IODevice::out8(WORD port, BYTE data)
{
    vlog(VM_ALERT, "FIXME: IODevice[%s]::out8(%04X, %02X)", m_name, port, data);
}

BYTE IODevice::in8(WORD port)
{
    vlog(VM_ALERT, "FIXME: IODevice[%s]::in8(%04X)", m_name, port);
    return IODevice::JunkValue;
}
