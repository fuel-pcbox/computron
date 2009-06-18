/*
 * NEC µPD765 FDC emulation
 * for the VOMIT 80186 emulator
 * by Andreas Kling
 */

#include "vomit.h"
#include "debug.h"

#define DATA_REGISTER_READY 0x80
#define DATA_FROM_FDC_TO_CPU 0x40
#define DATA_FROM_CPU_TO_FDC 0x00

static byte fdc_status_a( word );
static byte fdc_status_b( word );
static byte fdc_main_status( word );
static void fdc_digital_output( word, byte );
static void fdc_data_fifo_write( word, byte );
static byte fdc_data_fifo_read( word );

static byte current_drive;
static bool fdc_enabled;
static bool dma_io_enabled;
static bool motor[4];
static byte fdc_data_direction;
static byte fdc_current_status_register;
static byte fdc_main_status_register;
static bool fdc_command_complete;
static byte fdc_command[8];
static byte fdc_command_size;
static byte fdc_command_index;

void
fdc_init()
{
	vm_listen( 0x3f0, fdc_status_a, 0L );
	vm_listen( 0x3f1, fdc_status_b, 0L );
	vm_listen( 0x3f2, 0L, fdc_digital_output );
	vm_listen( 0x3f4, fdc_main_status, 0L );
	vm_listen( 0x3f5, fdc_data_fifo_read, fdc_data_fifo_write );
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

byte
fdc_status_a( word port )
{
	byte data = 0x00;

	if( drv_status[1] != 0 )
	{
		/* Second drive installed */
		data |= 0x40;
	}

	vlog( VM_FDCMSG, "Reading FDC status register A, data: %02X", data );
	return data;
}

byte
fdc_status_b( word port )
{
	vlog( VM_FDCMSG, "Reading FDC status register B" );
	return 0;
}

byte
fdc_main_status( word port )
{
	vlog( VM_FDCMSG, "Reading FDC main status register: %02X", fdc_main_status_register );
	return fdc_main_status_register;
}

void
fdc_digital_output( word port, byte data )
{
	bool old_fdc_enabled = fdc_enabled;

	vlog( VM_FDCMSG, "Writing to FDC digital output, data: %02X", data );

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

	if( fdc_enabled != old_fdc_enabled )
		irq( 6 );
}

void
fdc_execute_command()
{
	vlog( VM_FDCMSG, "Executing command %02X", fdc_command[0] );
}

void
fdc_data_fifo_write( word port, byte data )
{
	if( fdc_command_complete )
	{
		fdc_command[0] = data;
		fdc_command_index = 1;
		fdc_command_complete = 0;

		switch( data )
		{
			case 0x08:	// Sense Interrupt Status
				vlog( VM_FDCMSG, "Sense interrupt" );
				fdc_data_direction = DATA_FROM_FDC_TO_CPU;
				break;
			default:
				vlog( VM_FDCMSG, "DATA FIFO Wr: %02X", data );
		}
	}
	else
	{
		fdc_command[fdc_command_index++] = data;
	}

	if( fdc_command_index == fdc_command_size )
	{
		fdc_execute_command();
		fdc_command_complete = 1;
	}
}

byte
fdc_data_fifo_read( word port )
{
	switch( fdc_current_status_register )
	{
		case 0:
			break;
	}

	vlog( VM_FDCMSG, "Read command status register %u\n", fdc_current_status_register );

	fdc_current_status_register++;

	if( fdc_current_status_register == 4 )
		fdc_current_status_register = 0;
}
