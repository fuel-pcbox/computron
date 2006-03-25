/* 8086/math.c
 * Mathematical instuctions
 *
 */

#include "vomit.h"
#include "templates.h"

void
cpu_mathflags( dword result, word dest, word src, byte bits )
{
	if( bits == 8 )
	{
		cpu.CF = ( result >> 8 ) & 1;
		cpu.SF = ( result >> 7 ) & 1;
		cpu.ZF = ( result & 0x00FF ) == 0;
	}
	else
	{
		cpu.CF = ( result >> 16 ) & 1;
		cpu.SF = ( result >> 15 ) & 1;
		cpu.ZF = ( result & 0xFFFF ) == 0;
	}
	cpu_setPF( result );
	cpu_setAF( result, dest, src );
}

void
cpu_cmpflags( dword result, word dest, word src, byte bits )
{
	cpu_mathflags( result, dest, src, bits );
	cpu.OF = ((
	         ((src)^(dest)) &
	         ((src)^(src-dest))
	         )>>(bits-1))&1;
}

dword
cpu_add( word dest, word src, byte bits )
{
	dword result = dest + src;
	cpu_mathflags( result, dest, src, bits );
	cpu.OF = ((
	         ((result)^(dest)) &
	         ((result)^(src))
	         )>>(bits-1))&1;
	return result;
}

dword
cpu_adc( word dest, word src, byte bits )
{
	dword result;
	src += cpu.CF;
	result = dest + src;

	cpu_mathflags( result, dest, src, bits );
	cpu.OF = ((
	         ((result)^(dest)) &
	         ((result)^(src))
	         )>>(bits-1))&1;
	return result;
}

dword
cpu_sub( word dest, word src, byte bits )
{
	dword result = dest - src;
	cpu_cmpflags( result, dest, src, bits );
	return result;
}

dword
cpu_sbb( word dest, word src, byte bits )
{
	dword result;
	src += cpu.CF;
	result = dest - src;
	cpu_cmpflags( result, dest, src, bits );
	return result;
}

dword
cpu_mul( word acc, word multi, byte bits )
{
	dword result = acc * multi;
	cpu_mathflags( result, acc, multi, bits );

	/* 8086 CPUs set ZF on zero result */
	if( cpu.type == INTEL_8086 )
	{
		cpu.ZF = (result == 0);
	}
	return result;
}

dword
cpu_imul (word acc, word multi, byte bits) {
	sigword result = (sigword)acc * (sigword)multi;
	cpu_mathflags(result, acc, multi, bits);
	return result;
}

DEFAULT_RM8_reg8( cpu_add, _ADD_RM8_reg8 )
DEFAULT_RM16_reg16( cpu_add, _ADD_RM16_reg16 )
DEFAULT_reg8_RM8( cpu_add, _ADD_reg8_RM8 )
DEFAULT_reg16_RM16( cpu_add, _ADD_reg16_RM16 )
DEFAULT_RM8_imm8( cpu_add, _ADD_RM8_imm8 )
DEFAULT_RM16_imm16( cpu_add, _ADD_RM16_imm16 )
DEFAULT_RM16_imm8( cpu_add, _ADD_RM16_imm8 )
DEFAULT_AL_imm8( cpu_add, _ADD_AL_imm8 )
DEFAULT_AX_imm16( cpu_add, _ADD_AX_imm16 )

DEFAULT_RM8_reg8( cpu_adc, _ADC_RM8_reg8 )
DEFAULT_RM16_reg16( cpu_adc, _ADC_RM16_reg16 )
DEFAULT_reg8_RM8( cpu_adc, _ADC_reg8_RM8 )
DEFAULT_reg16_RM16( cpu_adc, _ADC_reg16_RM16 )
DEFAULT_RM8_imm8( cpu_adc, _ADC_RM8_imm8 )
DEFAULT_RM16_imm16( cpu_adc, _ADC_RM16_imm16 )
DEFAULT_RM16_imm8( cpu_adc, _ADC_RM16_imm8 )
DEFAULT_AL_imm8( cpu_adc, _ADC_AL_imm8 )
DEFAULT_AX_imm16( cpu_adc, _ADC_AX_imm16 )

DEFAULT_RM8_reg8( cpu_sub, _SUB_RM8_reg8 )
DEFAULT_RM16_reg16( cpu_sub, _SUB_RM16_reg16 )
DEFAULT_reg8_RM8( cpu_sub, _SUB_reg8_RM8 )
DEFAULT_reg16_RM16( cpu_sub, _SUB_reg16_RM16 )
DEFAULT_RM8_imm8( cpu_sub, _SUB_RM8_imm8 )
DEFAULT_RM16_imm16( cpu_sub, _SUB_RM16_imm16 )
DEFAULT_RM16_imm8( cpu_sub, _SUB_RM16_imm8 )
DEFAULT_AL_imm8( cpu_sub, _SUB_AL_imm8 )
DEFAULT_AX_imm16( cpu_sub, _SUB_AX_imm16 )

DEFAULT_RM8_reg8( cpu_sbb, _SBB_RM8_reg8 )
DEFAULT_RM16_reg16( cpu_sbb, _SBB_RM16_reg16 )
DEFAULT_reg8_RM8( cpu_sbb, _SBB_reg8_RM8 )
DEFAULT_reg16_RM16( cpu_sbb, _SBB_reg16_RM16 )
DEFAULT_RM8_imm8( cpu_sbb, _SBB_RM8_imm8 )
DEFAULT_RM16_imm16( cpu_sbb, _SBB_RM16_imm16 )
DEFAULT_RM16_imm8( cpu_sbb, _SBB_RM16_imm8 )
DEFAULT_AL_imm8( cpu_sbb, _SBB_AL_imm8 )
DEFAULT_AX_imm16( cpu_sbb, _SBB_AX_imm16 )

