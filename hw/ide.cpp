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
#include "debug.h"
#include "ide.h"

static IDE theIDE;

struct IDEController
{
    WORD cylinderIndex;
    BYTE sectorIndex;
    BYTE headIndex;
    BYTE sectorCount;
    BYTE error;

    IDEController()
        : cylinderIndex(0)
        , sectorIndex(0)
        , headIndex(0)
        , sectorCount(0)
        , error(0)
    { }

private:
    IDEController(const IDEController&);
};

static const int gNumControllers = 2;

struct IDE::Private
{
    IDEController controller[gNumControllers];
};

IDE::IDE()
    : IODevice("IDE")
    , d(new Private)
{
    listen(0x171, IODevice::ReadOnly);
    listen(0x172, IODevice::ReadWrite);
    listen(0x173, IODevice::ReadWrite);
    listen(0x174, IODevice::ReadWrite);
    listen(0x175, IODevice::ReadWrite);
    listen(0x176, IODevice::ReadWrite);
    listen(0x177, IODevice::ReadWrite);
    listen(0x1F1, IODevice::ReadOnly);
    listen(0x1F2, IODevice::ReadWrite);
    listen(0x1F3, IODevice::ReadWrite);
    listen(0x1F4, IODevice::ReadWrite);
    listen(0x1F5, IODevice::ReadWrite);
    listen(0x1F6, IODevice::ReadWrite);
    listen(0x1F7, IODevice::ReadWrite);
}

IDE::~IDE()
{
    delete d;
    d = 0;
}

IDE* IDE::the()
{
    return &theIDE;
}

void IDE::out8(WORD port, BYTE data)
{
    const int controllerIndex = (((port) & 0x1F0) == 0x170);
    IDEController& controller = d->controller[controllerIndex];

    switch (port & 0xF) {
    case 0x2:
        vlog(LogIDE, "Controller %d sector count set: %u", controllerIndex, data);
        controller.sectorCount = data;
        break;
    case 0x3:
        vlog(LogIDE, "Controller %d sector index set: %u", controllerIndex, data);
        controller.sectorIndex = data;
        break;
    case 0x4:
        vlog(LogIDE, "Controller %d cylinder LSB set: %u", controllerIndex, data);
        controller.cylinderIndex = MAKEWORD(data, MSB(controller.cylinderIndex));
        break;
    case 0x5:
        vlog(LogIDE, "Controller %d cylinder MSB set: %u", controllerIndex, data);
        controller.cylinderIndex = MAKEWORD(LSB(controller.cylinderIndex), data);
        break;
    case 0x6:
        vlog(LogIDE, "Controller %d head index set: %u", controllerIndex, data);
        controller.headIndex = data;
        break;
    case 0x7:
        // FIXME: ...
        vlog(LogIDE, "Controller %d received command %02X", controllerIndex, data);
        break;
    default:
        IODevice::out8(port, data);
    }
}

BYTE IDE::in8(WORD port)
{
    const int controllerIndex = (((port) & 0x1F0) == 0x170);
    const IDEController& controller = d->controller[controllerIndex];

    switch (port & 0xF) {
    case 0x1:
        vlog(LogIDE, "Controller %d error queried: %02X", controllerIndex, controller.error);
        return controller.error;
    case 0x2:
        vlog(LogIDE, "Controller %d sector count queried: %u", controllerIndex, controller.sectorCount);
        return controller.sectorCount;
    case 0x3:
        vlog(LogIDE, "Controller %d sector index queried: %u", controllerIndex, controller.sectorIndex);
        return controller.sectorIndex;
    case 0x4:
        vlog(LogIDE, "Controller %d cylinder LSB queried: %02X", controllerIndex, LSB(controller.cylinderIndex));
        return LSB(controller.cylinderIndex);
    case 0x5:
        vlog(LogIDE, "Controller %d cylinder MSB queried: %02X", controllerIndex, MSB(controller.cylinderIndex));
        return LSB(controller.cylinderIndex);
    case 0x6:
        vlog(LogIDE, "Controller %d head index queried: %u", controllerIndex, controller.headIndex);
        return controller.headIndex;
    case 0x7: {
        BYTE ret = status();
        vlog(LogIDE, "Controller %d status queried: %02X", controllerIndex, ret);
        return ret;
    }
    default:
        return IODevice::in8(port);
    }
}

IDE::Status IDE::status() const
{
    // FIXME: ...
    return static_cast<Status>(INDEX | DRDY);
}
