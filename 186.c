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
	switch ( op ) {
	case 0xFF:		/* UD0 */
	case 0xB9:		/* UD1 */
	case 0x0B:		/* UD2 */
		int_call( 6 );	/* Undefined opcode */
		break;
	default:
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
	mem_push( BasePointer );
	FrameTemp = StackPointer;
	if ( NestingLevel != 0 ) {
		for ( i = 1; i <= ( NestingLevel - 1 ); ++i ) {
			BasePointer -= 2;
			mem_push( mem_getword( SS, BasePointer ) );
		}
	}
	mem_push( FrameTemp );
	BasePointer = FrameTemp;
	StackPointer = BasePointer - Size;
}

void
_LEAVE()
{
	StackPointer = BasePointer;
	BasePointer = mem_pop();
}

void
_PUSHA()
{
	word oldsp = StackPointer;
	mem_push(AX);
	mem_push(BX);
	mem_push(CX);
	mem_push(DX);
	mem_push(BasePointer);
	mem_push(oldsp);
	mem_push(SI);
	mem_push(DI);
}

void
_POPA()
{
	DI = mem_pop();
	SI = mem_pop();
	(void) mem_pop();
	BasePointer = mem_pop();
	DX = mem_pop();
	CX = mem_pop();
	BX = mem_pop();
	AX = mem_pop();
}

