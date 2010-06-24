#include "vomit.h"
#include "vcpu.h"
#include "vga.h"
#include "debug.h"
#include "disasm.h" // MAKEWORD
#include <QtGui/QImage>
#include <QtGui/QColor>
#include <QtGui/QBrush>
#include <QtCore/QDebug>

#define WRITE_MODE (vga_read_register2(5) & 0x03)
#define READ_MODE ((vga_read_register2(5) >> 3) & 1)
#define ODD_EVEN ((vga_read_register2(5) >> 4) & 1)
#define SHIFT_REG ((vga_read_register2(5) >> 5) & 0x03)
#define ROTATE ((vga_read_register2(3)) & 0x07)
#define DRAWOP ((vga_read_register2(3) >> 3) & 3)
#define MAP_MASK_BIT(i) ((vga_read_sequencer(2) >> i)&1)
#define SET_RESET_BIT(i) ((vga_read_register2(0) >> i)&1)
#define SET_RESET_ENABLE_BIT(i) ((vga_read_register2(1) >> i)&1)
#define BIT_MASK (vga_read_register2(8))

struct VgaMemory::Private
{
    VCpu *cpu;

    BYTE* plane[4];
    BYTE latch[4];

    QImage screen12;
    QColor color[16];
    QBrush brush[16];

    bool dirty;
    QRect dirtyRect;

    void synchronizeColors()
    {
        for (int i = 0; i < 16; ++i) {
            color[i].setRgb(
                vga_color_register[vga_palette_register[i]].r << 2,
                vga_color_register[vga_palette_register[i]].g << 2,
                vga_color_register[vga_palette_register[i]].b << 2);

            brush[i] = QBrush(color[i]);
            screen12.setColor(i, color[i].rgb());
        }
    }
};

VgaMemory::VgaMemory(VCpu *cpu)
    : d(new Private)
{
    d->cpu = cpu;

    d->screen12 = QImage(640, 480, QImage::Format_Indexed8);
    d->screen12.fill(0);

    d->plane[0] = new BYTE[0x10000];
    d->plane[1] = new BYTE[0x10000];
    d->plane[2] = new BYTE[0x10000];
    d->plane[3] = new BYTE[0x10000];

    d->latch[0] = 0;
    d->latch[1] = 0;
    d->latch[2] = 0;
    d->latch[3] = 0;

    d->synchronizeColors();
    mark_palette_dirty();

    d->dirty = true;
}

VgaMemory::~VgaMemory()
{
    delete [] d->plane[3];
    delete [] d->plane[2];
    delete [] d->plane[1];
    delete [] d->plane[0];

    delete d;
    d = 0;
}

