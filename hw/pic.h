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

#ifndef PIC_H
#define PIC_H

#include "iodevice.h"
#include <QtCore/QMutex>

class VCpu;

class PIC : public IODevice
{
public:
    PIC(WORD baseAddress, BYTE isrBase);
    ~PIC();

    void out8(WORD port, BYTE data);
    BYTE in8(WORD port);

    void raise(BYTE num);

    BYTE getIMR() const { return m_imr; }
    BYTE getIRR() const { return m_irr; }
    BYTE getISR() const { return m_isr; }

    static bool hasPendingIRQ();
    static void serviceIRQ(VCpu*);
    static void raiseIRQ(BYTE num);

private:
    WORD m_baseAddress;
    BYTE m_isrBase;

    BYTE m_isr;
    BYTE m_irr;
    BYTE m_imr;

    bool m_icw2Expected;
    bool m_readIRR;

    static WORD s_pendingRequests;
    static void updatePendingRequests();
    static QMutex s_mutex;
};

#endif
