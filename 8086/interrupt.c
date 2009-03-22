/* 8086/interrupt.c
 * Interrupt instructions
 *
 */

#include "vomit.h"
#include "debug.h"
#include <stdio.h>

void
_INT_imm8()
{
	byte imm = cpu_pfq_getbyte();
	int_call( imm );
}

void
_INT3()
{
	int_call( 3 );
}

void
_INTO()
{
	/* XXX: I've never seen this used, so it's probably good to log it. */
	vlog( VM_ALERT, "INTO used, can you believe it?" );

	if( cpu.OF == 1 )
		int_call( 4 );
}

void
_IRET()
{
	word nip = mem_pop();
	word ncs = mem_pop();
	cpu_jump( ncs, nip );
	cpu_setflags( mem_pop() );
}

void
int_call( byte isr )
{
	word segment, offset;

#ifdef VM_DEBUG
	if( trapint )
		vlog( VM_PICMSG, "%04X:%04X Interrupt %02X,%02X trapped", cpu.base_CS, cpu.base_IP, isr, cpu.regs.B.AH );
#endif

#if 0
	if( isr == 0x21 )
	{
		switch( cpu.regs.B.AH )
		{
			case 0x30:
				vlog( VM_DOSMSG, "GetVersion" );
				break;
			case 0x3B:
				vlog( VM_DOSMSG, "ChDir '%s'", mem_get_ascii$( cpu.DS, cpu.regs.W.DX ));
				break;
			case 0x3D:
				vlog( VM_DOSMSG, "Open '%s' (%s)", mem_get_ascii$( cpu.DS, cpu.regs.W.DX ), cpu.regs.B.AL == 0 ? "R" : cpu.regs.B.AL == 1 ? "W" : "RW" );
				break;
		}
	}

	if( isr == 0x2F )
	{
		switch( cpu.regs.W.AX )
		{
			case 0x1684:
				vlog( VM_DOSMSG, "Get API entry point" );
				break;
		}
	}
#endif

	if( isr == 0x06 )
	{
		vlog( VM_CPUMSG, "Invalid opcode trap at %04X:%04X (%02X)", cpu.base_CS, cpu.base_IP, mem_getbyte(cpu.base_CS, cpu.base_IP) );
	}

#ifdef VOMIT_SLEEP_WHEN_DOS_IDLE
	if( isr == 0x28 )
	{
		/* DOS idle interrupt, catch a quick rest! */
		usleep( 10 );
	}
#endif

	if( isr == 0x10 )
	{
		bios_interrupt10();
		return;
	}

	mem_push( cpu_getflags() );
	cpu.IF = 0;
	cpu.TF = 0;
	mem_push( cpu.CS );
	mem_push( cpu.IP );

	segment = (mem_space[isr * 4 + 3] << 8) | mem_space[isr * 4 + 2];
	offset = (mem_space[isr * 4 + 1] << 8) | mem_space[isr * 4];

	cpu_jump( segment, offset );
}
