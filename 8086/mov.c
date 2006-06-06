/* 8086/mov.c
 * MOVe instructions
 *
 */

#include "vomit.h"
#include <assert.h>

void
_MOV_RM8_imm8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	*p = cpu_pfq_getbyte();
}

void
_MOV_RM16_imm16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	*p = cpu_pfq_getword();
}

void
_MOV_RM16_seg()
{
	byte rm  = cpu_pfq_getbyte();
	word *p = cpu_rmptr( rm, 16 );
	assert( rmreg(rm) >= 0 && rmreg(rm) <= 5 );
	*p = *tseg[rmreg(rm)];

	if( rmreg(rm) == REG_FS || rmreg(rm) == REG_GS )
	{
		vlog( VM_CPUMSG, "%04X:%04X: Read from 80386 segment register" );
	}
}

void
_MOV_seg_RM16()
{
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr( rm, 16 );
	assert( rmreg(rm) >= 0 && rmreg(rm) <= 5 );
	*tseg[rmreg(rm)] = *p;

	if( rmreg(rm) == REG_FS || rmreg(rm) == REG_GS )
	{
		vlog( VM_CPUMSG, "%04X:%04X: Write to 80386 segment register" );
	}
}

void
_MOV_RM8_reg8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	*p = *treg8[rmreg(rm)];
}

void
_MOV_reg8_RM8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	*treg8[rmreg(rm)] = *p;
}

void
_MOV_RM16_reg16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	*p = *treg16[rmreg(rm)];
}

void
_MOV_reg16_RM16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	*treg16[rmreg(rm)] = *p;
}

void _MOV_reg8_imm8() { *treg8[cpu_opcode&7] = cpu_pfq_getbyte(); }
void _MOV_reg16_imm16() { *treg16[cpu_opcode&7] = cpu_pfq_getword(); }
void _MOV_AL_moff8() { cpu.regs.B.AL = mem_getbyte( *(cpu.CurrentSegment), cpu_pfq_getword() ); }
void _MOV_AX_moff16() { cpu.regs.W.AX = mem_getword( *(cpu.CurrentSegment), cpu_pfq_getword() ); }
void _MOV_moff8_AL() { mem_setbyte( *(cpu.CurrentSegment), cpu_pfq_getword(), cpu.regs.B.AL ); }
void _MOV_moff16_AX() { mem_setword( *(cpu.CurrentSegment), cpu_pfq_getword(), cpu.regs.W.AX ); }

