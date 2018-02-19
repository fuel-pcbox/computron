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
#include "floppy.h"
#include "fdc.h"
#include "pic.h"
#include "debug.h"

#define FDC_DEBUG

#define DATA_REGISTER_READY 0x80
#define DATA_FROM_FDC_TO_CPU 0x40
#define DATA_FROM_CPU_TO_FDC 0x00

struct FDCDrive
{
    bool motor;
    BYTE cylinder;
    BYTE head;

    FDCDrive()
        : motor(false)
        , cylinder(0)
        , head(0)
    { }
};

struct FDC::Private
{
    FDCDrive drive[2];
    BYTE driveIndex;
    bool enabled;
    bool usingDMA;
    BYTE dataDirection;
    BYTE currentStatusRegister;
    BYTE statusRegister[4];
    BYTE command[8];
    BYTE commandSize;
    BYTE commandIndex;
    QList<BYTE> commandResult;

    FDCDrive& currentDrive() { return drive[driveIndex]; }
};

FDC::FDC(Machine& machine)
    : IODevice("FDC", machine, 6)
    , d(make<Private>())
{
    listen(0x3F0, IODevice::ReadOnly);
    listen(0x3F1, IODevice::ReadOnly);
    listen(0x3F2, IODevice::WriteOnly);
    listen(0x3F4, IODevice::ReadOnly);
    listen(0x3F5, IODevice::ReadWrite);

    reset();
}

FDC::~FDC()
{
}

void FDC::reset()
{
    d->driveIndex = 0;
    d->enabled = false;
    d->usingDMA = false;
    d->dataDirection = DATA_FROM_CPU_TO_FDC;
    d->currentStatusRegister = 0;

    d->commandIndex = 0;
    d->commandSize = 0;
    d->command[0] = 0;

    d->statusRegister[0] = 0;
    d->statusRegister[1] = 0;
    d->statusRegister[2] = 0;
    d->statusRegister[3] = 0;
}

BYTE FDC::in8(WORD port)
{
    BYTE data = 0;
    switch (port) {
    case 0x3F0: {
        if (drv_status[1] != 0) {
            /* Second drive installed */
            data |= 0x40;
        }
        vlog(LogFDC, "Read status register A: %02X", data);
        break;
    }
    case 0x3F1:
        vlog(LogFDC, "Read status register B: (FIXME)");
        // FIXME: What should this register contain?
        break;

    case 0x3F4: {
        // 0x80 - MRQ  - main request (1: data register ready, 0: data register not ready)
        // 0x40 - DIO  - data input/output (1: controller ? cpu, 0: cpu ? controller)
        // 0x20 - NDMA - non-DMA mode (1: controller not in DMA mode, 0: controller in DMA mode)
        // 0x10 - BUSY - device busy (1: busy, 0: ready)
        // 0x08 - ACTD ..
        // 0x04 - ACTC ..
        // 0x02 - ACTB ..
        // 0x01 - ACTA - drive X in positioning mode

        data |= 0x80; // MRQ = 1
        data |= d->dataDirection;

        if (!d->usingDMA)
            data |= 0x20;

        vlog(LogFDC, "Read main status register: %02X (direction: %s)", data, (d->dataDirection == DATA_FROM_CPU_TO_FDC) ? "to FDC" : "from FDC");
        break;
    }

    case 0x3F5: {
        if (d->commandResult.isEmpty()) {
            vlog(LogFDC, "Read from empty command result register");
            return IODevice::JunkValue;
        }

        data = d->commandResult.takeFirst();
        vlog(LogFDC, "Read command result byte %02X", data);

        break;
    }

    default:
        ASSERT_NOT_REACHED();
        IODevice::in8(port);
    }
#ifdef FDC_DEBUG
    vlog(LogFDC, " in8 %03x = %02x", port, data);
#endif
    return data;
}

void FDC::out8(WORD port, BYTE data)
{
#ifdef FDC_DEBUG
    vlog(LogFDC, "out8 %03x, %02x", port, data);
#endif
    switch (port) {
    case 0x3F2: {
        bool old_fdc_enabled = d->enabled;

        vlog(LogFDC, "Writing to FDC digital output, data: %02X", data);

        d->driveIndex = data & 3;
        d->enabled = (data & 0x04) != 0;
        d->usingDMA = (data & 0x08) != 0;

        d->drive[0].motor = (data & 0x10) != 0;
        d->drive[1].motor = (data & 0x20) != 0;

        vlog(LogFDC, "  Current drive: %u", d->driveIndex);
        vlog(LogFDC, "  FDC enabled:   %s", d->enabled ? "yes" : "no");
        vlog(LogFDC, "  DMA+I/O mode:  %s", d->usingDMA ? "yes" : "no");

        vlog(LogFDC, "  Motors:        %u %u", d->drive[0].motor, d->drive[1].motor);

        //if (!d->currentDrive().motor)
        //    vlog(LogFDC, "Invalid state: Current drive (%u) has motor off.", d->driveIndex);

        if (d->enabled != old_fdc_enabled)
            generateFDCInterrupt();

        break;
    }

    case 0x3F5: {
        vlog(LogFDC, "Command: %02X", data);

        if (d->commandIndex == 0) {
            // Determine the command length
            switch (data) {
            case 0x08:
                d->commandSize = 0;
                break;
            }
        }

        d->command[d->commandIndex++] = data;

        if (d->commandIndex >= d->commandSize) {
            executeCommand();
            d->commandIndex = 0;
        }
        break;
    }

    default:
        IODevice::out8(port, data);
    }
}

void FDC::executeCommand()
{
    vlog(LogFDC, "Executing command %02X", d->command[0]);

    switch (d->command[0]) {
    case 0x08: // Sense Interrupt Status
        vlog(LogFDC, "Sense interrupt");
        d->dataDirection = DATA_FROM_FDC_TO_CPU;
        d->commandResult.clear();
        d->commandResult.append(d->statusRegister[0]);
        d->commandResult.append(d->currentDrive().cylinder);
        break;
    default:
        vlog(LogFDC, "Unknown command! %02X", d->command[0]);
    }
}

void FDC::generateFDCInterrupt()
{
    d->statusRegister[0] = d->driveIndex;
    d->statusRegister[0] |= d->currentDrive().head * 0x02;
    d->statusRegister[0] |= 0x20;
    raiseIRQ();
}
