#include "vomit.h"
#include "vcpu.h"
#include "vga.h"
#include "debug.h"
#include "disasm.h"
#include <string.h>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtGui/QColor>

static VGA theVGA;

typedef struct {
    BYTE r;
    BYTE g;
    BYTE b;
    operator QColor() { return QColor::fromRgb(r << 2, g << 2, b << 2); }
} RGBColor;

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

VGA* VGA::s_the = 0;

VGA* VGA::the()
{
    VM_ASSERT(s_the);
    return s_the;
}

VGA::VGA()
    : IODevice("VGA")
    , d(new Private)
{
    VM_ASSERT(!s_the);
    s_the = this;

    listen(0x3B4, IODevice::ReadWrite);
    listen(0x3B5, IODevice::ReadWrite);
    listen(0x3BA, IODevice::ReadWrite);
    listen(0x3C0, IODevice::Write);
    listen(0x3C1, IODevice::Read);
    listen(0x3C4, IODevice::Write);
    listen(0x3C5, IODevice::ReadWrite);
    listen(0x3C7, IODevice::Write);
    listen(0x3C8, IODevice::Write);
    listen(0x3C9, IODevice::ReadWrite);
    listen(0x3CA, IODevice::Read);
    listen(0x3CC, IODevice::Read);
    listen(0x3CE, IODevice::Write);
    listen(0x3CF, IODevice::ReadWrite);
    listen(0x3D4, IODevice::ReadWrite);
    listen(0x3D5, IODevice::ReadWrite);
    listen(0x3DA, IODevice::Read);

    d->columns = 80;
    d->rows = 0;

    d->currentRegister = 0;
    d->currentRegister2 = 0;
    d->currentSequencer = 0;

    memset(d->ioRegister, 0, sizeof(d->ioRegister));
    memset(d->ioRegister2, 0, sizeof(d->ioRegister2));
    memset(d->ioSequencer, 0, sizeof(d->ioSequencer));

    d->ioSequencer[2] = 0x0F;

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
}

VGA::~VGA()
{
    delete d;
    d = 0;
}

void VGA::out8(WORD port, BYTE data)
{
    switch (port) {
    case 0x3B4:
    case 0x3D4:
        d->currentRegister = data & 0x1F;
        if (d->currentRegister >= 0x12)
            vlog(VM_VIDEOMSG, "Invalid IO register #%u selected", d->currentRegister);
        break;

    case 0x3B5:
    case 0x3D5:
        if (d->currentRegister >= 0x12)
            vlog(VM_VIDEOMSG, "Invalid IO register #%u written", d->currentRegister);
        d->ioRegister[d->currentRegister] = data;
        break;

    case 0x3BA:
        vlog(VM_VIDEOMSG, "Writing FCR");
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
        if (d->currentSequencer >= 0x4)
            vlog(VM_VIDEOMSG, "Invalid IO sequencer #%u selected", d->currentSequencer);
        break;

    case 0x3C5:
        if (d->currentSequencer >= 0x4)
            vlog(VM_VIDEOMSG, "Invalid IO sequencer #%u written", d->currentSequencer);
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
        // vlog(VM_VIDEOMSG, "Setting component %u of color %02X to %02X", dac_data_subindex, dac_data_index, data);
        RGBColor& c = d->colorRegister[d->dac_data_write_index];
        switch (d->dac_data_write_subindex) {
        case 0:
            c.r = data;
            break;
        case 1:
            c.g = data;
            break;
        case 2:
            c.b = data;
            break;
        }

        if (++d->dac_data_write_subindex >= 3) {
            d->dac_data_write_subindex = 0;
            ++d->dac_data_write_index;
        }

        setPaletteDirty(true);
        g_cpu->vgaMemory()->syncPalette();
        break;
    }

    case 0x3CE:
        // FIXME: Find the number of valid registers and do something for OOB access.
        d->currentRegister2 = data;
        break;

    case 0x3CF:
        // FIXME: Find the number of valid registers and do something for OOB access.
        // vlog(VM_VIDEOMSG, "writing to reg2 %d, data is %02X", current_register2, data);
        d->ioRegister2[d->currentRegister2] = data;
        break;

    default:
        IODevice::out8(port, data);
    }
}

