/* 8086/jump.c
 * Jump instructions
 *
 */

#include "vomit.h"

void
_JCXZ_imm8()
{
	sigbyte imm = cpu_pfq_getbyte();

	if( CX == 0 )
	{
		cpu_jump_relative8( imm );
	}
}

void
_JMP_imm16()
{
	sigword imm = cpu_pfq_getword();
	cpu_jump_relative16( imm );
}

void
_JMP_imm16_imm16()
{
	word newip = cpu_pfq_getword();
	cpu_jump( cpu_pfq_getword(), newip );
}

void
_JMP_short_imm8()
{
	sigbyte imm = cpu_pfq_getbyte();
	cpu_jump_relative8( imm);
}

void
_JMP_RM16()
{
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	cpu_jump_absolute16( *p );
}

void
_JMP_FAR_mem16()
{
	byte rm = cpu_rmbyte;
	word nip, ncs;
	word *p = cpu_rmptr(rm, 16);
	nip = *(p++);
	ncs = *p;
	cpu_jump(ncs, nip);
}

void
_Jcc_imm8()
{
	sigbyte imm = cpu_pfq_getbyte();
	if( cpu_evaluate( cpu_opcode & 0x0F ))
	{
		cpu_jump_relative8( imm );
	}
}

void
_CALL_imm16()
{
	sigword imm = cpu_pfq_getword();
	mem_push( IP );
	cpu_jump_relative16( imm );
}

void
_CALL_imm16_imm16()
{
	word newip = cpu_pfq_getword();
	word segment = cpu_pfq_getword();
	mem_push(CS);
	mem_push(IP);
	cpu_jump(segment, newip);
}

void
_CALL_FAR_mem16()
{
	word nip, ncs;
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	nip = *(p++);
	ncs = *p;
	mem_push(CS);
	mem_push(IP);
	cpu_jump(ncs, nip);
}

void
_CALL_RM16()
{
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	mem_push(IP);
	cpu_jump_absolute16( *p );
}

void
_RET()
{
	cpu_jump_absolute16( mem_pop() );
}

void
_RET_imm16()
{
	word imm = cpu_pfq_getword();
	cpu_jump_absolute16( mem_pop() );
	StackPointer += imm;
}

void
_RETF()
{
	word nip = mem_pop();
	cpu_jump(mem_pop(), nip);
}

void
_RETF_imm16()
{
	word nip = mem_pop();
	word imm = cpu_pfq_getword();
	cpu_jump(mem_pop(), nip);
	StackPointer += imm;
}
