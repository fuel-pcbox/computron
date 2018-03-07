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
#include "vga.h"
#include "debug.h"
#include "machine.h"
#include "CPU.h"
#include <string.h>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtGui/QColor>

struct RGBColor {
    BYTE red;
    BYTE green;
    BYTE blue;
    operator QColor() const { return QColor::fromRgb(red << 2, green << 2, blue << 2); }
};

struct VGA::Private
{
    BYTE currentRegister;
    BYTE currentRegister2;
    BYTE currentSequencer;
    BYTE ioRegister[0x20];
    BYTE ioRegister2[0x20];
    BYTE ioSequencer[0x20];
    BYTE paletteIndex;
    BYTE columns;
    BYTE rows;

    BYTE dac_data_read_index;
    BYTE dac_data_read_subindex;
    BYTE dac_data_write_index;
    BYTE dac_data_write_subindex;

    bool next3C0IsIndex;
    bool paletteDirty;

    QMutex paletteMutex;

    BYTE paletteRegister[17];
    RGBColor colorRegister[256];

    bool screenInRefresh { false };
    BYTE statusRegister { 0 };
};

static const RGBColor default_vga_color_registers[256] =
{
    {0x00,0x00,0x00}, {0x00,0x00,0x2a}, {0x00,0x2a,0x00}, {0x00,0x2a,0x2a}, {0x2a,0x00,0x00}, {0x2a,0x00,0x2a}, {0x2a,0x15,0x00}, {0x2a,0x2a,0x2a},
    {0x00,0x00,0x00}, {0x00,0x00,0x2a}, {0x00,0x2a,0x00}, {0x00,0x2a,0x2a}, {0x2a,0x00,0x00}, {0x2a,0x00,0x2a}, {0x2a,0x15,0x00}, {0x2a,0x2a,0x2a},
    {0x15,0x15,0x15}, {0x15,0x15,0x3f}, {0x15,0x3f,0x15}, {0x15,0x3f,0x3f}, {0x3f,0x15,0x15}, {0x3f,0x15,0x3f}, {0x3f,0x3f,0x15}, {0x3f,0x3f,0x3f},
    {0x15,0x15,0x15}, {0x15,0x15,0x3f}, {0x15,0x3f,0x15}, {0x15,0x3f,0x3f}, {0x3f,0x15,0x15}, {0x3f,0x15,0x3f}, {0x3f,0x3f,0x15}, {0x3f,0x3f,0x3f},
    {0x00,0x00,0x00}, {0x00,0x00,0x2a}, {0x00,0x2a,0x00}, {0x00,0x2a,0x2a}, {0x2a,0x00,0x00}, {0x2a,0x00,0x2a}, {0x2a,0x15,0x00}, {0x2a,0x2a,0x2a},
    {0x00,0x00,0x00}, {0x00,0x00,0x2a}, {0x00,0x2a,0x00}, {0x00,0x2a,0x2a}, {0x2a,0x00,0x00}, {0x2a,0x00,0x2a}, {0x2a,0x15,0x00}, {0x2a,0x2a,0x2a},
    {0x15,0x15,0x15}, {0x15,0x15,0x3f}, {0x15,0x3f,0x15}, {0x15,0x3f,0x3f}, {0x3f,0x15,0x15}, {0x3f,0x15,0x3f}, {0x3f,0x3f,0x15}, {0x3f,0x3f,0x3f},
    {0x15,0x15,0x15}, {0x15,0x15,0x3f}, {0x15,0x3f,0x15}, {0x15,0x3f,0x3f}, {0x3f,0x15,0x15}, {0x3f,0x15,0x3f}, {0x3f,0x3f,0x15}, {0x3f,0x3f,0x3f},
};

VGA::VGA(Machine& m)
    : IODevice("VGA", m)
    , d(make<Private>())
{
    listen(0x3B4, IODevice::ReadWrite);
    listen(0x3B5, IODevice::ReadWrite);
    listen(0x3BA, IODevice::ReadWrite);
    listen(0x3C0, IODevice::WriteOnly);
    listen(0x3C1, IODevice::ReadOnly);
    listen(0x3C4, IODevice::ReadWrite);
    listen(0x3C5, IODevice::ReadWrite);
    listen(0x3C7, IODevice::WriteOnly);
    listen(0x3C8, IODevice::WriteOnly);
    listen(0x3C9, IODevice::ReadWrite);
    listen(0x3CA, IODevice::ReadOnly);
    listen(0x3CC, IODevice::ReadOnly);
    listen(0x3CE, IODevice::WriteOnly);
    listen(0x3CF, IODevice::ReadWrite);
    listen(0x3D4, IODevice::ReadWrite);
    listen(0x3D5, IODevice::ReadWrite);
    listen(0x3DA, IODevice::ReadOnly);

    reset();
}

