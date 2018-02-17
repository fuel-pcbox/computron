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

#include "cmos.h"
#include "debug.h"
#include "CPU.h"
#include <QtCore/QDate>
#include <QtCore/QTime>

CMOS::CMOS(Machine& machine)
    : IODevice("CMOS", machine)
{
    listen(0x70, IODevice::WriteOnly);
    listen(0x71, IODevice::ReadWrite);
    reset();
}

CMOS::~CMOS()
{
}

void CMOS::reset()
{
    m_registerIndex = 0;
    m_statusRegisterB = 0x00;

    // FIXME: This thing needs more work, 0x26 is just an initial value.
    m_statusRegisterA = 0x26;
}

bool CMOS::inBinaryClockMode() const
{
    return m_statusRegisterB & 0x04;
}

bool CMOS::in24HourMode() const
{
    return m_statusRegisterB & 0x02;
}

static QTime currentTimeForCMOS()
{
#ifdef CT_DETERMINISTIC
    return QTime(1, 2, 3, 4);
#endif
    return QTime::currentTime();
}

static QDate currentDateForCMOS()
{
#ifdef CT_DETERMINISTIC
    return QDate(2018, 2, 9);
#endif
    return QDate::currentDate();
}

BYTE CMOS::in8(WORD)
{
    BYTE value = 0;

    switch (m_registerIndex) {
    case 0x00: value = currentTimeForCMOS().second(); break;
    case 0x02: value = currentTimeForCMOS().minute(); break;
    case 0x04: value = currentTimeForCMOS().hour(); break;
    case 0x06: value = currentDateForCMOS().dayOfWeek(); break;
    case 0x07: value = currentDateForCMOS().day(); break;
    case 0x08: value = currentDateForCMOS().month(); break;
    case 0x09: value = currentDateForCMOS().year() % 100; break;
    case 0x0A: value = m_statusRegisterA; break;
    case 0x0B: value = m_statusRegisterB; break;
    case 0x15: value = getLSB(g_cpu->baseMemorySize() / 1024); break;
    case 0x16: value = getMSB(g_cpu->baseMemorySize() / 1024); break;
    case 0x17: value = getLSB(g_cpu->extendedMemorySize() / 1024 - 1024); break;
    case 0x18: value = getMSB(g_cpu->extendedMemorySize() / 1024 - 1024); break;
    case 0x30: value = getLSB(g_cpu->extendedMemorySize() / 1024 - 1024); break;
    case 0x31: value = getMSB(g_cpu->extendedMemorySize() / 1024 - 1024); break;
    case 0x32: value = currentDateForCMOS().year() / 100; break;
    default: vlog(LogCMOS, "WARNING: Read unsupported register %02X", m_registerIndex);
    }

    if (!inBinaryClockMode()) {
        switch (m_registerIndex) {
        case 0x00:
        case 0x02:
        case 0x04:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x09:
        case 0x32:
            BYTE bcd = (value / 10 << 4) | (value - (value / 10) * 10);;
            value = bcd;
            break;
        }
    }

#ifdef CMOS_DEBUG
    vlog(LogCMOS, "Read register %02X (%02X)", m_registerIndex, value);
#endif
    return value;
}

void CMOS::out8(WORD, BYTE data)
{
#ifdef CMOS_DEBUG
    vlog(LogCMOS, "Select register %02X", data);
#endif
    m_registerIndex = data;
}
