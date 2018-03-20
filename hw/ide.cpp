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
#include "debug.h"
#include "ide.h"
#include "floppy.h"

#define IDE_DEBUG

static DWORD chs2lba(BYTE drive, WORD cyl, BYTE head, WORD sector)
{
    return (sector - 1) +
           (head * drv_spt[drive]) +
           (cyl * drv_spt[drive] * drv_heads[drive]);
}

struct IDEController
{
    unsigned controllerIndex { 0xffffffff };

    WORD cylinderIndex { 0 };
    BYTE sectorIndex { 0 };
    BYTE headIndex { 0 };
    BYTE sectorCount { 0 };
    BYTE error { 0 };
    bool inLBAMode { false };

    void identify(IDE&);
    void readSectors(IDE&);
    void writeSectors();

    DWORD lba() const
    {
        if (inLBAMode) {
            return ((DWORD)cylinderIndex << 8) | sectorIndex;
        }
        unsigned drive = controllerIndex + 2;
        return chs2lba(drive, cylinderIndex, headIndex, sectorIndex);
    }

    template<typename T> T readFromSectorBuffer();
    template<typename T> void writeToSectorBuffer(IDE&, T);

    QByteArray m_readBuffer;
    int m_readBufferIndex { 0 };

    QByteArray m_writeBuffer;
    int m_writeBufferIndex { 0 };
};

void IDEController::identify(IDE& ide)
{
    unsigned drive = controllerIndex + 2;
    WORD data[256];
    memset(data, 0, sizeof(data));
    data[1] = drv_sectors[drive] / (drv_spt[drive] * drv_heads[drive]);
    data[3] = drv_heads[drive];
    data[6] = drv_spt[drive];
    m_readBuffer.resize(512);
    memcpy(m_readBuffer.data(), data, sizeof(data));
    strcpy(m_readBuffer.data() + 54, "oCpmtuor niDks");
    m_readBufferIndex = 0;
    ide.raiseIRQ();
}

void IDEController::readSectors(IDE& ide)
{
    unsigned drive = controllerIndex + 2;
    vlog(LogIDE, "ide%u: Read sectors (LBA: %u, count: %u)", controllerIndex, lba(), sectorCount);
    FILE* f = fopen(drv_imgfile[drive], "rb");
    RELEASE_ASSERT(f);
    m_readBuffer.resize(drv_sectsize[drive] * sectorCount);
    int result;
    result = fseek(f, lba() * drv_sectsize[drive], SEEK_SET);
    ASSERT(result != -1);
    result = fread(m_readBuffer.data(), drv_sectsize[drive], sectorCount, f);
    ASSERT(result != -1);
    result = fclose(f);
    ASSERT(result != -1);
    m_readBufferIndex = 0;
    ide.raiseIRQ();
}

void IDEController::writeSectors()
{
    unsigned drive = controllerIndex + 2;
    vlog(LogIDE, "ide%u: Write sectors (LBA: %u, count: %u)", controllerIndex, lba(), sectorCount);
    m_writeBuffer.resize(drv_sectsize[drive] * sectorCount);
    m_writeBufferIndex = 0;
}

template<typename T>
void IDEController::writeToSectorBuffer(IDE& ide, T data)
{
    if (m_writeBufferIndex >= m_writeBuffer.size()) {
        vlog(LogIDE, "ide%u: Write buffer already full!");
        return;
    }
    if ((m_writeBufferIndex + static_cast<int>(sizeof(T))) > m_writeBuffer.size()) {
        vlog(LogIDE, "ide%u: Not enough space left in write buffer!");
        ASSERT_NOT_REACHED();
        return;
    }
    T* bufferPtr = reinterpret_cast<T*>(&m_writeBuffer.data()[m_writeBufferIndex]);
    *bufferPtr = data;
    m_writeBufferIndex += sizeof(T);
    if (m_writeBufferIndex < m_writeBuffer.size())
        return;
    vlog(LogIDE, "ide%u: Got all sector data, flushing to disk!", controllerIndex);
    unsigned drive = controllerIndex + 2;
    FILE* f = fopen(drv_imgfile[drive], "rb+");
    RELEASE_ASSERT(f);
    int result;
    result = fseek(f, lba() * drv_sectsize[drive], SEEK_SET);
    ASSERT(result != -1);
    result = fwrite(m_writeBuffer.data(), drv_sectsize[drive], sectorCount, f);
    ASSERT(result != -1);
    result = fclose(f);
    ASSERT(result != -1);
    ide.raiseIRQ();
}

