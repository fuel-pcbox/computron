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

#include "vomctl.h"
#include "vcpu.h"
#include "debug.h"

static VomCtl the;

struct VomCtl::Private
{
    QString consoleWriteBuffer;
};

VomCtl::VomCtl()
    : IODevice("VomCtl")
    , d(new Private)
{
    m_registerIndex = 0;
    listen(0xD6, IODevice::ReadWrite);
    listen(0xD7, IODevice::ReadWrite);
}

VomCtl::~VomCtl()
{
    delete d;
    d = 0;
}

uint8_t
VomCtl::in8(WORD port)
{
    switch (port) {
    case 0xD6: // VOMCTL_REGISTER
        vlog(VM_VOMCTL, "Read register %02X", m_registerIndex);
        switch (m_registerIndex) {
        case 0x00: // Always 0
            return 0;
        case 0x01: // Get CPU type
            return VOMIT_CPU_LEVEL;
        case 0x02: // RAM size LSB
            return LSB(g_cpu->baseMemorySize() / 1024);
        case 0x03: // RAM size MSB
            return MSB(g_cpu->baseMemorySize() / 1024);
        }
        vlog(VM_VOMCTL, "Invalid register %02X read", m_registerIndex);
        break;
    case 0xD7: // VOMCTL_CONSOLE_WRITE
        vlog(VM_VOMCTL, "%s", d->consoleWriteBuffer.toLatin1().constData());
        d->consoleWriteBuffer.clear();
        break;
    }

    return 0;
}

void VomCtl::out8(WORD port, BYTE data)
{
    switch (port) {
    case 0xD6: // VOMCTL_REGISTER
        //vlog(VM_VOMCTL, "Select register %02X", data);
        m_registerIndex = data;
        break;
    case 0xD7: // VOMCTL_CONSOLE_WRITE
        d->consoleWriteBuffer += QChar::fromLatin1(data);
        break;
    }
}
