/* 186.c
 * 80186 instructions.
 * Mostly interpreted from the ever loveable IA32 manual.
 *
 */

#include "vomit.h"

void
_wrap_0x0F()
{
	byte op = cpu_pfq_getbyte();
	switch( op )
	{
		case 0xFF:		/* UD0 */
		case 0xB9:		/* UD1 */
		case 0x0B:		/* UD2 */
		default:
			vlog( VM_ALERT, "Undefinded opcode 0F %02X", op );
			int_call( 6 );
			break;
	}
}

void
_BOUND()
{
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr( rm, 16 );
	if ( ( *treg16[rmreg(rm)] < *(p) ) || ( *treg16[rmreg(rm)] >= *(p + 1) ) )
		int_call( 5 );	/* BR exception */
}

void
_PUSH_imm8()
{
	mem_push( signext( cpu_pfq_getbyte() ) );
}

void
_PUSH_imm16()
{
	mem_push( cpu_pfq_getword() );
}

void
_ENTER()
{
	word Size = cpu_pfq_getword();
	byte NestingLevel = cpu_pfq_getbyte() % 32;
	word FrameTemp, i;
	mem_push( cpu.regs.W.BP );
	FrameTemp = cpu.regs.W.SP;
	if ( NestingLevel != 0 ) {
		for ( i = 1; i <= ( NestingLevel - 1 ); ++i ) {
			cpu.regs.W.BP -= 2;
			mem_push( mem_getword( cpu.SS, cpu.regs.W.BP ) );
		}
	}
	mem_push( FrameTemp );
	cpu.regs.W.BP = FrameTemp;
	cpu.regs.W.SP = cpu.regs.W.BP - Size;
}

void
_LEAVE()
{
	cpu.regs.W.SP = cpu.regs.W.BP;
	cpu.regs.W.BP = mem_pop();
}

void
_PUSHA()
{
	word oldsp = cpu.regs.W.SP;
	mem_push( cpu.regs.W.AX );
	mem_push( cpu.regs.W.BX );
	mem_push( cpu.regs.W.CX );
	mem_push( cpu.regs.W.DX );
	mem_push( cpu.regs.W.BP );
	mem_push( oldsp );
	mem_push( cpu.regs.W.SI );
	mem_push( cpu.regs.W.DI );
}

void
_POPA()
{
	cpu.regs.W.DI = mem_pop();
	cpu.regs.W.SI = mem_pop();
	(void) mem_pop();
	cpu.regs.W.BP = mem_pop();
	cpu.regs.W.DX = mem_pop();
	cpu.regs.W.CX = mem_pop();
	cpu.regs.W.BX = mem_pop();
	cpu.regs.W.AX = mem_pop();
}

