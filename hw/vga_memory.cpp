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

#include "vga_memory.h"
#include "vomit.h"
#include "vcpu.h"
#include "vga.h"
#include "debug.h"
#include <QtGui/QImage>
#include <QtGui/QColor>
#include <QtGui/QBrush>
#include <QtCore/QDebug>

#define WRITE_MODE (VGA::the()->readRegister2(5) & 0x03)
#define READ_MODE ((VGA::the()->readRegister2(5) >> 3) & 1)
#define ODD_EVEN ((VGA::the()->readRegister2(5) >> 4) & 1)
#define SHIFT_REG ((VGA::the()->readRegister2(5) >> 5) & 0x03)
#define ROTATE ((VGA::the()->readRegister2(3)) & 0x07)
#define DRAWOP ((VGA::the()->readRegister2(3) >> 3) & 3)
#define MAP_MASK_BIT(i) ((VGA::the()->readSequencer(2) >> i)&1)
#define SET_RESET_BIT(i) ((VGA::the()->readRegister2(0) >> i)&1)
#define SET_RESET_ENABLE_BIT(i) ((VGA::the()->readRegister2(1) >> i)&1)
#define BIT_MASK (VGA::the()->readRegister2(8))

VGAMemory::VGAMemory(VCpu *cpu)
{
    m_cpu = cpu;

    m_screen12 = QImage(640, 480, QImage::Format_Indexed8);
    m_screen12.fill(0);

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
    VGA::the()->setPaletteDirty(true);

    m_dirty = true;
}

VGAMemory::~VGAMemory()
{
    delete [] m_plane[0];
}

void VGAMemory::write8(DWORD address, BYTE value)
{
    /*
     * fprintf(stderr,"mem_write: %02X:%04X = %02X <%d>, BM=%02X, ESR=%02X, SR=%02X\n", VGA::the()->readSequencer(2) & 0x0F, a-0xA0000, value, DRAWOP, BIT_MASK, VGA::the()->readRegister2(1), VGA::the()->readRegister2(0));
     */

    if (address >= 0xAFFFF) {
        // FIXME: Not sure that this is correct.
        vlog(LogVGA, "OOB write 0x%lx", address);
        m_cpu->writeUnmappedMemory8(address, value);
        return;
    }

    BYTE new_val[4];

    address -= 0xA0000;

    // FIXME: Don't get current video mode from BDA
    if (g_cpu->readUnmappedMemory8(0x449) == 0x13) {
        m_plane[0][address] = value;
        return;
    }

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
            vomit_exit(1);
        }
    } else if (WRITE_MODE == 0) {

        BYTE bitmask = BIT_MASK;
        BYTE set_reset = VGA::the()->readRegister2(0);
        BYTE enable_set_reset = VGA::the()->readRegister2(1);
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
            vomit_exit(0);
        }
    } else if(WRITE_MODE == 1) {
        new_val[0] = m_latch[0];
        new_val[1] = m_latch[1];
        new_val[2] = m_latch[2];
        new_val[3] = m_latch[3];
    } else {
        vlog(LogVGA, "Unsupported 6845 write mode %d", WRITE_MODE);
        vomit_exit(1);

        /* This is just here to make GCC stop worrying about accessing new_val[] uninitialized. */
        return;
    }

    /*
     * Check first if any planes should be written.
     */

    BYTE plane_mask = VGA::the()->readSequencer(2) & 0x0F;

    if (plane_mask) {
        if (plane_mask & 0x01)
            m_plane[0][address] = new_val[0];
        if (plane_mask & 0x02)
            m_plane[1][address] = new_val[1];
        if (plane_mask & 0x04)
            m_plane[2][address] = new_val[2];
        if (plane_mask & 0x08)
            m_plane[3][address] = new_val[3];

#define D(i) ((m_plane[0][address]>>i) & 1) | (((m_plane[1][address]>>i) & 1)<<1) | (((m_plane[2][address]>>i) & 1)<<2) | (((m_plane[3][address]>>i) & 1)<<3)

        // HACK: I don't really like this way of obtaining the current mode...
        if ((m_cpu->readUnmappedMemory8(0x449) & 0x7F) == 0x12 && address < (640 * 480 / 8)) {

            // address 100 -> pixels 800-807
            uchar *px = &m_screen12.bits()[address * 8];

            *(px++) = D(7);
            *(px++) = D(6);
            *(px++) = D(5);
            *(px++) = D(4);
            *(px++) = D(3);
            *(px++) = D(2);
            *(px++) = D(1);
            *(px++) = D(0);

            uint y = address / 80;
            uint x1 = (address % 80) * 8;

            // 1 changed byte == 8 dirty pixels
            QRect newDirty(x1, y, 8, 1);
            if (!m_dirtyRect.contains(newDirty))
                m_dirtyRect = m_dirtyRect.united(newDirty);
        }

        m_dirty = true;
    }
}

BYTE VGAMemory::read8(DWORD address) {
    if (READ_MODE != 0) {
        vlog(LogVGA, "ZOMG! READ_MODE = %u", READ_MODE);
        vomit_exit(1);
    }

    /* We're assuming READ_MODE == 0 now... */

    if (address < 0xB0000) {
        address -= 0xA0000;

        // FIXME: Don't get current video mode from BDA
        if (g_cpu->readUnmappedMemory8(0x449) == 0x13)
            return m_plane[0][address];

        m_latch[0] = m_plane[0][address];
        m_latch[1] = m_plane[1][address];
        m_latch[2] = m_plane[2][address];
        m_latch[3] = m_plane[3][address];
        /*
        fprintf(stderr, "mem_read: %02X {%02X, %02X, %02X, %02X}\n", m_latch[VGA::the()->readRegister2(4)], m_latch[0], m_latch[1], m_latch[2], m_latch[3]);
        */
        return m_latch[VGA::the()->readRegister2(4)];
    } else {
        // FIXME: Not sure that this is correct.
        vlog(LogVGA, "OOB read 0x%lx", address);
        return m_cpu->readUnmappedMemory8(address);
    }
}

void VGAMemory::write16(DWORD address, WORD value)
{
    write8(address, vomit_LSB(value));
    write8(address + 1, vomit_MSB(value));
}

WORD VGAMemory::read16(DWORD address)
{
    return vomit_MAKEWORD(read8(address), read8(address + 1));
}

BYTE *VGAMemory::plane(int index) const
{
    VM_ASSERT(index >= 0 && index <= 3);
    return m_plane[index];
}

void VGAMemory::syncPalette()
{
    synchronizeColors();
}

const QImage* VGAMemory::modeImage(BYTE mode) const
{
    if (mode == 0x12)
        return &m_screen12;

#ifdef VOMIT_DEBUG
    vlog(LogAlert, "Screen image for unknown mode 0x%02X requested. Crashing!", mode);
#endif
    return 0;
}

bool VGAMemory::isDirty() const
{
    return m_dirty;
}

QRect VGAMemory::dirtyRect() const
{
    return m_dirtyRect;
}

void VGAMemory::clearDirty()
{
    m_dirtyRect = QRect();
    m_dirty = false;
}

void VGAMemory::synchronizeColors()
{
    for (int i = 0; i < 16; ++i) {
        m_color[i] = VGA::the()->paletteColor(i);
        m_brush[i] = QBrush(m_color[i]);
        m_screen12.setColor(i, m_color[i].rgb());
    }
}
