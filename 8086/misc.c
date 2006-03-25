/* 8086/misc.c
 * Miscellaneous instructions
 *
 */

#include "vomit.h"
#include <unistd.h>
#include <stdlib.h>

void
_NOP()
{
}

void
_HLT()
{
	/* XXX: When halted, we're not really waiting for an interrupt.
	 *      We should, though. */
	cpu_state = CPU_HALTED;
	vlog( VM_ALERT, "CPU halted. Awaiting interrupt..." );
	if( g_try_run )
	{
		vm_kill();
		dump_try();
		exit( 0 );
	}
	while( cpu_state == CPU_HALTED )
	{
		/* Sleep for 100ms when halted. Prevents resource sucking. */
		usleep( 100 );

		/* TODO: This is a clone of code in cpu_main(). Me no likey. */
		if( g_break_pressed )
		{
			ui_kill();
			vm_debug();
			if( !g_debug_step )
				ui_show();
			g_break_pressed = false;
			/* TODO: int_call( 9 ); */
		}

		ui_sync();
	}
}

void
cpu_updflags (word data, byte bits) {
	if(bits==8) data &= 0xFF; else data &= 0xFFFF;
	cpu_setPF((dword)data);
	cpu_setZF((dword)data);
	cpu_setSF((dword)data, bits);
}

void _STC() { cpu.CF = 1; } void _STD() { cpu.DF = 1; } void _STI() { cpu.IF = 1; }
void _CLC() { cpu.CF = 0; } void _CLD() { cpu.DF = 0; } void _CLI() { cpu.IF = 0; }
void _CMC() { cpu.CF = !cpu.CF; }

void
_XLAT() {
	cpu.regs.B.AL = mem_getbyte( *(cpu.CurrentSegment), cpu.regs.W.BX + cpu.regs.B.AL );
}

void
_CS()
{
	byte opcode;
	cpu.SegmentPrefix = cpu.CS;
	cpu.CurrentSegment = &cpu.SegmentPrefix;
	opcode = cpu_pfq_getbyte();
	cpu_optable[opcode]();
	cpu.CurrentSegment = &cpu.DS;
	if( cpu.TF && cpu.IF )
	{
		int_call( 1 );
	}
}

void
_DS()
{
	byte opcode;
	cpu.SegmentPrefix = cpu.DS;
	cpu.CurrentSegment = &cpu.SegmentPrefix;
	opcode = cpu_pfq_getbyte();
	cpu_optable[opcode]();
	cpu.CurrentSegment = &cpu.DS;
	if( cpu.TF && cpu.IF )
	{
		int_call( 1 );
	}
}

void
_ES()
{
	byte opcode;
	cpu.SegmentPrefix = cpu.ES;
	cpu.CurrentSegment = &cpu.SegmentPrefix;
	opcode = cpu_pfq_getbyte();
	cpu_optable[opcode]();
	cpu.CurrentSegment = &cpu.DS;
	if( cpu.TF && cpu.IF )
	{
		int_call( 1 );
	}
}

void
_SS() {
	byte opcode;
	cpu.SegmentPrefix = cpu.SS;
	cpu.CurrentSegment = &cpu.SegmentPrefix;
	opcode = cpu_pfq_getbyte();
	cpu_optable[opcode]();
	cpu.CurrentSegment = &cpu.DS;
	if( cpu.TF && cpu.IF )
	{
		int_call( 1 );
	}
}

void
_LAHF()
{
	cpu.regs.B.AH = cpu.CF | (cpu.PF * 4) | (cpu.AF * 16) | (cpu.ZF * 64) | (cpu.SF * 128) | 2;
}

void
_SAHF()
{
	cpu.CF = (cpu.regs.B.AH & 0x01) != 0;
	cpu.PF = (cpu.regs.B.AH & 0x04) != 0;
	cpu.AF = (cpu.regs.B.AH & 0x10) != 0;
	cpu.ZF = (cpu.regs.B.AH & 0x40) != 0;
	cpu.SF = (cpu.regs.B.AH & 0x80) != 0;
}

void
_XCHG_AX_reg16()
{
	word tmpax = cpu.regs.W.AX;
	cpu.regs.W.AX = *treg16[cpu_opcode & 7];
	*treg16[cpu_opcode & 7] = tmpax;
}

void
_XCHG_reg8_RM8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	byte tmpreg = *treg8[rmreg(rm)];
	*treg8[rmreg(rm)] = *p;
	*p = tmpreg;
}

void
_XCHG_reg16_RM16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	word tmpreg = *treg16[rmreg(rm)];
	*treg16[rmreg(rm)] = *p;
	*p = tmpreg;
}

void
_DEC_reg16()
{
	dword i = *treg16[cpu_opcode & 7];

	/* Overflow if we'll wrap. */
	cpu.OF = (i == 0);

	i--;
	cpu_setAF(i, *treg16[cpu_opcode & 7], 1);
	cpu_updflags(i, 16);
	--*treg16[cpu_opcode & 7];
}

void
_INC_reg16()
{
	dword i = *treg16[cpu_opcode & 7];

	/* Overflow if we'll wrap. */
	cpu.OF = (i == 32767);

	i++;
	cpu_setAF(i,*treg16[cpu_opcode & 7],1);
	cpu_updflags(i, 16);
	++*treg16[cpu_opcode & 7];
}

void
_INC_RM16()
{
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	dword i = *p;

	/* Overflow if we'll wrap. */
	cpu.OF = (i == 32767);

	i++;
	cpu_setAF(i,*p,1);
	cpu_updflags(i, 16);
	++*p;
}

void
_DEC_RM16()
{
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	dword i = *p;

	/* Overflow if we'll wrap. */
	cpu.OF = (i == 0);

	i--;
	cpu_setAF(i,*p,1);
	cpu_updflags(i, 16);
	--*p;
}

word
signext (byte b) {
	word w = 0x0000 + b;
	if ((w&0x80)>0)
		return (w | 0xff00);
	else
		return (w & 0x00ff);
}

dword
signext32 (word w) {
	dword d = 0x00000000 + w;
	if ((d&0x8000)>0)
		return (d | 0xffff0000);
	else
		return (d & 0x0000ffff);
}