VGA::~VGA()
{
}

void VGA::reset()
{
    d->columns = 80;
    d->rows = 0;

    d->currentRegister = 0;
    d->currentRegister2 = 0;
    d->currentSequencer = 0;

    memset(d->ioRegister, 0, sizeof(d->ioRegister));
    memset(d->ioRegister2, 0, sizeof(d->ioRegister2));
    memset(d->ioSequencer, 0, sizeof(d->ioSequencer));

    d->ioSequencer[2] = 0x0F;

    // Start with graphics mode bitmask 0xFF by default.
    // FIXME: This kind of stuff should be done by a VGA BIOS..
    d->ioRegister2[0x8] = 0xff;

    d->dac_data_read_index = 0;
    d->dac_data_read_subindex = 0;
    d->dac_data_write_index = 0;
    d->dac_data_write_subindex = 0;

    for (int i = 0; i < 16; ++i)
        d->paletteRegister[i] = i;
    d->paletteRegister[16] = 0x03;

    memcpy(d->colorRegister, default_vga_color_registers, sizeof(default_vga_color_registers));

    d->next3C0IsIndex = true;
    d->paletteDirty = true;
    d->screenInRefresh = false;
    d->statusRegister = 0;
}

void VGA::out8(WORD port, BYTE data)
{
    machine().notifyScreen();

    switch (port) {
    case 0x3B4:
    case 0x3D4:
        d->currentRegister = data & 0x1F;
        if (d->currentRegister > 0x18)
            vlog(LogVGA, "Invalid I/O register 0x%02X selected through port %03X", d->currentRegister, port);
        else if (options.vgadebug)
            vlog(LogVGA, "I/O register 0x%02X selected through port %03X", d->currentRegister, port);
        break;

    case 0x3B5:
    case 0x3D5:
        if (d->currentRegister > 0x18)
            vlog(LogVGA, "Invalid I/O register 0x%02X written (%02X) through port %03X", d->currentRegister, data, port);
        else if (options.vgadebug)
            vlog(LogVGA, "I/O register 0x%02X written (%02X) through port %03X", d->currentRegister, data, port);
        d->ioRegister[d->currentRegister] = data;
        break;

    case 0x3BA:
        vlog(LogVGA, "Writing FCR");
        break;

    case 0x3C0: {
        QMutexLocker locker(&d->paletteMutex);
        if (d->next3C0IsIndex)
            d->paletteIndex = data;
        else
            d->paletteRegister[d->paletteIndex] = data;
        d->next3C0IsIndex = !d->next3C0IsIndex;
        break;
    }

    case 0x3C4:
        d->currentSequencer = data & 0x1F;
        if (d->currentSequencer > 0x4)
            vlog(LogVGA, "Invalid I/O sequencer #%u selected", d->currentSequencer);
        break;

    case 0x3C5:
        if (d->currentSequencer > 0x4)
            vlog(LogVGA, "Invalid I/O sequencer #%u written", d->currentSequencer);
        d->ioSequencer[d->currentSequencer] = data;
        break;

    case 0x3C7:
        d->dac_data_read_index = data;
        d->dac_data_read_subindex = 0;
        break;

    case 0x3C8:
        d->dac_data_write_index = data;
        d->dac_data_write_subindex = 0;
        break;

    case 0x3C9: {
        // vlog(LogVGA, "Setting component %u of color %02X to %02X", dac_data_subindex, dac_data_index, data);
        RGBColor& color = d->colorRegister[d->dac_data_write_index];
        switch (d->dac_data_write_subindex) {
        case 0:
            color.red = data;
            d->dac_data_write_subindex = 1;
            break;
        case 1:
            color.green = data;
            d->dac_data_write_subindex = 2;
            break;
        case 2:
            color.blue = data;
            d->dac_data_write_subindex = 0;
            d->dac_data_write_index += 1;
            break;
        }

        setPaletteDirty(true);
        break;
    }

    case 0x3CE:
        // FIXME: Find the number of valid registers and do something for OOB access.
        d->currentRegister2 = data;
        break;

    case 0x3CF:
        // FIXME: Find the number of valid registers and do something for OOB access.
        //vlog(LogVGA, "Writing to reg2 %02x, data is %02x", d->currentRegister2, data);
        d->ioRegister2[d->currentRegister2] = data;
        break;

    default:
        IODevice::out8(port, data);
    }
}

void VGA::willRefreshScreen()
{
    d->screenInRefresh = true;
}

void VGA::didRefreshScreen()
{
    d->screenInRefresh = false;
    d->statusRegister |= 0x08;
}

