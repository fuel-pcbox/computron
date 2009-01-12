/* 8086/bitwise.c
 * Bitwise instructions
 *
 */

#define MASK_STEPS_IF_80286 if( cpu.type >= INTEL_80286 ) { steps &= 0x1F; }

#include "vomit.h"
#include "templates.h"

DEFAULT_RM8_reg8( cpu_and, _AND_RM8_reg8 )
DEFAULT_RM16_reg16( cpu_and, _AND_RM16_reg16 )
DEFAULT_reg8_RM8( cpu_and, _AND_reg8_RM8 )
DEFAULT_reg16_RM16( cpu_and, _AND_reg16_RM16 )
DEFAULT_RM8_imm8( cpu_and, _AND_RM8_imm8 )
DEFAULT_RM16_imm16( cpu_and, _AND_RM16_imm16 )
DEFAULT_RM16_imm8( cpu_and, _AND_RM16_imm8 )
DEFAULT_AL_imm8( cpu_and, _AND_AL_imm8 )
DEFAULT_AX_imm16( cpu_and, _AND_AX_imm16 )

DEFAULT_RM8_reg8( cpu_xor, _XOR_RM8_reg8 )
DEFAULT_RM16_reg16( cpu_xor, _XOR_RM16_reg16 )
DEFAULT_reg8_RM8( cpu_xor, _XOR_reg8_RM8 )
DEFAULT_reg16_RM16( cpu_xor, _XOR_reg16_RM16 )
DEFAULT_RM8_imm8( cpu_xor, _XOR_RM8_imm8 )
DEFAULT_RM16_imm16( cpu_xor, _XOR_RM16_imm16 )
DEFAULT_RM16_imm8( cpu_xor, _XOR_RM16_imm8 )
DEFAULT_AL_imm8( cpu_xor, _XOR_AL_imm8 )
DEFAULT_AX_imm16( cpu_xor, _XOR_AX_imm16 )

DEFAULT_RM8_reg8( cpu_or, _OR_RM8_reg8 )
DEFAULT_RM16_reg16( cpu_or, _OR_RM16_reg16 )
DEFAULT_reg8_RM8( cpu_or, _OR_reg8_RM8 )
DEFAULT_reg16_RM16( cpu_or, _OR_reg16_RM16 )
DEFAULT_RM8_imm8( cpu_or, _OR_RM8_imm8 )
DEFAULT_RM16_imm16( cpu_or, _OR_RM16_imm16 )
DEFAULT_RM16_imm8( cpu_or, _OR_RM16_imm8 )
DEFAULT_AL_imm8( cpu_or, _OR_AL_imm8 )
DEFAULT_AX_imm16( cpu_or, _OR_AX_imm16 )

READONLY_RM8_reg8( cpu_and, _TEST_RM8_reg8 )
READONLY_RM16_reg16( cpu_and, _TEST_RM16_reg16 )
READONLY_reg8_RM8( cpu_and, _TEST_reg8_RM8 )
READONLY_reg16_RM16( cpu_and, _TEST_reg16_RM16 )
READONLY_RM8_imm8( cpu_and, _TEST_RM8_imm8 )
READONLY_RM16_imm16( cpu_and, _TEST_RM16_imm16 )
READONLY_RM16_imm8( cpu_and, _TEST_RM16_imm8 )
READONLY_AL_imm8( cpu_and, _TEST_AL_imm8 )
READONLY_AX_imm16( cpu_and, _TEST_AX_imm16 )

void
_CBW()
{
	cpu.regs.W.AX = signext( cpu.regs.B.AL );
}

void
_CWD()
{
	if( cpu.regs.B.AH & 0x80 )
		cpu.regs.W.DX = 0xFFFF;
	else
		cpu.regs.W.DX = 0x0000;
}

void
_SALC()
{
	cpu.regs.B.AL = cpu.CF ? 0xFF : 0x00;
}

dword
cpu_or( word dest, word src, byte bits )
{
	dword result = dest | src;
	cpu_updflags( result, bits );
	cpu.OF = 0;
	cpu.CF = 0;
	return result;
}

dword
cpu_xor( word dest, word src, byte bits )
{
	dword result = dest ^ src;
	cpu_updflags( result, bits );
	cpu.OF = 0;
	cpu.CF = 0;
	return result;
}

dword
cpu_and( word dest, word src, byte bits )
{
	dword result = dest & src;
	cpu_updflags( result, bits );
	cpu.OF = 0;
	cpu.CF = 0;
	return result;
}

dword
cpu_shl( word data, byte steps, byte bits )
{
	unsigned int i;
	dword result = (dword)data;

	MASK_STEPS_IF_80286;

	if( bits == 8 )
	{
		for( i = 0; i < steps; ++i )
		{
			cpu.CF = (result>>7) & 1;
			result <<= 1;
		}
	}
	else
	{
		for( i = 0; i < steps; ++i )
		{
			cpu.CF = (result>>15) & 1;
			result <<= 1;
		}
	}

	if( steps == 1 )
	{
		cpu.OF = (data >> ( bits - 1 )) ^ cpu.CF;
	}

	cpu_updflags( result, bits );
	return result;
}

