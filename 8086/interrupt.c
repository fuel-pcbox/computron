/* 8086/interrupt.c
 * Interrupt instructions
 *
 */

#include "vomit.h"

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
	{
		int_call( 4 );
	}
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

	if( trapint )
	{
		vlog( VM_CPUMSG, "%04X:%04X Interrupt %02X,%02X trapped", BCS, BIP, isr, cpu.regs.B.AH );
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
