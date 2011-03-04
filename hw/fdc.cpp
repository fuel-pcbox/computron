/*
 * NEC µPD765 FDC emulation
 * for the VOMIT 80186 emulator
 * by Andreas Kling
 */

#include "vomit.h"
#include "floppy.h"
#include "fdc.h"
#include "pic.h"
#include "debug.h"

#define DATA_REGISTER_READY 0x80
#define DATA_FROM_FDC_TO_CPU 0x40
#define DATA_FROM_CPU_TO_FDC 0x00

struct FDC::Private
{
    BYTE current_drive;
    bool fdc_enabled;
    bool dma_io_enabled;
    bool motor[2];
    BYTE fdc_data_direction;
    BYTE fdc_current_status_register;
    BYTE fdc_status_register[4];
    BYTE fdc_command[8];
    BYTE fdc_command_size;
    BYTE fdc_command_index;
    BYTE fdc_current_drive_cylinder[2];
    BYTE fdc_current_drive_head[2];
    QList<BYTE> fdc_command_result;
};

FDC theFDC;

FDC::FDC()
    : IODevice("FDC")
    , d(new Private)
{
#if 0
    listen(0x3F0, fdc_status_a, 0L);
    listen(0x3F1, fdc_status_b, 0L);
    listen(0x3F2, 0L, fdc_digital_output);
    listen(0x3F4, fdc_main_status, 0L);
    listen(0x3F5, fdc_data_fifo_read, fdc_data_fifo_write);
#endif
    listen(0x3F0, IODevice::Read);
    listen(0x3F1, IODevice::Read);
    listen(0x3F2, IODevice::Write);
    listen(0x3F4, IODevice::Read);
    listen(0x3F5, IODevice::ReadWrite);

    d->current_drive = 0;
    d->fdc_enabled = false;
    d->dma_io_enabled = false;
    d->fdc_data_direction = DATA_FROM_CPU_TO_FDC;
    d->fdc_current_status_register = 0;

    d->fdc_command_index = 0;
    d->fdc_command_size = 0;
    d->fdc_command[0] = 0;

    for (int i = 0; i < 2; ++i) {
        d->motor[i] = false;
        d->fdc_current_drive_cylinder[i] = 0;
        d->fdc_current_drive_head[i] = 0;
    }

    d->fdc_status_register[0] = 0;
    d->fdc_status_register[1] = 0;
    d->fdc_status_register[2] = 0;
    d->fdc_status_register[3] = 0;
}

FDC::~FDC()
{
    delete d;
    d = 0;
}

BYTE FDC::in8(WORD port)
{
    switch (port) {
    case 0x3F0: {
        BYTE data = 0x00;
        if (drv_status[1] != 0) {
            /* Second drive installed */
            data |= 0x40;
        }
        vlog(VM_FDCMSG, "Read status register A: %02X", data);
        return data;
    }
    case 0x3F1:
        vlog(VM_FDCMSG, "Read status register B: (FIXME)");
        // FIXME: What should this register contain?
        return 0;

    case 0x3F4: {
        BYTE status = 0;

        // 0x80 - MRQ  - main request (1: data register ready, 0: data register not ready)
        // 0x40 - DIO  - data input/output (1: controller ? cpu, 0: cpu ? controller)
        // 0x20 - NDMA - non-DMA mode (1: controller not in DMA mode, 0: controller in DMA mode)
        // 0x10 - BUSY - device busy (1: busy, 0: ready)
        // 0x08 - ACTD ..
        // 0x04 - ACTC ..
        // 0x02 - ACTB ..
        // 0x01 - ACTA - drive X in positioning mode

        status |= 0x80; // MRQ = 1
        status |= d->fdc_data_direction;

        if (!d->dma_io_enabled)
            status |= 0x20;

        vlog(VM_FDCMSG, "Read main status register: %02X (direction: %s)", status, (d->fdc_data_direction == DATA_FROM_CPU_TO_FDC) ? "to FDC" : "from FDC");

        return status;
    }

    case 0x3F5: {
        if (d->fdc_command_result.isEmpty()) {
            vlog(VM_FDCMSG, "Read from empty command result register");
            return IODevice::JunkValue;
        }

        BYTE value = d->fdc_command_result.takeFirst();
        vlog(VM_FDCMSG, "Read command result byte %02X", value);

        return value;
    }

    default:
        return IODevice::in8(port);
    }
}

void FDC::out8(WORD port, BYTE data)
{
    switch (port) {
    case 0x3F2: {
        bool old_fdc_enabled = d->fdc_enabled;

        vlog(VM_FDCMSG, "Writing to FDC digital output, data: %02X", data);

        d->current_drive = data & 3;
        d->fdc_enabled = (data & 0x04) != 0;
        d->dma_io_enabled = (data & 0x08) != 0;

        d->motor[0] = (data & 0x10) != 0;
        d->motor[1] = (data & 0x20) != 0;

        vlog(VM_FDCMSG, "  Current drive: %u", d->current_drive);
        vlog(VM_FDCMSG, "  FDC enabled:   %s", d->fdc_enabled ? "yes" : "no");
        vlog(VM_FDCMSG, "  DMA+I/O mode:  %s", d->dma_io_enabled ? "yes" : "no");

        vlog(VM_FDCMSG, "  Motors:        %u %u", d->motor[0], d->motor[1]);

        if (d->fdc_enabled != old_fdc_enabled)
            raiseIRQ();
    }

    case 0x3F5: {
        vlog(VM_FDCMSG, "Command: %02X", data);

        if (d->fdc_command_index == 0) {
            // Determine the command length
            switch (data) {
            case 0x08:
                d->fdc_command_size = 0;
                break;
            }
        }

        d->fdc_command[d->fdc_command_index++] = data;

        if (d->fdc_command_index >= d->fdc_command_size) {
            executeCommand();
            d->fdc_command_index = 0;
        }
        break;
    }

    default:
        IODevice::out8(port, data);
    }
}

void FDC::executeCommand()
{
    vlog(VM_FDCMSG, "Executing command %02X", d->fdc_command[0]);

    switch (d->fdc_command[0]) {
    case 0x08: // Sense Interrupt Status
        vlog(VM_FDCMSG, "Sense interrupt");
        d->fdc_data_direction = DATA_FROM_FDC_TO_CPU;
        d->fdc_command_result.clear();
        d->fdc_command_result.append(d->fdc_status_register[0]);
        d->fdc_command_result.append(d->fdc_current_drive_cylinder[0]);
        break;
    default:
        vlog(VM_FDCMSG, "Unknown command! %02X", d->fdc_command[0]);
    }
}

void FDC::raiseIRQ()
{
    theFDC.d->fdc_status_register[0] = theFDC.d->current_drive;
    theFDC.d->fdc_status_register[0] |= (theFDC.d->fdc_current_drive_head[theFDC.d->current_drive] * 0x02);
    theFDC.d->fdc_status_register[0] |= 0x20;
    PIC::raiseIRQ(6);
}
