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

#include "vga_memory.h"
#include "machine.h"
#include "screen.h"
#include "Common.h"
#include "vga.h"
#include "debug.h"
#include <QtGui/QImage>
#include <QtGui/QColor>
#include <QtGui/QBrush>

#define WRITE_MODE (machine().vga().readRegister2(5) & 0x03)
#define READ_MODE ((machine().vga().readRegister2(5) >> 3) & 1)
#define ODD_EVEN ((machine().vga().readRegister2(5) >> 4) & 1)
#define SHIFT_REG ((machine().vga().readRegister2(5) >> 5) & 0x03)
#define ROTATE ((machine().vga().readRegister2(3)) & 0x07)
#define DRAWOP ((machine().vga().readRegister2(3) >> 3) & 3)
#define MAP_MASK_BIT(i) ((machine().vga().readSequencer(2) >> i)&1)
#define SET_RESET_BIT(i) ((machine().vga().readRegister2(0) >> i)&1)
#define SET_RESET_ENABLE_BIT(i) ((machine().vga().readRegister2(1) >> i)&1)
#define BIT_MASK (machine().vga().readRegister2(8))

VGAMemory::VGAMemory(Machine& m)
    : m_machine(m)
{
    m_plane[0] = new BYTE[0x40000];
    m_plane[1] = m_plane[0] + 0x10000;
    m_plane[2] = m_plane[1] + 0x10000;
    m_plane[3] = m_plane[2] + 0x10000;

    memset(m_plane[0], 0x00, 0x40000);

    m_latch[0] = 0;
    m_latch[1] = 0;
    m_latch[2] = 0;
    m_latch[3] = 0;

    synchronizeColors();
    machine().vga().setPaletteDirty(true);
}

VGAMemory::~VGAMemory()
{
    delete [] m_plane[0];
}

template<typename T>
void VGAMemory::write(DWORD address, T value)
{
    switch (sizeof(T)) {
    case 1:
        write8(address, value);
        return;
    case 2:
        write8(address, getLSB(value));
        write8(address + 1, getMSB(value));
        return;
    case 4:
        write8(address + 0, getLSB(getLSW(value)));
        write8(address + 1, getMSB(getLSW(value)));
        write8(address + 2, getLSB(getMSW(value)));
        write8(address + 3, getMSB(getMSW(value)));
        return;
    }
    // FIXME: This should be a static assert somehow.
    ASSERT_NOT_REACHED();
}