void VgaMemory::write8(DWORD address, BYTE value)
{
    /*
     * fprintf(stderr,"mem_write: %02X:%04X = %02X <%d>, BM=%02X, ESR=%02X, SR=%02X\n", vga_read_sequencer(2) & 0x0F, a-0xA0000, value, DRAWOP, BIT_MASK, vga_read_register2(1), vga_read_register2(0));
     */

    if (address >= 0xAFFFF) {
        vlog(VM_VIDEOMSG, "OOB write 0x%lx", address);
        d->cpu->memory[address] = value;
        return;
    }

    BYTE new_val[4];

    address -= 0xA0000;

    // FIXME: Don't get current video mode from BDA
    if (g_cpu->readUnmappedMemory8(0x449) == 0x13) {
        d->plane[0][address] = value;
        return;
    }

    if (WRITE_MODE == 2) {

        BYTE bitmask = BIT_MASK;

        new_val[0] = d->latch[0] & ~bitmask;
        new_val[1] = d->latch[1] & ~bitmask;
        new_val[2] = d->latch[2] & ~bitmask;
        new_val[3] = d->latch[3] & ~bitmask;

        switch (DRAWOP) {
        case 0:
            new_val[0] |= (value & 1) ? bitmask : 0;
            new_val[1] |= (value & 2) ? bitmask : 0;
            new_val[2] |= (value & 4) ? bitmask : 0;
            new_val[3] |= (value & 8) ? bitmask : 0;
            break;
        default:
            vlog(VM_VIDEOMSG, "Gaah, unsupported raster op %d in mode 2 :(\n", DRAWOP);
            vm_exit(1);
        }
    } else if (WRITE_MODE == 0) {

        BYTE bitmask = BIT_MASK;
        BYTE set_reset = vga_read_register2(0);
        BYTE enable_set_reset = vga_read_register2(1);
        BYTE val = value;

        if (ROTATE) {
            vlog(VM_VIDEOMSG, "Rotate used!");
            val = (val >> ROTATE) | (val << (8 - ROTATE));
        }

        new_val[0] = d->latch[0] & ~bitmask;
        new_val[1] = d->latch[1] & ~bitmask;
        new_val[2] = d->latch[2] & ~bitmask;
        new_val[3] = d->latch[3] & ~bitmask;

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
                    ? (~d->latch[0] & bitmask)
                    : (d->latch[0] & bitmask))
                : (val & d->latch[0]) & bitmask);

            new_val[1] |= ((enable_set_reset & 2)
                ? ((set_reset & 2)
                    ? (~d->latch[1] & bitmask)
                    : (d->latch[1] & bitmask))
                : (val & d->latch[1]) & bitmask);

            new_val[2] |= ((enable_set_reset & 4)
                ? ((set_reset & 4)
                    ? (~d->latch[2] & bitmask)
                    : (d->latch[2] & bitmask))
                : (val & d->latch[2]) & bitmask);

            new_val[3] |= ((enable_set_reset & 8)
                ? ((set_reset & 8)
                    ? (~d->latch[3] & bitmask)
                    : (d->latch[3] & bitmask))
                : (val & d->latch[3]) & bitmask);
            break;
        case 3:
            new_val[0] |= ((enable_set_reset & 1)
                ? ((set_reset & 1)
                    ? (~d->latch[0] & bitmask)
                    : (d->latch[0] & bitmask))
                : (val ^ d->latch[0]) & bitmask);

            new_val[1] |= ((enable_set_reset & 2)
                ? ((set_reset & 2)
                    ? (~d->latch[1] & bitmask)
                    : (d->latch[1] & bitmask))
                : (val ^ d->latch[1]) & bitmask);

            new_val[2] |= ((enable_set_reset & 4)
                ? ((set_reset & 4)
                    ? (~d->latch[2] & bitmask)
                    : (d->latch[2] & bitmask))
                : (val ^ d->latch[2]) & bitmask);

            new_val[3] |= ((enable_set_reset & 8)
                ? ((set_reset & 8)
                    ? (~d->latch[3] & bitmask)
                    : (d->latch[3] & bitmask))
                : (val ^ d->latch[3]) & bitmask);
            break;
        default:
            vlog(VM_VIDEOMSG, "Unsupported raster operation %d", DRAWOP);
            vm_exit(0);
        }
    } else if(WRITE_MODE == 1) {
        new_val[0] = d->latch[0];
        new_val[1] = d->latch[1];
        new_val[2] = d->latch[2];
        new_val[3] = d->latch[3];
    } else {
        vlog(VM_VIDEOMSG, "Unsupported 6845 write mode %d", WRITE_MODE);
        vm_exit(1);

        /* This is just here to make GCC stop worrying about accessing new_val[] uninitialized. */
        return;
    }

    /*
     * Check first if any planes should be written.
     */

    BYTE plane_mask = vga_read_sequencer(2) & 0x0F;

    if (plane_mask) {
        if (plane_mask & 0x01)
            d->plane[0][address] = new_val[0];
        if (plane_mask & 0x02)
            d->plane[1][address] = new_val[1];
        if (plane_mask & 0x04)
            d->plane[2][address] = new_val[2];
        if (plane_mask & 0x08)
            d->plane[3][address] = new_val[3];

#define D(i) ((d->plane[0][address]>>i) & 1) | (((d->plane[1][address]>>i) & 1)<<1) | (((d->plane[2][address]>>i) & 1)<<2) | (((d->plane[3][address]>>i) & 1)<<3)

        // HACK: I don't really like this way of obtaining the current mode...
        if ((d->cpu->memory[0x449] & 0x7F) == 0x12 && address < (640 * 480 / 8)) {

            // address 100 -> pixels 800-807
            uchar *px = &d->screen12.bits()[address * 8];

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
            if (!d->dirtyRect.contains(newDirty))
                d->dirtyRect = d->dirtyRect.united(newDirty);
        }

        d->dirty = true;
    }
}

BYTE VgaMemory::read8(DWORD address) {
    if (READ_MODE != 0) {
        vlog(VM_VIDEOMSG, "ZOMG! READ_MODE = %u", READ_MODE);
        vm_exit(1);
    }

    /* We're assuming READ_MODE == 0 now... */

    if (address < 0xB0000) {
        address -= 0xA0000;

        // FIXME: Don't get current video mode from BDA
        if (g_cpu->readUnmappedMemory8(0x449) == 0x13)
            return d->plane[0][address];

        d->latch[0] = d->plane[0][address];
        d->latch[1] = d->plane[1][address];
        d->latch[2] = d->plane[2][address];
        d->latch[3] = d->plane[3][address];
        /*
        fprintf(stderr, "mem_read: %02X {%02X, %02X, %02X, %02X}\n", d->latch[vga_read_register2(4)], d->latch[0], d->latch[1], d->latch[2], d->latch[3]);
        */
        return d->latch[vga_read_register2(4)];
    } else {
        vlog(VM_VIDEOMSG, "OOB read 0x%lx", address);
        return d->cpu->memory[address];
    }
}

void VgaMemory::write16(DWORD address, WORD value)
{
    write8(address, LSB(value));
    write8(address + 1, MSB(value));
}

WORD VgaMemory::read16(DWORD address)
{
    return MAKEWORD(read8(address), read8(address + 1));
}

BYTE *VgaMemory::plane(int index) const
{
    VM_ASSERT(d);
    VM_ASSERT(index >= 0 && index <= 3);
    return d->plane[index];
}

void VgaMemory::syncPalette()
{
    VM_ASSERT(d);
    d->synchronizeColors();
}

QImage *VgaMemory::modeImage(BYTE mode) const
{
    if (mode == 0x12)
        return &d->screen12;

#ifdef VOMIT_DEBUG
    vlog(VM_ALERT, "Screen image for unknown mode 0x%02X requested. Crashing!", mode);
#endif
    return 0;
}

bool VgaMemory::isDirty() const
{
    VM_ASSERT(d);
    return d->dirty;
}

QRect VgaMemory::dirtyRect() const
{
    VM_ASSERT(d);
    return d->dirtyRect;
}

void VgaMemory::clearDirty()
{
    VM_ASSERT(d);
    d->dirtyRect = QRect();
    d->dirty = false;
}

void VgaMemory::setDirty()
{
    VM_ASSERT(d);
    d->dirtyRect = QRect(0, 0, 640, 480);
    d->dirty = true;
}