dword
cpu_shr( word data, byte steps, byte bits )
{
	unsigned int i;
	dword result = (dword)data;

	MASK_STEPS_IF_80286;

	if( bits == 8 )
	{
		for( i = 0; i < steps; ++i )
		{
			cpu.CF = result & 1;
			result >>= 1;
		}
	}
	else
	{
		for( i = 0; i < steps; ++i )
		{
			cpu.CF = result & 1;
			result >>= 1;
		}
	}

	if( steps == 1 )
	{
		cpu.OF = (data >> ( bits - 1 )) & 1;
	}

	cpu_updflags( result, bits );
	return result;
}

dword
cpu_sar( word data, byte steps, byte bits )
{
	unsigned int i;
	dword result = (dword)data;
	word n;

	MASK_STEPS_IF_80286;

	if( bits == 8 )
	{
		for( i = 0; i < steps; ++i )
		{
			n = result;
			result = (result>>1) | (n&0x80);
			cpu.CF = n & 1;
		}
	}
	else
	{
		for( i = 0; i < steps; ++i )
		{
			n = result;
			result = (result>>1) | (n&0x8000);
			cpu.CF = n & 1;
		}
	}

	if( steps == 1 )
	{
		cpu.OF = 0;
	}

	cpu_updflags( result, bits );
	return result;
}

dword
cpu_rol( word data, byte steps, byte bits )
{
	unsigned int i;
	dword result = (dword)data;

	MASK_STEPS_IF_80286;

	if( bits == 8 )
	{
		for( i = 0; i < steps; ++i )
		{
			cpu.CF = (result>>7) & 1;
			result = (result<<1) | cpu.CF;
		}
	}
	else
	{
		for( i = 0; i < steps; ++i )
		{
			cpu.CF = (result>>15) & 1;
			result = (result<<1) | cpu.CF;
		}
	}

	if( steps == 1 )
	{
		cpu.OF = ( ( result >> ( bits - 1 ) ) & 1 ) ^ cpu.CF;
	}

	return result;
}

dword
cpu_ror( word data, byte steps, byte bits )
{
	unsigned int i;
	dword result = (dword)data;

	MASK_STEPS_IF_80286;

	if( bits == 8 )
	{
		for( i = 0; i < steps; ++i )
		{
			cpu.CF = result & 1;
			result = (result>>1) | (cpu.CF<<7);
		}
	}
	else
	{
		for( i = 0; i < steps; ++i )
		{
			cpu.CF = result & 1;
			result = (result>>1) | (cpu.CF<<15);
		}
	}

	if( steps == 1 )
	{
		cpu.OF = (result >> (bits - 1)) ^ ((result >> (bits - 2) & 1));
	}

	return result;
}

dword
cpu_rcl( word data, byte steps, byte bits )
{
	unsigned int i;
	dword result = (dword)data;
	word n;

	MASK_STEPS_IF_80286;

	if( bits == 8 )
	{
		for( i = 0; i < steps; ++i )
		{
			n = result;
			result = ((result<<1) & 0xFF) | cpu.CF;
			cpu.CF = (n>>7) & 1;
		}
	}
	else
	{
		for( i = 0; i < steps; ++i )
		{
			n = result;
			result = ((result<<1) & 0xFFFF) | cpu.CF;
			cpu.CF = (n>>15) & 1;
		}
	}

	if( steps == 1 )
	{
		cpu.OF = (result >> (bits - 1)) ^ cpu.CF;
	}

	return result;
}

dword
cpu_rcr( word data, byte steps, byte bits )
{
	unsigned int i;
	dword result = (dword)data;
	word n;

	MASK_STEPS_IF_80286;

	if( bits == 8 )
	{
		for( i = 0; i < steps; ++i )
		{
			n = result;
			result = (result>>1) | (cpu.CF<<7);
			cpu.CF = n & 1;
		}
	}
	else
	{
		for( i = 0; i < steps; ++i )
		{
			n = result;
			result = (result>>1) | (cpu.CF<<15);
			cpu.CF = n & 1;
		}
	}

	if( steps == 1 )
	{
		cpu.OF = (result >> (bits - 1)) ^ ((result >> (bits - 2) & 1));
	}

	return result;
}

void
_NOT_RM8()
{
	byte value = modrm_read8( cpu_rmbyte );
	modrm_update8( ~value );
}

void
_NOT_RM16()
{
	word value = modrm_read16( cpu_rmbyte );
	modrm_update16( ~value );
}

void
_NEG_RM8()
{
	byte value = modrm_read8( cpu_rmbyte );
	byte old = value;
	value = -value;
	modrm_update8( value );
	cpu.CF = ( old != 0 );
	cpu_updflags( value, 8 );
	cpu.OF = ((
	         ((0)^(old)) &
	         ((0)^(value))
	         )>>(7))&1;
	cpu_setAF( value, 0, old );
}

void
_NEG_RM16()
{
	word value = modrm_read16( cpu_rmbyte );
	word old = value;
	value = -value;
	modrm_update16( value );
	cpu.CF = ( old != 0 );
	cpu_updflags( value, 16 );
	cpu.OF = ((
	         ((0)^(old)) &
	         ((0)^(value))
	         )>>(15))&1;
	cpu_setAF( value, 0, old );
}