void VGAMemory::write8(DWORD address, BYTE value)
{
    ASSERT(addressIsInVGAMemory(address));

    machine().notifyScreen();

    BYTE new_val[4];

    address -= 0xA0000;

    if (WRITE_MODE == 2) {

        BYTE bitmask = BIT_MASK;

        new_val[0] = m_latch[0] & ~bitmask;
        new_val[1] = m_latch[1] & ~bitmask;
        new_val[2] = m_latch[2] & ~bitmask;
        new_val[3] = m_latch[3] & ~bitmask;

        switch (DRAWOP) {
        case 0:
            new_val[0] |= (value & 1) ? bitmask : 0;
            new_val[1] |= (value & 2) ? bitmask : 0;
            new_val[2] |= (value & 4) ? bitmask : 0;
            new_val[3] |= (value & 8) ? bitmask : 0;
            break;
        default:
            vlog(LogVGA, "Gaah, unsupported raster op %d in mode 2 :(\n", DRAWOP);
            hard_exit(1);
        }
    } else if (WRITE_MODE == 0) {

        BYTE bitmask = BIT_MASK;
        BYTE set_reset = machine().vga().readRegister2(0);
        BYTE enable_set_reset = machine().vga().readRegister2(1);
        BYTE val = value;

        if (ROTATE) {
            vlog(LogVGA, "Rotate used!");
            val = (val >> ROTATE) | (val << (8 - ROTATE));
        }

        new_val[0] = m_latch[0] & ~bitmask;
        new_val[1] = m_latch[1] & ~bitmask;
        new_val[2] = m_latch[2] & ~bitmask;
        new_val[3] = m_latch[3] & ~bitmask;

        switch (DRAWOP) {
        case 0:
            new_val[0] |= ((enable_set_reset & 1)
                ? ((set_reset & 1) ? bitmask : 0)
                : (val & bitmask));
            new_val[1] |= ((enable_set_reset & 2)
                ? ((set_reset & 2) ? bitmask : 0)
                : (val & bitmask));
            new_val[2] |= ((enable_set_reset & 4)
                ? ((set_reset & 4) ? bitmask : 0)
                : (val & bitmask));
            new_val[3] |= ((enable_set_reset & 8)
                ? ((set_reset & 8) ? bitmask : 0)
                : (val & bitmask));
            break;
        case 1:
            new_val[0] |= ((enable_set_reset & 1)
                ? ((set_reset & 1)
                    ? (~m_latch[0] & bitmask)
                    : (m_latch[0] & bitmask))
                : (val & m_latch[0]) & bitmask);

            new_val[1] |= ((enable_set_reset & 2)
                ? ((set_reset & 2)
                    ? (~m_latch[1] & bitmask)
                    : (m_latch[1] & bitmask))
                : (val & m_latch[1]) & bitmask);

            new_val[2] |= ((enable_set_reset & 4)
                ? ((set_reset & 4)
                    ? (~m_latch[2] & bitmask)
                    : (m_latch[2] & bitmask))
                : (val & m_latch[2]) & bitmask);

            new_val[3] |= ((enable_set_reset & 8)
                ? ((set_reset & 8)
                    ? (~m_latch[3] & bitmask)
                    : (m_latch[3] & bitmask))
                : (val & m_latch[3]) & bitmask);
            break;
        case 3:
            new_val[0] |= ((enable_set_reset & 1)
                ? ((set_reset & 1)
                    ? (~m_latch[0] & bitmask)
                    : (m_latch[0] & bitmask))
                : (val ^ m_latch[0]) & bitmask);

            new_val[1] |= ((enable_set_reset & 2)
                ? ((set_reset & 2)
                    ? (~m_latch[1] & bitmask)
                    : (m_latch[1] & bitmask))
                : (val ^ m_latch[1]) & bitmask);

            new_val[2] |= ((enable_set_reset & 4)
                ? ((set_reset & 4)
                    ? (~m_latch[2] & bitmask)
                    : (m_latch[2] & bitmask))
                : (val ^ m_latch[2]) & bitmask);

            new_val[3] |= ((enable_set_reset & 8)
                ? ((set_reset & 8)
                    ? (~m_latch[3] & bitmask)
                    : (m_latch[3] & bitmask))
                : (val ^ m_latch[3]) & bitmask);
            break;
        default:
            vlog(LogVGA, "Unsupported raster operation %d", DRAWOP);
            hard_exit(0);
        }
    } else if(WRITE_MODE == 1) {
        new_val[0] = m_latch[0];
        new_val[1] = m_latch[1];
        new_val[2] = m_latch[2];
        new_val[3] = m_latch[3];
    } else {
        vlog(LogVGA, "Unsupported 6845 write mode %d", WRITE_MODE);
        hard_exit(1);

        /* This is just here to make GCC stop worrying about accessing new_val[] uninitialized. */
        return;
    }

    BYTE plane = 0xf;

    if (machine().vga().inChain4Mode()) {
        plane = 1 << (address & 0x3);
        address &= ~0x3;
    }

    plane &= machine().vga().readSequencer(2) & 0x0f;

    if (plane) {
        if (plane & 0x01)
            m_plane[0][address] = new_val[0];
        if (plane & 0x02)
            m_plane[1][address] = new_val[1];
        if (plane & 0x04)
            m_plane[2][address] = new_val[2];
        if (plane & 0x08)
            m_plane[3][address] = new_val[3];
    }
}

BYTE VGAMemory::read8(DWORD address)
{
    if (READ_MODE != 0) {
        vlog(LogVGA, "ZOMG! READ_MODE = %u", READ_MODE);
        hard_exit(1);
    }

    // FIXME: We're assuming READ_MODE == 0 from here on, this can't be safe.

    ASSERT(addressIsInVGAMemory(address));

    address -= 0xA0000;

    m_latch[0] = m_plane[0][address];
    m_latch[1] = m_plane[1][address];
    m_latch[2] = m_plane[2][address];
    m_latch[3] = m_plane[3][address];

    BYTE plane = machine().vga().readRegister2(4) & 0xf;
    if (machine().vga().inChain4Mode()) {
        plane = address & 3;
        address &= ~3;
    }

    return m_latch[plane];
}

template<typename T>
T VGAMemory::read(DWORD address)
{
    switch (sizeof(T)) {
    case 1:
        return read8(address);
    case 2:
        return makeWORD(read8(address + 1), read8(address));
    case 4:
        return makeDWORD(read<WORD>(address + 2), read<WORD>(address));
    }
    // FIXME: This should be a static assert somehow.
    ASSERT_NOT_REACHED();
}

template BYTE VGAMemory::read<BYTE>(DWORD address);
template WORD VGAMemory::read<WORD>(DWORD address);
template DWORD VGAMemory::read<DWORD>(DWORD address);
template void VGAMemory::write<BYTE>(DWORD address, BYTE value);
template void VGAMemory::write<WORD>(DWORD address, WORD value);
template void VGAMemory::write<DWORD>(DWORD address, DWORD value);

BYTE *VGAMemory::plane(int index) const
{
    ASSERT(index >= 0 && index <= 3);
    return m_plane[index];
}

void VGAMemory::synchronizeColors()
{
    for (int i = 0; i < 16; ++i) {
        m_color[i] = machine().vga().paletteColor(i);
        m_brush[i] = QBrush(m_color[i]);
    }
}
