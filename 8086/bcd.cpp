/* 8086/bcd.c
 * BCD instruction handlers
 *
 * Based entirely on the Intel IA32 manual.
 *
 */

#include "vomit.h"

void
_AAA()
{
	if( ((cpu.regs.B.AL & 0x0F)>9) || cpu.AF )
	{
		cpu.regs.B.AL += 6;
		cpu.regs.B.AH += 1;
		cpu.AF = 1;
		cpu.CF = 1;
	}
	else
	{
		cpu.AF = 0;
		cpu.CF = 0;
	}
	cpu.regs.B.AL &= 0x0F;
}

void
_AAM()
{
	byte tempAL;
	byte imm = cpu_pfq_getbyte();

    if( imm == 0 )
	{
		/* Exceptions return to offending IP */
        cpu.IP--;
        int_call( 0 );
        return;
    }

	tempAL = cpu.regs.B.AL;

	cpu.regs.B.AH = tempAL / imm;
	cpu.regs.B.AL = tempAL % imm;
	cpu_update_flags8( cpu.regs.B.AL );
}

void
_AAD()
{
	byte tempAL = cpu.regs.B.AL;
	byte tempAH = cpu.regs.B.AH;
	byte imm = cpu_pfq_getbyte();

	cpu.regs.B.AL = (tempAL + (tempAH * imm)) & 0xFF;
	cpu.regs.B.AH = 0x00;
	cpu_update_flags8( cpu.regs.B.AL );
}

void
_AAS()
{
	if( ((cpu.regs.B.AL & 0x0F) > 9) || cpu.AF )
	{
		cpu.regs.B.AL -= 6;
		cpu.regs.B.AH -= 1;
		cpu.AF = 1;
		cpu.CF = 1;
	}
	else
	{
		cpu.AF = 0;
		cpu.CF = 0;
	}
}

void
_DAS()
{
    byte oldcf = cpu.CF;
	byte oldal = cpu.regs.B.AL;
	cpu.CF = 0;
    if( ((cpu.regs.B.AL & 0x0F) > 0x09) || cpu.AF )
	{
		/* TODO: eh? */
		cpu.CF = ((cpu.regs.B.AL-6) >> 8) & 1;
        cpu.regs.B.AL -= 0x06;
        cpu.CF = oldcf | cpu.CF;
        cpu.AF = 1;
    }
	else
	{
		cpu.AF = 0;
	}
    if( (oldal>0x99) || oldcf==1 )
	{
        cpu.regs.B.AL -= 0x60;
        cpu.CF = 1;
    }
	else
	{
		cpu.CF = 0;
	}
}

void
_DAA()
{
	byte oldcf = cpu.CF;
	byte oldal = cpu.regs.B.AL;
	cpu.CF = 0;
	if( ((cpu.regs.B.AL & 0x0F) > 0x09) || cpu.AF )
	{
		cpu.CF = ((cpu.regs.B.AL+6) >> 8) & 1;
		cpu.regs.B.AL += 6;
		cpu.CF = oldcf | cpu.CF;
		cpu.AF = 1;
	}
	else
	{
		cpu.AF = 0;
	}
	if( (oldal>0x99) || oldcf==1 )
	{
		cpu.regs.B.AL += 0x60;
		cpu.CF = 1;
	}
	else
	{
		cpu.CF = 0;
	}
}

