/* 8086/flags.c
 * Handler of the hell that is flags
 *
 * 031102:	Fixed that AddOF fucker.
 *			DOS 3 & 4 are now functioning :-)
 *
 */

#include "vomit.h"

static const byte parity_table[0x100] = {
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
};

void
cpu_setAF( dword ans, word src, word dest )
{
	cpu.AF = ( ( ( ans ^ ( src ^ dest ) ) & 0x10 ) >> 4) & 1;
}

word
cpu_static_flags()
{
	switch( cpu.type )
	{
		case INTEL_8086:
		case INTEL_80186:
			return 0xF002;
	}
	return 0x0000;
}

void
cpu_update_flags16( word data )
{
    cpu.PF = parity_table[data & 0xFF];
	cpu.SF = (data & 0x8000) != 0;
	cpu.ZF = data == 0;
}

void
cpu_update_flags8( byte data )
{
    cpu.PF = parity_table[data];
	cpu.SF = (data & 0x80) != 0;
	cpu.ZF = data == 0;
}

void
cpu_update_flags( word data, byte bits )
{
	if( bits == 8 )
	{
		data &= 0xFF;
		cpu.PF = parity_table[data];
		cpu.SF = (data & 0x80) != 0;
	}
	else
	{
		cpu.PF = parity_table[data & 0xFF];
		cpu.SF = (data & 0x8000) != 0;
	}
	cpu.ZF = data == 0;
}

void
_STC()
{
	cpu.CF = 1;
}

void
_STD()
{
	cpu.DF = 1;
}

void
_STI()
{
	cpu.IF = 1;
}

void
_CLI()
{
	cpu.IF = 0;
}

void
_CLC()
{
	cpu.CF = 0;
}

void
_CLD()
{
	cpu.DF = 0;
}

void
_CMC()
{
	cpu.CF = !cpu.CF;
}

void
cpu_math_flags8( dword result, byte dest, byte src )
{
	cpu.CF = ( result & 0x0100 ) != 0;
	cpu.SF = ( result & 0x0080 ) != 0;
	cpu.ZF = ( result & 0x00FF ) == 0;
	cpu.PF = parity_table[result & 0xFF];
	cpu_setAF( result, dest, src );
}

void
cpu_math_flags16( dword result, word dest, word src )
{
	cpu.CF = ( result & 0x10000 ) != 0;
	cpu.SF = ( result &  0x8000 ) != 0;
	cpu.ZF = ( result &  0xFFFF ) == 0;
	cpu.PF = parity_table[result & 0xFF];
	cpu_setAF( result, dest, src );
}

void
cpu_cmp_flags8( dword result, byte dest, byte src )
{
	cpu_math_flags8( result, dest, src );
	cpu.OF = ((
	         ((src)^(dest)) &
	         ((src)^(src-dest))
	         )>>(7))&1;
}

void
cpu_cmp_flags16( dword result, word dest, word src )
{
	cpu_math_flags16( result, dest, src );
	cpu.OF = ((
	         ((src)^(dest)) &
	         ((src)^(src-dest))
	         )>>(15))&1;
}
