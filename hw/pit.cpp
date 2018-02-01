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

#include "vomit.h"
#include "debug.h"
#include "pic.h"
#include "pit.h"
#include <QtCore/QThread>

enum DecrementMode { DecrementBinary = 0, DecrementBCD = 1 };

struct CounterInfo {
    WORD reload;
    WORD value;
    BYTE mode;
    DecrementMode decrementMode;
};

struct TimerInfo {
    CounterInfo counter[3];
    BYTE command;
    bool gotLSB;
};

struct PIT::Private
{
    TimerInfo timer[2];
    int frequency;
    int timerId;
};

PIT::PIT(Machine& machine)
    : QObject(nullptr)
    , IODevice("PIT", machine)
    , d(new Private)
{
    listen(0x40, IODevice::ReadWrite);
    listen(0x42, IODevice::WriteOnly);
    listen(0x43, IODevice::WriteOnly);

    d->frequency = 0;
    d->timerId = -1;

    d->timer[0].counter[0].reload = 0xFFFF;
    d->timer[0].counter[0].value = 0xFFFF;
}

PIT::~PIT()
{
    delete d;
    d = 0;
}

void PIT::reconfigureTimer()
{
    if (QThread::currentThread() != thread()) {
        QMetaObject::invokeMethod(this, "reconfigureTimer", Qt::QueuedConnection);
        return;
    }

    if (d->timerId >= 0)
        killTimer(d->timerId);

    const qreal baseFrequency = 1193181.666667;
    qreal frequency = baseFrequency / float(d->timer[0].counter[0].reload);
    int timerInterval = 1000 / frequency;

    vlog(LogTimer, "Started timer for frequency %f at interval %d", frequency, timerInterval);
    d->timerId = startTimer(timerInterval);
}

void PIT::boot()
{
    // FIXME: This should be done by the BIOS instead.
    reconfigureTimer();
}

void PIT::timerEvent(QTimerEvent*)
{
    raiseIRQ();
}

BYTE PIT::in8(WORD port)
{
    switch (port) {
    case 0x40:
    case 0x41:
    case 0x42:
        return d->timer[0].counter[port - 0x40].value;
    }

    vlog(LogTimer, "Unhandled read from port 0x%02X", port);
    return IODevice::JunkValue;
}

void PIT::out8(WORD port, BYTE data)
{
    switch (port) {
    case 0x40:
        if (d->timer[0].command == 3) {
            if (d->timer[0].gotLSB) {
                d->timer[0].counter[0].reload = vomit_MAKEWORD(vomit_MSB(d->timer[0].counter[0].reload), data);
                d->timer[0].gotLSB = false;
                reconfigureTimer();
            } else {
                d->timer[0].counter[0].reload = vomit_MAKEWORD(data, vomit_LSB(d->timer[0].counter[0].reload));
                d->timer[0].gotLSB = true;
            }
        }
        vlog(LogTimer, "Unhandled command: 0x%02X", d->timer[0].command);
        return;
    case 0x42:
        // PC Speaker data.
        return;
    case 0x43:
        modeControl(0, data);
        return;
    default:
        break;
    }

    vlog(LogTimer, "Unhandled write to port 0x%02X <- 0x%02X", port, data);
}

void PIT::modeControl(int timerIndex, BYTE data)
{
    VM_ASSERT(timerIndex == 0 || timerIndex == 1);
    TimerInfo& timer = d->timer[timerIndex];

    BYTE counterIndex = (data >> 6);

    if (counterIndex > 2) {
        vlog(LogTimer, "Invalid counter index %d specified.", counterIndex);
        return;
    }

    timer.command = (data >> 4) & 3;

    VM_ASSERT(counterIndex <= 2);
    CounterInfo& counter = timer.counter[counterIndex];

    counter.decrementMode = static_cast<DecrementMode>(data & 1);
    counter.mode = (data >> 1) & 7;
    timer.gotLSB = false;

#if 1
    vlog(LogTimer, "Setting mode for counter %d", counterIndex);
    vlog(LogTimer, " - Decrement %s", (counter.decrementMode == DecrementBCD) ? "BCD" : "binary");
    vlog(LogTimer, " - Mode %d", counter.mode);
    vlog(LogTimer, " - Command %02X", timer.command);
#endif

    if (counterIndex == 0)
        reconfigureTimer();
}

void PIT::raiseIRQ()
{
    PIC::raiseIRQ(machine(), 0);
}