BYTE VGA::in8(WORD port)
{
    switch (port) {
    case 0x3B4:
    case 0x3D4:
        return d->currentRegister;

    case 0x3B5:
    case 0x3D5:
        if (d->currentRegister > 0x18) {
            vlog(LogVGA, "Invalid I/O register 0x%02X read through port %03X", d->currentRegister, port);
            return IODevice::JunkValue;
        }
        if (options.vgadebug)
            vlog(LogVGA, "I/O register 0x%02X read through port %03X", d->currentRegister, port);
        return d->ioRegister[d->currentRegister];

    case 0x3BA:
    case 0x3DA: {
        BYTE value = d->statusRegister;
        // 6845 - Port 3DA Status Register
        //
        //  |7|6|5|4|3|2|1|0|  3DA Status Register
        //  | | | | | | | `---- 1 = display enable, RAM access is OK
        //  | | | | | | `----- 1 = light pen trigger set
        //  | | | | | `------ 0 = light pen on, 1 = light pen off
        //  | | | | `------- 1 = vertical retrace, RAM access OK for next 1.25ms
        //  `-------------- unused

        d->statusRegister ^= 0x01;
        d->statusRegister &= 0x01;

        d->next3C0IsIndex = true;
        return value;
    }

    case 0x3C1: {
        QMutexLocker locker(&d->paletteMutex);
        //vlog(LogVGA, "Read PALETTE[%u] (=%02X)", d->paletteIndex, d->paletteRegister[d->paletteIndex]);
        return d->paletteRegister[d->paletteIndex];
    }

    case 0x3C4:
        return d->currentSequencer;

    case 0x3C5:
        if (d->currentSequencer > 0x4) {
            vlog(LogVGA, "Invalid IO sequencer #%u read", d->currentSequencer);
            return IODevice::JunkValue;
        }
        //vlog(LogVGA, "Reading sequencer %d, data is %02X", d->currentSequencer, d->ioSequencer[d->currentSequencer]);
        return d->ioSequencer[d->currentSequencer];

    case 0x3C9: {
        BYTE data = 0;
        RGBColor& color = d->colorRegister[d->dac_data_read_index];
        switch (d->dac_data_read_subindex) {
        case 0:
            data = color.red;
            d->dac_data_read_subindex = 1;
            break;
        case 1:
            data = color.green;
            d->dac_data_read_subindex = 2;
            break;
        case 2:
            data = color.blue;
            d->dac_data_read_subindex = 0;
            d->dac_data_read_index += 1;
            break;
        }

        // vlog(LogVGA, "Reading component %u of color %02X (%02X)", dac_data_read_subindex, dac_data_read_index, data);
        return data;
    }

    case 0x3CA:
        vlog(LogVGA, "Reading FCR");
        return 0x00;

    case 0x3CC:
        vlog(LogVGA, "Read MOR (Miscellaneous Output Register)");
        // 0x01: I/O at 0x3Dx (otherwise at 0x3Bx)
        // 0x02: RAM access enabled?
        return 0x03;

    case 0x3CF:
        // FIXME: Find the number of valid registers and do something for OOB access.
        // vlog(LogVGA, "reading reg2 %d, data is %02X", current_register2, io_register2[current_register2]);
        return d->ioRegister2[d->currentRegister2];

    default:
        return IODevice::in8(port);
    }
}

BYTE VGA::readRegister(BYTE index)
{
    ASSERT(index <= 0x18);
    return d->ioRegister[index];
}

BYTE VGA::readRegister2(BYTE index)
{
    // FIXME: Check if 12 is the correct limit here.
    ASSERT(index < 0x12);
    return d->ioRegister2[index];
}

BYTE VGA::readSequencer(BYTE index)
{
    ASSERT(index < 0x5);
    return d->ioSequencer[index];
}

void VGA::writeRegister(BYTE index, BYTE value)
{
    ASSERT(index < 0x12);
    d->ioRegister[index] = value;
}

void VGA::setPaletteDirty(bool dirty)
{
    bool shouldEmit = dirty != d->paletteDirty;

    {
        QMutexLocker locker(&d->paletteMutex);
        d->paletteDirty = dirty;
    }

    if (shouldEmit)
        emit paletteChanged();
}

bool VGA::isPaletteDirty()
{
    QMutexLocker locker(&d->paletteMutex);
    return d->paletteDirty;
}

QColor VGA::paletteColor(int paletteIndex) const
{
    const RGBColor& c = d->colorRegister[d->paletteRegister[paletteIndex]];
    return c;
}

QColor VGA::color(int index) const
{
    const RGBColor& c = d->colorRegister[index];
    return c;
}

WORD VGA::startAddress() const
{
    return makeWORD(d->ioRegister[0x0C], d->ioRegister[0x0D]);
}

BYTE VGA::currentVideoMode() const
{
    // FIXME: This is not the correct way to obtain the video mode (BDA.)
    //        Need to find out how the 6845 stores this information.
    return machine().cpu().readUnmappedMemory8(0x449) & 0x7f;
}
