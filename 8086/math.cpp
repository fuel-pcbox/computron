/* 8086/math.c
 * Mathematical instuctions
 *
 */

#include "vomit.h"
#include "templates.h"
#include "debug.h"

word
cpu_add8( byte dest, byte src )
{
	word result = dest + src;
	cpu_math_flags8( result, dest, src );
	cpu.OF = ((
	         ((result)^(dest)) &
	         ((result)^(src))
	         )>>(7))&1;
	return result;
}

dword
cpu_add16( word dest, word src )
{
	dword result = dest + src;
	cpu_math_flags16( result, dest, src );
	cpu.OF = ((
	         ((result)^(dest)) &
	         ((result)^(src))
	         )>>(15))&1;
	return result;
}

word
cpu_adc8( word dest, word src )
{
	word result;
	src += cpu.CF;
	result = dest + src;

	cpu_math_flags8( result, dest, src );
	cpu.OF = ((
	         ((result)^(dest)) &
	         ((result)^(src))
	         )>>(7))&1;
	return result;
}

dword
cpu_adc16( word dest, word src )
{
	dword result;
	src += cpu.CF;
	result = dest + src;

	cpu_math_flags16( result, dest, src );
	cpu.OF = ((
	         ((result)^(dest)) &
	         ((result)^(src))
	         )>>(15))&1;
	return result;
}

word
cpu_sub8( byte dest, byte src )
{
	word result = dest - src;
	cpu_cmp_flags8( result, dest, src );
	return result;
}

dword
cpu_sub16( word dest, word src )
{
	dword result = dest - src;
	cpu_cmp_flags16( result, dest, src );
	return result;
}

word
cpu_sbb8( byte dest, byte src )
{
	word result;
	src += cpu.CF;
	result = dest - src;
	cpu_cmp_flags8( result, dest, src );
	return result;
}

dword
cpu_sbb16( word dest, word src )
{
	dword result;
	src += cpu.CF;
	result = dest - src;
	cpu_cmp_flags16( result, dest, src );
	return result;
}

word
cpu_mul8( byte acc, byte multi )
{
	word result = acc * multi;
	cpu_math_flags8( result, acc, multi );

	/* 8086 CPUs set ZF on zero result */
	if( cpu.type == INTEL_8086 )
	{
		cpu.ZF = (result == 0);
	}
	return result;
}

dword
cpu_mul16( word acc, word multi )
{
	dword result = acc * multi;
	cpu_math_flags16( result, acc, multi );

	/* 8086 CPUs set ZF on zero result */
	if( cpu.type == INTEL_8086 )
	{
		cpu.ZF = (result == 0);
	}
	return result;
}

sigword
cpu_imul8( sigbyte acc, sigbyte multi )
{
	sigword result = acc * multi;
	cpu_math_flags8( result, acc, multi );
	return result;
}


sigdword
cpu_imul16( sigword acc, sigword multi )
{
	sigdword result = acc * multi;
	cpu_math_flags16( result, acc, multi );
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
	byte value = modrm_read8( cpu.rmbyte );
	cpu.regs.W.AX = cpu_mul8( cpu.regs.B.AL, value );

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
	word value = modrm_read16( cpu.rmbyte );
	dword result = cpu_mul16( cpu.regs.W.AX, value );
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
	byte rm = cpu.rmbyte;
	sigbyte value = (sigbyte)modrm_read8( rm );
	cpu.regs.W.AX = (sigword) cpu_imul8( cpu.regs.B.AL, value );

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
_IMUL_reg16_RM16_imm8()
{
	byte rm = cpu_pfq_getbyte();
	byte imm = cpu_pfq_getbyte();
	sigword value = (sigword)modrm_read16( rm );
	sigword result = cpu_imul16( value, imm );

	*treg16[rmreg(rm)] = result;

	if( (result & 0xFF00) == 0x00 || (result & 0xFF00) == 0xFF )
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
	sigword value = modrm_read16( cpu.rmbyte );
	sigdword result = cpu_imul16( cpu.regs.W.AX, value );
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
	word offend = cpu.IP;
	byte value = modrm_read8( cpu.rmbyte );
	word tAX = cpu.regs.W.AX;

	if( value == 0 )
	{
		/* Exceptions return to offending IP */
		cpu.IP = offend - 2;
		int_call( 0 );
		return;
	}
	cpu.regs.B.AL = (byte)(tAX / value); /* Quote        */
	cpu.regs.B.AH = (byte)(tAX % value); /* Remainder    */
}

void
_DIV_RM16()
{
	word offend = cpu.IP;
	word value = modrm_read16( cpu.rmbyte );
	dword tDXAX = cpu.regs.W.AX + (cpu.regs.W.DX << 16);

	if( value == 0 )
	{
		/* Exceptions return to offending IP */
		cpu.IP = offend - 2;
		int_call( 0 );
		return;
	}
	cpu.regs.W.AX = (word)(tDXAX / value); /* Quote      */
	cpu.regs.W.DX = (word)(tDXAX % value); /* Remainder  */
}

void
_IDIV_RM8()
{
	word offend = cpu.IP;
	sigbyte value = (sigbyte)modrm_read8( cpu.rmbyte );
	sigword tAX = (sigword)cpu.regs.W.AX;

	if( value == 0 )
	{
		/* Exceptions return to offending IP */
		cpu.IP = offend - 2;
		int_call( 0 );
		return;
	}
	cpu.regs.B.AL = (sigbyte)(tAX / value); /* Quote        */
	cpu.regs.B.AH = (sigbyte)(tAX % value); /* Remainder    */
}

void
_IDIV_RM16()
{
	word offend = cpu.IP;
	sigword value = modrm_read16( cpu.rmbyte );
	sigdword tDXAX = (cpu.regs.W.AX + (cpu.regs.W.DX << 16));

	if( value == 0 )
	{
		/* Exceptions return to offending IP */
		cpu.IP = offend - 2;
		int_call( 0 );
		return;
	}
	cpu.regs.W.AX = (sigword)(tDXAX / value); /* Quote      */
	cpu.regs.W.DX = (sigword)(tDXAX % value); /* Remainder  */
}