READONLY_RM8_reg8( cpu_sub, _CMP_RM8_reg8 )
READONLY_RM16_reg16( cpu_sub, _CMP_RM16_reg16 )
READONLY_reg8_RM8( cpu_sub, _CMP_reg8_RM8 )
READONLY_reg16_RM16( cpu_sub, _CMP_reg16_RM16 )
READONLY_RM8_imm8( cpu_sub, _CMP_RM8_imm8 )
READONLY_RM16_imm16( cpu_sub, _CMP_RM16_imm16 )
READONLY_RM16_imm8( cpu_sub, _CMP_RM16_imm8 )
READONLY_AL_imm8( cpu_sub, _CMP_AL_imm8 )
READONLY_AX_imm16( cpu_sub, _CMP_AX_imm16 )

void
_MUL_RM8()
{
	byte rm = cpu_rmbyte;
	byte *p = cpu_rmptr( rm, 8 );
	cpu.regs.W.AX = cpu_mul( cpu.regs.B.AL, *p, 8 );

	if( cpu.regs.B.AH == 0x00 )
	{
		cpu.CF = 0;
		cpu.OF = 0;
	}
	else
	{
		cpu.CF = 1;
		cpu.OF = 1;
	}
}

void
_MUL_RM16()
{
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr( rm, 16 );
	dword result = cpu_mul( cpu.regs.W.AX, *p, 16 );
	cpu.regs.W.AX = result & 0xFFFF;
	cpu.regs.W.DX = (result >> 16) & 0xFFFF;

	if( cpu.regs.W.DX == 0x0000 )
	{
		cpu.CF = 0;
		cpu.OF = 0;
	}
	else
	{
		cpu.CF = 1;
		cpu.OF = 1;
	}
}

void
_IMUL_RM8()
{
	byte rm = cpu_rmbyte;
	sigbyte *p = cpu_rmptr( rm, 8 );
	cpu.regs.W.AX = (sigword) cpu_imul( cpu.regs.B.AL, *p, 8 );

	if( cpu.regs.B.AH == 0x00 || cpu.regs.B.AH == 0xFF )
	{
		cpu.CF = 0;
		cpu.OF = 0;
	}
	else
	{
		cpu.CF = 1;
		cpu.OF = 1;
	}
}

void
_IMUL_RM16()
{
	byte rm = cpu_rmbyte;
	sigword *p = cpu_rmptr(rm, 16);
	sigdword result = (sigdword) cpu_imul( cpu.regs.W.AX, *p, 16 );
	cpu.regs.W.AX = result;
	cpu.regs.W.DX = result >> 16;

	if( cpu.regs.W.DX == 0x0000 || cpu.regs.W.DX == 0xFFFF )
	{
		cpu.CF = 0;
		cpu.OF = 0;
	}
	else
	{
		cpu.CF = 1;
		cpu.OF = 1;
	}
}

void
_DIV_RM8()
{
	byte rm = cpu_rmbyte;
	word offend = cpu.IP;
	byte *p = cpu_rmptr( rm, 8 );
	word tAX = cpu.regs.W.AX;

	if( *p == 0 )
	{
		/* Exceptions return to offending IP */
		cpu.IP = offend - 2;
		int_call( 0 );
		return;
	}
	cpu.regs.B.AL = (byte)(tAX / *p); /* Quote        */
	cpu.regs.B.AH = (byte)(tAX % *p); /* Remainder    */
}

void
_DIV_RM16()
{
	byte rm = cpu_rmbyte;
	word offend = cpu.IP;
	word *p = cpu_rmptr( rm, 16 );
	dword tDXAX = cpu.regs.W.AX + (cpu.regs.W.DX << 16);

	if( *p == 0 )
	{
		/* See above. */
		cpu.IP = offend - 2;
		int_call(0x00);
		return;
	}
	cpu.regs.W.AX = (word)(tDXAX / *p); /* Quote      */
	cpu.regs.W.DX = (word)(tDXAX % *p); /* Remainder  */
}

void
_IDIV_RM8()
{
	byte rm = cpu_rmbyte;
	word offend = cpu.IP;
	sigbyte *p = cpu_rmptr( rm, 8 );
	sigword tAX = (sigword)cpu.regs.W.AX;
	if( *p == 0 )
	{
		/* Exceptions return to offending IP */
		cpu.IP = offend - 2;
		int_call( 0 );
		return;
	}
	cpu.regs.B.AL = (sigbyte)(tAX / *p); /* Quote        */
	cpu.regs.B.AH = (sigbyte)(tAX % *p); /* Remainder    */
}

void
_IDIV_RM16()
{
	word offend = cpu.IP;
	sigword *p = cpu_rmptr( cpu_rmbyte, 16 );
	sigdword tDXAX = (sigword)(cpu.regs.W.AX + (cpu.regs.W.DX << 16));

	if( *p == 0 )
	{
		/* See above. */
		cpu.IP = offend - 2;
		int_call( 0 );
		return;
	}
	cpu.regs.W.AX = (sigword)(tDXAX / *p); /* Quote      */
	cpu.regs.W.DX = (sigword)(tDXAX % *p); /* Remainder  */
}
