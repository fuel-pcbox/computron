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
#include "vcpu.h"
#include "pic.h"
#include "debug.h"

static PIC theMaster(0x20, 0x08);
static PIC theSlave(0xA0, 0x70);

WORD PIC::s_pendingRequests = 0x0000;
QMutex PIC::s_mutex;

void PIC::updatePendingRequests()
{
    QMutexLocker locker(&s_mutex);
    s_pendingRequests = (theMaster.getIRR() & ~theMaster.getIMR() ) | ((theSlave.getIRR() & ~theSlave.getIMR()) << 8);
}

bool PIC::hasPendingIRQ()
{
    QMutexLocker locker(&s_mutex);
    return s_pendingRequests != 0x0000;
}

PIC::PIC(WORD baseAddress, BYTE isrBase)
    : IODevice("PIC")
    , m_baseAddress(baseAddress)
    , m_isrBase(isrBase)
    , m_isr(0x00)
    , m_irr(0x00)
    , m_imr(0x00)
    , m_icw2Expected(false)
    , m_readIRR(true)
{
    listen(m_baseAddress, IODevice::ReadWrite);
    listen(m_baseAddress + 1, IODevice::ReadWrite);
}

PIC::~PIC()
{
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
            vlog(VM_PICMSG, "Got OCW3 %02X on port %02X", data, port);
            if (data & 0x02)
                m_readIRR = data & 0x01;
            return;
        }
        if ((data & 0x18) == 0x00) {
            // OCW2
            vlog(VM_PICMSG, "Got OCW2 %02X on port %02X", data, port);
            return;
        }
        if (data & 0x10) {
            // ICW1
            vlog(VM_PICMSG, "Got ICW1 %02X on port %02X", data, port);
            // I'm not sure we'll ever see an ICW4...
            // m_icw4Needed = data & 0x01;
            vlog(VM_PICMSG, "[ICW1] Cascade = %s", (data & 2) ? "yes" : "no");
            vlog(VM_PICMSG, "[ICW1] Vector size = %u", (data & 4) ? 4 : 8);
            vlog(VM_PICMSG, "[ICW1] Level triggered = %s", (data & 8) ? "yes" : "no");
            m_imr = 0;
            m_isr = 0;
            m_irr = 0;
            m_readIRR = true;
            m_icw2Expected = true;
            updatePendingRequests();
            return;
        }

    } else {
        if (((data & 0x07) == 0x00) && m_icw2Expected) {
            // ICW2
            vlog(VM_PICMSG, "Got ICW2 %02X on port %02X", data, port);
            m_isrBase = data & 0xF8;
            m_icw2Expected = false;
            return;
        }

        // OCW1 - IMR write
        vlog(VM_PICMSG, "New IRQ mask set: %02X", data);
        for (int i = 0; i < 8; ++i)
            vlog(VM_PICMSG, " - IRQ %u: %s", i, (data & (1 << i)) ? "masked" : "service");
        m_imr = data;
        updatePendingRequests();
        return;
    }

    vlog(VM_PICMSG, "Write PIC ICW on port %04X (data: %02X)", port, data);
    vlog(VM_PICMSG, "I can't handle that request, better quit!");
    vm_exit(1);
}

BYTE PIC::in8(WORD)
{
    if (m_readIRR) {
        vlog(VM_PICMSG, "Read IRR (%02X)", m_irr);
        return m_irr;
    } else {
        vlog(VM_PICMSG, "Read ISR (%02X)", m_isr);
        return m_isr;
    }
}

void PIC::raise(BYTE num)
{
    m_irr |= 1 << num;
    m_isr |= 1 << num;
}

void PIC::raiseIRQ(BYTE num)
{
    if (num < 8)
        theMaster.raise(num);
    else
        theSlave.raise(num - 8);

    updatePendingRequests();
}

void PIC::serviceIRQ(VCpu* cpu)
{
    QMutexLocker lockerGlobal(&s_mutex);
    QMutexLocker lockerMaster(&theMaster.m_mutex);
    QMutexLocker lockerSlave(&theSlave.m_mutex);

    if (!s_pendingRequests)
        return;

    BYTE interrupt_to_service = 0xFF;

    for (int i = 0; i < 16; ++i)
        if (s_pendingRequests & (1 << i))
            interrupt_to_service = i;

    if (interrupt_to_service == 0xFF)
        return;

    if (interrupt_to_service < 8) {
        theMaster.m_irr &= ~(1 << interrupt_to_service);
        theMaster.m_isr |= (1 << interrupt_to_service);

        cpu->jumpToInterruptHandler(theMaster.m_isrBase | interrupt_to_service);
    }
    else
    {
        theSlave.m_irr &= ~(1 << (interrupt_to_service - 8));
        theSlave.m_isr |= (1 << (interrupt_to_service - 8));

        cpu->jumpToInterruptHandler(theSlave.m_isrBase | interrupt_to_service);
    }

    lockerGlobal.unlock();
    updatePendingRequests();

    cpu->setState(VCpu::Alive);
}
