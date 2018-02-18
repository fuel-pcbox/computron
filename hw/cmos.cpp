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
#include "machine.h"
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
    auto& cpu = machine().cpu();

    memset(m_ram, 0, sizeof(m_ram));
    m_registerIndex = 0;

    // FIXME: This thing needs more work, 0x26 is just an initial value.
    m_ram[StatusRegisterA] = 0x26;

    m_ram[StatusRegisterB] = 0x02;

    m_ram[BaseMemoryInKilobytesLSB] = getLSB(cpu.baseMemorySize() / 1024);
    m_ram[BaseMemoryInKilobytesMSB] = getMSB(cpu.baseMemorySize() / 1024);
    m_ram[ExtendedMemoryInKilobytesLSB] = getLSB(cpu.extendedMemorySize() / 1024 - 1024);
    m_ram[ExtendedMemoryInKilobytesMSB] = getMSB(cpu.extendedMemorySize() / 1024 - 1024);
    m_ram[ExtendedMemoryInKilobytesAltLSB] = getLSB(cpu.extendedMemorySize() / 1024 - 1024);
    m_ram[ExtendedMemoryInKilobytesAltMSB] = getMSB(cpu.extendedMemorySize() / 1024 - 1024);
}

bool CMOS::inBinaryClockMode() const
{
    return m_ram[StatusRegisterB] & 0x04;
}

bool CMOS::in24HourMode() const
{
    return m_ram[StatusRegisterB] & 0x02;
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

BYTE CMOS::toCurrentClockFormat(BYTE value) const
{
    if (!inBinaryClockMode())
        return value;
    return (value / 10 << 4) | (value - (value / 10) * 10);
}

void CMOS::updateClock()
{
    // FIXME: Support 12-hour clock mode for RTCHour!
    ASSERT(in24HourMode());

    auto nowTime = currentTimeForCMOS();
    auto nowDate = currentDateForCMOS();
    m_ram[RTCSecond] = toCurrentClockFormat(nowTime.second());
    m_ram[RTCMinute] = toCurrentClockFormat(nowTime.minute());
    m_ram[RTCHour] = toCurrentClockFormat(nowTime.hour());
    m_ram[RTCDayOfWeek] = toCurrentClockFormat(nowDate.dayOfWeek());
    m_ram[RTCDay] = toCurrentClockFormat(nowDate.day());
    m_ram[RTCMonth] = toCurrentClockFormat(nowDate.month());
    m_ram[RTCYear] = toCurrentClockFormat(nowDate.year() % 100);
    m_ram[RTCCentury] = toCurrentClockFormat(nowDate.year() / 100);
    m_ram[RTCCenturyPS2] = toCurrentClockFormat(nowDate.year() / 100);
}

BYTE CMOS::in8(WORD)
{
    updateClock();

    BYTE value = m_ram[m_registerIndex];

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

void CMOS::set(RegisterIndex index, BYTE data)
{
    ASSERT((size_t)index < sizeof(m_ram));
    m_ram[index] = data;
}

BYTE CMOS::get(RegisterIndex index) const
{
    ASSERT((size_t)index < sizeof(m_ram));
    return m_ram[index];
}
