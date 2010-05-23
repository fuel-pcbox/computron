/*
 * NEC µPD765 FDC emulation
 * for the VOMIT 80186 emulator
 * by Andreas Kling
 */

#include "vomit.h"
#include "floppy.h"
#include "debug.h"

#define DATA_REGISTER_READY 0x80
#define DATA_FROM_FDC_TO_CPU 0x40
#define DATA_FROM_CPU_TO_FDC 0x00

static BYTE fdc_status_a(VCpu* cpu, WORD);
static BYTE fdc_status_b(VCpu* cpu, WORD);
static BYTE fdc_main_status(VCpu* cpu, WORD);
static void fdc_digital_output(VCpu* cpu, WORD, BYTE);
static void fdc_data_fifo_write(VCpu* cpu, WORD, BYTE);
static BYTE fdc_data_fifo_read(VCpu* cpu, WORD);

static BYTE current_drive;
static bool fdc_enabled;
static bool dma_io_enabled;
static bool motor[4];
static BYTE fdc_data_direction;
static BYTE fdc_current_status_register;
static bool fdc_command_complete;
static BYTE fdc_command[8];
static BYTE fdc_command_size;
static BYTE fdc_command_index;

void fdc_init()
{
	vm_listen(0x3f0, fdc_status_a, 0L);
	vm_listen(0x3f1, fdc_status_b, 0L);
	vm_listen(0x3f2, 0L, fdc_digital_output);
	vm_listen(0x3f4, fdc_main_status, 0L);
	vm_listen(0x3f5, fdc_data_fifo_read, fdc_data_fifo_write);
	// etc..

	current_drive = 0;
	fdc_enabled = false;
	dma_io_enabled = false;
	fdc_data_direction = DATA_FROM_CPU_TO_FDC;
	fdc_current_status_register = 0;

	fdc_command_index = 0;
	fdc_command_size = 0;
	fdc_command[0] = 0;
	fdc_command_complete = true;

	motor[0] = false;
	motor[1] = false;
	motor[2] = false;
	motor[3] = false;
}

BYTE fdc_status_a(VCpu*, WORD port)
{
	BYTE data = 0x00;

	if (drv_status[1] != 0) {
		/* Second drive installed */
		data |= 0x40;
	}

	vlog(VM_FDCMSG, "Reading FDC status register A, data: %02X", data);
	return data;
}

BYTE fdc_status_b(VCpu*, WORD port)
{
	vlog(VM_FDCMSG, "Reading FDC status register B");
	return 0;
}

BYTE fdc_main_status(VCpu*, WORD port)
{
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
    status |= fdc_data_direction;

	vlog(VM_FDCMSG, "Reading FDC main status register: %02X (direction: %s)", status,(fdc_data_direction == DATA_FROM_CPU_TO_FDC) ? "to FDC" : "from FDC");

	return status;
}

void fdc_digital_output(VCpu*, WORD port, BYTE data)
{
	bool old_fdc_enabled = fdc_enabled;

	vlog(VM_FDCMSG, "Writing to FDC digital output, data: %02X", data);

	current_drive = data & 3;
	fdc_enabled = (data & 0x04) != 0;
	dma_io_enabled = (data & 0x08) != 0;

	motor[0] = (data & 0x10) != 0;
	motor[1] = (data & 0x10) != 0;
	motor[2] = (data & 0x10) != 0;
	motor[3] = (data & 0x10) != 0;

	vlog(VM_FDCMSG, "  Current drive: %u", current_drive);
	vlog(VM_FDCMSG, "  FDC enabled:   %s", fdc_enabled ? "yes" : "no");
	vlog(VM_FDCMSG, "  DMA+I/O mode:  %s", dma_io_enabled ? "yes" : "no");

	vlog(VM_FDCMSG, "  Motors:        %u %u %u %u", motor[0], motor[1], motor[2], motor[3]);

	if (fdc_enabled != old_fdc_enabled) {
        vlog(VM_FDCMSG, "Raising IRQ");
		irq(6);
    }
}

void fdc_execute_command()
{
	vlog(VM_FDCMSG, "Executing command %02X", fdc_command[0]);
}

void fdc_data_fifo_write(VCpu*, WORD port, BYTE data)
{
	if (fdc_command_complete) {
		fdc_command[0] = data;
		fdc_command_index = 1;
		fdc_command_complete = 0;

		switch (data) {
			case 0x08:	// Sense Interrupt Status
				vlog(VM_FDCMSG, "Sense interrupt");
				//fdc_data_direction = DATA_FROM_FDC_TO_CPU;
				break;
			default:
				vlog(VM_FDCMSG, "DATA FIFO Wr: %02X", data);
		}
	} else {
		fdc_command[fdc_command_index++] = data;
	}

	if (fdc_command_index == fdc_command_size) {
		fdc_execute_command();
		fdc_command_complete = 1;
	}
}

BYTE fdc_data_fifo_read(VCpu*, WORD port)
{
	switch (fdc_current_status_register) {
		case 0:
			break;
	}

	vlog(VM_FDCMSG, "Read command status register %u\n", fdc_current_status_register);

	fdc_current_status_register++;

	if(fdc_current_status_register == 4)
		fdc_current_status_register = 0;

    return 0xAA;
}