BYTE VGA::in8(WORD port)
{
    switch (port) {
    case 0x3B4:
    case 0x3D4:
        return d->currentRegister;

    case 0x3B5:
    case 0x3D5:
        if (d->currentRegister >= 0x12) {
            vlog(VM_VIDEOMSG, "Invalid IO register #%u read", d->currentRegister);
            return IODevice::JunkValue;
        }
        return d->ioRegister[d->currentRegister];

    case 0x3BA:
    case 0x3DA: {
        // 6845 - Port 3DA Status Register
        //
        //  |7|6|5|4|3|2|1|0|  3DA Status Register
        //  | | | | | | | `---- 1 = display enable, RAM access is OK
        //  | | | | | | `----- 1 = light pen trigger set
        //  | | | | | `------ 0 = light pen on, 1 = light pen off
        //  | | | | `------- 1 = vertical retrace, RAM access OK for next 1.25ms
        //  `-------------- unused

        // 0000 0100
        BYTE value = 0x04;

        // FIXME: Needs mutex protection (or more clever mechanism.)
        if (vomit_in_vretrace())
            value |= 0x08;

        d->next3C0IsIndex = true;
        return value;
    }

    case 0x3C1: {
        QMutexLocker locker(&d->paletteMutex);
        //vlog(VM_VIDEOMSG, "Read PALETTE[%u] (=%02X)", d->paletteIndex, d->paletteRegister[d->paletteIndex]);
        return d->paletteRegister[d->paletteIndex];
    }

    case 0x3C5:
        if (d->currentSequencer >= 0x4) {
            vlog(VM_VIDEOMSG, "Invalid IO sequencer #%u read", d->currentSequencer);
            return IODevice::JunkValue;
        }
        //vlog(VM_VIDEOMSG, "Reading sequencer %d, data is %02X", d->currentSequencer, d->ioSequencer[d->currentSequencer]);
        return d->ioSequencer[d->currentSequencer];

    case 0x3C9: {
        BYTE data = 0;
        switch (d->dac_data_read_subindex) {
        case 0:
            data = d->colorRegister[d->dac_data_read_index].r;
            break;
        case 1:
            data = d->colorRegister[d->dac_data_read_index].g;
            break;
        case 2:
            data = d->colorRegister[d->dac_data_read_index].b;
            break;
        }

        // vlog(VM_VIDEOMSG, "Reading component %u of color %02X (%02X)", dac_data_read_subindex, dac_data_read_index, data);

        if (++d->dac_data_read_subindex >= 3) {
            d->dac_data_read_subindex = 0;
            ++d->dac_data_read_index;
        }

        return data;
    }

    case 0x3CA:
        vlog(VM_VIDEOMSG, "Reading FCR");
        return 0x00;

    case 0x3CC:
        vlog(VM_VIDEOMSG, "Read MOR (Miscellaneous Output Register)");
        // 0x01: I/O at 0x3Dx (otherwise at 0x3Bx)
        // 0x02: RAM access enabled?
        return 0x03;

    case 0x3CF:
        // FIXME: Find the number of valid registers and do something for OOB access.
        // vlog(VM_VIDEOMSG, "reading reg2 %d, data is %02X", current_register2, io_register2[current_register2]);
        return d->ioRegister2[d->currentRegister2];

    default:
        return IODevice::in8(port);
    }
}

BYTE VGA::readRegister(BYTE index)
{
    VM_ASSERT(index < 0x12);
    return d->ioRegister[index];
}

BYTE VGA::readRegister2(BYTE index)
{
    // FIXME: Check if 12 is the correct limit here.
    VM_ASSERT(index < 0x12);
    return d->ioRegister2[index];
}

BYTE VGA::readSequencer(BYTE index)
{
    VM_ASSERT(index < 0x4);
    return d->ioSequencer[index];
}

void VGA::writeRegister(BYTE index, BYTE value)
{
    VM_ASSERT(index < 0x12);
    d->ioRegister[index] = value;
}

void VGA::setPaletteDirty(bool dirty)
{
    QMutexLocker locker(&d->paletteMutex);
    d->paletteDirty = dirty;
}

bool VGA::isPaletteDirty()
{
    QMutexLocker locker(&d->paletteMutex);
    return d->paletteDirty;
}

QColor VGA::paletteColor(int paletteIndex) const
{
    RGBColor& c = d->colorRegister[d->paletteRegister[paletteIndex]];
    return c;
}

QColor VGA::color(int index) const
{
    RGBColor& c = d->colorRegister[index];
    return c;
}

