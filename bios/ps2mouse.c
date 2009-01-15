#include "vomit.h"
#include "8086.h"
#include <stdlib.h>

static bool mouse_enabled = false;
static word handler_segment = 0;
static word handler_offset = 0;

void
bios_ps2mouse()
{
	switch( cpu.regs.B.AL )
	{
		case 0x00:
			mouse_enabled = cpu.regs.B.BH == 1;
			vlog( VM_MOUSEMSG, "PS/2 mouse %s (BIOS)", mouse_enabled ? "enabled" : "disabled" );
			cpu.regs.B.AH = 0;
			cpu.CF = 0;
			break;
		case 0x01:
			vlog( VM_MOUSEMSG, "PS/2 mouse reset (BIOS)" );
			cpu.regs.B.AH = 0;
			cpu.regs.B.BH = 0x29; // "device ID" -- my birthday!
			cpu.regs.B.BL = 0xAA; // value returned by mouse at reset (0xAA == "hi, i'm a mouse")
			cpu.CF = 0;
			break;
		case 0x02:
			vlog( VM_MOUSEMSG, "PS/2 mouse sample rate set to %02X (BIOS)", cpu.regs.B.BH );
			cpu.regs.B.AH = 0;
			cpu.CF = 0;
			break;
		case 0x03:
			vlog( VM_MOUSEMSG, "PS/2 mouse resolution set to %02X (BIOS)", cpu.regs.B.BH );
			cpu.regs.B.AH = 0;
			cpu.CF = 0;
			break;
		case 0x04:
			vlog( VM_MOUSEMSG, "PS/2 mouse type queried (BIOS)" );
			cpu.regs.B.BH = 0x29;
			cpu.regs.B.AH = 0;
			cpu.CF = 0;
			break;
		case 0x05:
			vlog( VM_MOUSEMSG, "PS/2 mouse initialized: %u-byte pkg (BIOS)", cpu.regs.B.BH );
			cpu.regs.B.AH = 0;
			cpu.CF = 0;
			break;
		case 0x06:
			vlog( VM_MOUSEMSG, "PS/2 mouse status queried (BIOS)" );
			cpu.regs.B.AH = 0;
			cpu.CF = 0;
			break;
		case 0x07:
			vlog( VM_MOUSEMSG, "PS/2 mouse device handler registered (BIOS)" );
			vlog( VM_MOUSEMSG, "User routine at %04X:%04X", cpu.ES, cpu.regs.W.BX );
			handler_segment = cpu.ES;
			handler_offset = cpu.regs.W.BX;
			cpu.regs.B.AH = 0;
			cpu.CF = 0;
			break;
		default:
			vlog( VM_MOUSEMSG, "Unsupported PS/2 mouse request AX=%04X BX=%04X", cpu.regs.W.AX, cpu.regs.W.BX );
			cpu.CF = 1;
			cpu.regs.B.AH = 0x01;
			dump_all();
	}
}

void
bios_ps2mouse_irq()
{
	if( !mouse_enabled ) return;
	irq( 12 );
}

word mouse_x = 0;
word mouse_y = 0;

void
bios_ps2mouse_pulse()
{
	if( !handler_segment && !handler_offset )
	{
		cpu.CF = 1;
		return;
	}

	static int last_x = 0;
	static int last_y = 0;

	int current_x = get_current_x();
	int current_y = get_current_y();

	int delta_x = current_x - last_x;
	int delta_y = current_y - last_y;

	last_x = current_x;
	last_y = current_y;

	cpu.CF = 0;

	vlog( VM_MOUSEMSG, "dX = %d, dY = %d", delta_x, delta_y );

	byte status = 0x08;

	// if( button1 ) status |= 0x01;
	// if( button2 ) status |= 0x02;

	if( delta_x < 0 )
	{
		status |= 0x10;
		delta_x = -delta_x;
	}

	if( delta_y < 0 )
	{
		status |= 0x20;
		delta_y = -delta_y;
	}

	delta_x /= 2;
	delta_y /= 2;

	mem_push( 0x00 );
	mem_push( delta_x );
	mem_push( delta_y );
	mem_push( 0x00 );

	mem_push( cpu.CS );
	mem_push( cpu.IP );

	//vlog( VM_MOUSEMSG, "Exec user routine %04X:%04X", handler_segment, handler_offset );
	//vlog( VM_MOUSEMSG, " - return to %04X:%04X", cpu.base_CS, cpu.base_IP );

	cpu_jump( handler_segment, handler_offset );

	//vm_debug();
}
