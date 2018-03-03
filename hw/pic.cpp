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
#include "pic.h"
#include "debug.h"
#include "machine.h"

// FIXME: These should not be globals.
static volatile bool s_haveRequests = false;
WORD PIC::s_pendingRequests = 0x0000;
QMutex PIC::s_mutex;
static bool s_ignoringIRQs = false;

void PIC::setIgnoreAllIRQs(bool b)
{
    s_ignoringIRQs = b;
}

void PIC::updatePendingRequests(Machine& machine)
{
    QMutexLocker locker(&s_mutex);
    s_pendingRequests = (machine.masterPIC().getIRR() & ~machine.masterPIC().getIMR() ) | ((machine.slavePIC().getIRR() & ~machine.slavePIC().getIMR()) << 8);
    s_haveRequests = s_pendingRequests != 0;
}

bool PIC::hasPendingIRQ()
{
    return s_haveRequests;
}

PIC::PIC(bool isMaster, Machine& machine)
    : IODevice("PIC", machine)
    , m_baseAddress(isMaster ? 0x20 : 0xA0)
    , m_isrBase(isMaster ? 0x08 : 0x70)
    , m_irqBase(isMaster ? 0 : 8)
{    
    listen(m_baseAddress, IODevice::ReadWrite);
    listen(m_baseAddress + 1, IODevice::ReadWrite);

    reset();
}

PIC::~PIC()
{
}

void PIC::reset()
{
    m_isr = 0x00;
    m_irr = 0x00;
    m_imr = 0x00;
    m_icw2Expected = false;
    m_icw4Expected = false;
    m_readIRR = true;
}

void PIC::out8(WORD port, BYTE data)
{
    if ((port & 0x01) == 0x00) {
        if (data == 0x20) {
            // Nonspecific IOI
            m_isr = 0x00;
            return;
        }
        if ((data & 0x18) == 0x08) {
            // OCW3
            vlog(LogPIC, "Got OCW3 %02X on port %02X", data, port);
            if (data & 0x02)
                m_readIRR = data & 0x01;
            return;
        }
        if ((data & 0x18) == 0x00) {
            // OCW2
            vlog(LogPIC, "Got OCW2 %02X on port %02X", data, port);
            return;
        }
        if (data & 0x10) {
            // ICW1
            vlog(LogPIC, "Got ICW1 %02X on port %02X", data, port);
            vlog(LogPIC, "[ICW1] ICW4 needed = %s", (data & 1) ? "yes" : "no");
            vlog(LogPIC, "[ICW1] Cascade = %s", (data & 2) ? "yes" : "no");
            vlog(LogPIC, "[ICW1] Vector size = %u", (data & 4) ? 4 : 8);
            vlog(LogPIC, "[ICW1] Level triggered = %s", (data & 8) ? "yes" : "no");
            m_imr = 0;
            m_isr = 0;
            m_irr = 0;
            m_readIRR = true;
            m_icw2Expected = true;
            m_icw4Expected = data & 0x01;
            updatePendingRequests(machine());
            return;
        }

    } else {
        if (((data & 0x07) == 0x00) && m_icw2Expected) {
            // ICW2
            vlog(LogPIC, "Got ICW2 %02X on port %02X", data, port);
            m_isrBase = data & 0xF8;
            m_icw2Expected = false;
            return;
        }

        // OCW1 - IMR write
        vlog(LogPIC, "New IRQ mask set: %02X", data);
        for (int i = 0; i < 8; ++i)
            vlog(LogPIC, " - IRQ %u: %s", m_irqBase + i, (data & (1 << i)) ? "masked" : "service");
        m_imr = data;
        updatePendingRequests(machine());
        return;
    }

    vlog(LogPIC, "Write PIC ICW on port %04X (data: %02X)", port, data);
    vlog(LogPIC, "I can't handle that request, better quit!");
    hard_exit(1);
}

BYTE PIC::in8(WORD)
{
    if (m_readIRR) {
        vlog(LogPIC, "Read IRR (%02X)", m_irr);
        return m_irr;
    } else {
        vlog(LogPIC, "Read ISR (%02X)", m_isr);
        return m_isr;
    }
}

void PIC::raise(BYTE num)
{
    m_irr |= 1 << num;
    m_isr |= 1 << num;
}

void PIC::raiseIRQ(Machine& machine, BYTE num)
{
    if (num < 8)
        machine.masterPIC().raise(num);
    else
        machine.slavePIC().raise(num - 8);

    updatePendingRequests(machine);
}

void PIC::serviceIRQ(CPU& cpu)
{
    QMutexLocker lockerGlobal(&s_mutex);
    Machine& machine = cpu.machine();

    if (s_ignoringIRQs)
        return;

    if (!s_pendingRequests)
        return;

    BYTE interrupt_to_service = 0xFF;

    for (int i = 0; i < 16; ++i)
        if (s_pendingRequests & (1 << i))
            interrupt_to_service = i;

    if (interrupt_to_service == 0xFF)
        return;

    if (interrupt_to_service < 8) {
        machine.masterPIC().m_irr &= ~(1 << interrupt_to_service);
        machine.masterPIC().m_isr |= (1 << interrupt_to_service);

        cpu.jumpToInterruptHandler(machine.masterPIC().m_isrBase | interrupt_to_service, true);
    }
    else
    {
        machine.slavePIC().m_irr &= ~(1 << (interrupt_to_service - 8));
        machine.slavePIC().m_isr |= (1 << (interrupt_to_service - 8));

        cpu.jumpToInterruptHandler(machine.slavePIC().m_isrBase | (interrupt_to_service - 8), true);
    }

    lockerGlobal.unlock();
    updatePendingRequests(machine);

    cpu.setState(CPU::Alive);
}
