/*
 * NEC µPD765 FDC emulation
 * for the VOMIT 80186 emulator
 * by Andreas Kling
 */

#include "vomit.h"

static word fdc_status_a( word, byte );
static word fdc_status_b( word, byte );
static word fdc_main_status( word, byte );
static void fdc_digital_output( word, word, byte );

static byte current_drive;
static bool fdc_enabled;
static bool dma_io_enabled;
static bool motor[4];

void
fdc_init()
{
	vm_listen( 0x3f0, fdc_status_a, 0L );
	vm_listen( 0x3f1, fdc_status_b, 0L );
	vm_listen( 0x3f2, 0L, fdc_digital_output );
	vm_listen( 0x3f4, fdc_main_status, 0L );
	//vm_listen( 0x3f5, fdc_command_status, 
	// etc..

	current_drive = 0;
	fdc_enabled = false;
	dma_io_enabled = false;

	motor[0] = false;
	motor[1] = false;
	motor[2] = false;
	motor[3] = false;
}

word
fdc_status_a( word port, byte bits )
{
	byte data = 0x00;
	(void) bits;

	if( drv_status[1] != 0 )
	{
		/* Second drive installed */
		data |= 0x40;
	}

	vlog( VM_FDCMSG, "Reading FDC status register A, data: %02X", data );
	return data;
}

word
fdc_status_b( word port, byte bits )
{
	(void) bits;
	vlog( VM_FDCMSG, "Reading FDC status register B" );
	return 0;
}

word
fdc_main_status( word port, byte bits )
{
	(void) bits;
	vlog( VM_FDCMSG, "Reading FDC main status register" );
	return 0;
}

void
fdc_digital_output( word port, word data, byte bits )
{
	vlog( VM_FDCMSG, "Writing to FDC digital output, data: %04X (%d bits)", data, bits );

	current_drive = data & 3;
	fdc_enabled = (data & 0x04) != 0;
	dma_io_enabled = (data & 0x08) != 0;

	motor[0] = (data & 0x10) != 0;
	motor[1] = (data & 0x10) != 0;
	motor[2] = (data & 0x10) != 0;
	motor[3] = (data & 0x10) != 0;

	vlog( VM_FDCMSG, "Current drive: %u", current_drive );
	vlog( VM_FDCMSG, "FDC enabled:   %s", fdc_enabled ? "yes" : "no" );
	vlog( VM_FDCMSG, "DMA+I/O mode:  %s", dma_io_enabled ? "yes" : "no" );

	vlog( VM_FDCMSG, "Motors:        %u %u %u %u", motor[0], motor[1], motor[2], motor[3] );
}