template<typename T>
T IDEController::readFromSectorBuffer()
{
    if (m_readBufferIndex >= m_readBuffer.size()) {
        vlog(LogIDE, "ide%u: No data left in read buffer!");
        return 0;
    }
    if ((m_readBufferIndex + static_cast<int>(sizeof(T))) > m_readBuffer.size()) {
        vlog(LogIDE, "ide%u: Not enough data left in read buffer!");
        ASSERT_NOT_REACHED();
        return 0;
    }
    const T* data = reinterpret_cast<T*>(&m_readBuffer.data()[m_readBufferIndex]);
    m_readBufferIndex += sizeof(T);
    return *data;
}

static const int gNumControllers = 2;

struct IDE::Private
{
    IDEController controller[gNumControllers];
};

IDE::IDE(Machine& machine)
    : IODevice("IDE", machine, 14)
    , d(make<Private>())
{
    listen(0x170, IODevice::ReadWrite);
    listen(0x171, IODevice::ReadOnly);
    listen(0x172, IODevice::ReadWrite);
    listen(0x173, IODevice::ReadWrite);
    listen(0x174, IODevice::ReadWrite);
    listen(0x175, IODevice::ReadWrite);
    listen(0x176, IODevice::ReadWrite);
    listen(0x177, IODevice::ReadWrite);
    listen(0x1F0, IODevice::ReadWrite);
    listen(0x1F1, IODevice::ReadOnly);
    listen(0x1F2, IODevice::ReadWrite);
    listen(0x1F3, IODevice::ReadWrite);
    listen(0x1F4, IODevice::ReadWrite);
    listen(0x1F5, IODevice::ReadWrite);
    listen(0x1F6, IODevice::ReadWrite);
    listen(0x1F7, IODevice::ReadWrite);

    listen(0x3f6, IODevice::ReadOnly);

    reset();
}

IDE::~IDE()
{
}

void IDE::reset()
{
     d->controller[0] = IDEController();
     d->controller[0].controllerIndex = 0;
     d->controller[1] = IDEController();
     d->controller[1].controllerIndex = 1;
}

void IDE::out8(WORD port, BYTE data)
{
#ifdef IDE_DEBUG
    vlog(LogFDC, "out8 %03x, %02x", port, data);
#endif

    const int controllerIndex = (((port) & 0x1F0) == 0x170);
    IDEController& controller = d->controller[controllerIndex];

    switch (port & 0xF) {
    case 0x0:
        controller.writeToSectorBuffer<BYTE>(*this, data);
        break;
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
        controller.cylinderIndex = makeWORD(getMSB(controller.cylinderIndex), data);
        break;
    case 0x5:
        vlog(LogIDE, "Controller %d cylinder MSB set: %u", controllerIndex, data);
        controller.cylinderIndex = makeWORD(data, getLSB(controller.cylinderIndex));
        break;
    case 0x6:
        controller.headIndex = data & 0xf;
        controller.inLBAMode = data & 0x40;
        vlog(LogIDE, "Controller %d head index set: %u", controllerIndex, controller.headIndex);
        vlog(LogIDE, "Controller %d in %s mode", controllerIndex, controller.inLBAMode ? "LBA" : "CHS");
        break;
    case 0x7:
        // FIXME: ...
        vlog(LogIDE, "Controller %d received command %02X", controllerIndex, data);
        executeCommand(controller, data);
        break;
    default:
        IODevice::out8(port, data);
    }
}

