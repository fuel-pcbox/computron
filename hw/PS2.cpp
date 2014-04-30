/*
 * Copyright (C) 2014 Andreas Kling <kling@webkit.org>
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

#include "PS2.h"
#include "vomit.h"
#include "vcpu.h"

static PS2 thePS2;

static const WORD SystemControlPortA = 0x92;

PS2::PS2()
    : IODevice("PS2")
{
    listen(SystemControlPortA, IODevice::ReadWrite);
}

PS2::~PS2()
{
}

BYTE PS2::in8(WORD port)
{
    if (port == SystemControlPortA) {
        BYTE data = g_cpu->isA20Enabled() << 1;
        vlog(LogIO, "System Control Port A read, returning %02X\n", data);
        return data;
    }
    return IODevice::in8(port);
}

void PS2::out8(WORD port, BYTE data)
{
    if (port == SystemControlPortA) {
        vlog(LogIO, "A20=%u->%u (System Control Port A)\n", g_cpu->isA20Enabled(), !!(data & 0x2));
        g_cpu->setA20Enabled(data & 0x2);
        return;
    }
    IODevice::out8(port, data);
}
