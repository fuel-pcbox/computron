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

	if( OF == 1 )
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
#ifdef VM_DEBUG
	if( trapint )
	{
		vlog( VM_CPUMSG, "%04X:%04X Interrupt %02X,%02X trapped", BCS, BIP, isr, *treg8[REG_AH] );
	}
#endif

	/* XXX: VGA BIOS is currently residing in C land for... convenience.
	 *      I realize this breaks anything that relies on hooking. */
	if( isr == 0x10 )
	{
		bios_interrupt10();
		return;
	}

	mem_push( cpu_getflags() );
	IF = 0;
	TF = 0;
	mem_push( CS );
	mem_push( IP );

	/* TODO: Get rid of this ugly mess. */
#ifdef VM_DEBUG
	if( !mempeek )
	{
		cpu_jump(mem_getword(0, isr*4+2), mem_getword(0, isr*4));
	}
	else
	{
		mempeek = 0;
#endif
		cpu_jump(mem_getword(0, isr*4+2), mem_getword(0, isr*4));
#ifdef VM_DEBUG
		mempeek = 1;
	}
#endif
}