BYTE IDE::in8(WORD port)
{
    int controllerIndex = (((port) & 0x1F0) == 0x170);
    IDEController& controller = d->controller[controllerIndex];

    // FIXME: This port should maybe be managed by the FDC?
    if (port == 0x3f6) {
        vlog(LogIDE, "Controller %d alternate status queried: %02X", controllerIndex, status(controller));
        return status(controller);
    }

    switch (port & 0xF) {
    case 0:
        return controller.readFromSectorBuffer<BYTE>();
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
        vlog(LogIDE, "Controller %d cylinder LSB queried: %02X", controllerIndex, getLSB(controller.cylinderIndex));
        return getLSB(controller.cylinderIndex);
    case 0x5:
        vlog(LogIDE, "Controller %d cylinder MSB queried: %02X", controllerIndex, getMSB(controller.cylinderIndex));
        return getMSB(controller.cylinderIndex);
    case 0x6:
        vlog(LogIDE, "Controller %d head index queried: %u", controllerIndex, controller.headIndex);
        return controller.headIndex;
    case 0x7: {
        BYTE ret = status(controller);
        vlog(LogIDE, "Controller %d status queried: %02X", controllerIndex, ret);
        return ret;
    }
    default:
        return IODevice::in8(port);
    }
}

WORD IDE::in16(WORD port)
{
    int controllerIndex = (((port) & 0x1f0) == 0x170);
    IDEController& controller = d->controller[controllerIndex];

    switch (port & 0xF) {
    case 0:
        return controller.readFromSectorBuffer<WORD>();
    default:
        return IODevice::in16(port);
    }
}

DWORD IDE::in32(WORD port)
{
    int controllerIndex = (((port) & 0x1f0) == 0x170);
    IDEController& controller = d->controller[controllerIndex];

    switch (port & 0xF) {
    case 0:
        return controller.readFromSectorBuffer<DWORD>();
    default:
        return IODevice::in16(port);
    }
}

void IDE::out16(WORD port, WORD data)
{
#ifdef IDE_DEBUG
    vlog(LogFDC, "out16 %03x, %04x", port, data);
#endif

    const int controllerIndex = (((port) & 0x1F0) == 0x170);
    IDEController& controller = d->controller[controllerIndex];

    switch (port & 0xF) {
    case 0x0:
        controller.writeToSectorBuffer<WORD>(*this, data);
        break;
    default:
        return IODevice::out16(port, data);
    }
}

void IDE::out32(WORD port, DWORD data)
{
#ifdef IDE_DEBUG
    vlog(LogFDC, "out32 %03x, %08x", port, data);
#endif

    const int controllerIndex = (((port) & 0x1F0) == 0x170);
    IDEController& controller = d->controller[controllerIndex];

    switch (port & 0xF) {
    case 0x0:
        controller.writeToSectorBuffer<DWORD>(*this, data);
        break;
    default:
        return IODevice::out16(port, data);
    }
}

void IDE::executeCommand(IDEController& controller, BYTE command)
{
    switch (command) {
    case 0x20:
    case 0x21:
        controller.readSectors(*this);
        break;
    case 0x30:
        controller.writeSectors();
        break;
    case 0xEC:
        controller.identify(*this);
        break;
#if 0
    case 0x90:
        // Run diagnostics, FIXME: this isn't a very nice implementation lol.
        controller.error = 0;
        raiseIRQ();
        break;
#endif
    default:
        vlog(LogIDE, "Unknown command %02x", command);
        break;
    }
}

IDE::Status IDE::status(const IDEController& controller) const
{
    // FIXME: ...
    unsigned status = INDEX | DRDY;
    if (controller.m_readBufferIndex < controller.m_readBuffer.size()) {
        status |= DRQ;
    }
    if (controller.m_writeBufferIndex < controller.m_writeBuffer.size()) {
        status |= DRQ;
    }

    return static_cast<Status>(status);
}
