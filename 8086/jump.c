/* 8086/jump.c
 * Jump instructions
 *
 */

#include "vomit.h"

void
_JCXZ_imm8()
{
	sigbyte imm = cpu_pfq_getbyte();

	if( cpu.regs.W.CX == 0 )
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
	cpu_jump_relative8( imm );
}

void
_JMP_RM16()
{
	cpu_jump_absolute16( modrm_read16( cpu.rmbyte ));
}

void
_JMP_FAR_mem16()
{
	dword value = modrm_read32( cpu.rmbyte );
	cpu_jump( MSW(value), LSW(value) );
}

#define DO_JCC_imm8(name, condition) \
void _ ## name ## _imm8() { \
	sigbyte imm = cpu_pfq_getbyte(); \
	if( (condition) ) \
		cpu_jump_relative8( imm ); \
}

DO_JCC_imm8( JO,   cpu.OF )
DO_JCC_imm8( JNO,  !cpu.OF )
DO_JCC_imm8( JC,   cpu.CF )
DO_JCC_imm8( JNC,  !cpu.CF )
DO_JCC_imm8( JZ,   cpu.ZF )
DO_JCC_imm8( JNZ,  !cpu.ZF )
DO_JCC_imm8( JNA,  cpu.CF | cpu.ZF )
DO_JCC_imm8( JA,   !(cpu.CF | cpu.ZF) )
DO_JCC_imm8( JS,   cpu.SF )
DO_JCC_imm8( JNS,  !cpu.SF )
DO_JCC_imm8( JP,   cpu.PF )
DO_JCC_imm8( JNP,  !cpu.PF )
DO_JCC_imm8( JL,   cpu.SF ^ cpu.OF )
DO_JCC_imm8( JNL,  !(cpu.SF ^ cpu.OF) )
DO_JCC_imm8( JNG,  (cpu.SF ^ cpu.OF) | cpu.ZF )
DO_JCC_imm8( JG,   !((cpu.SF ^ cpu.OF) | cpu.ZF) )

void
_CALL_imm16()
{
	sigword imm = cpu_pfq_getword();
	mem_push( cpu.IP );
	cpu_jump_relative16( imm );
}

void
_CALL_imm16_imm16()
{
	word newip = cpu_pfq_getword();
	word segment = cpu_pfq_getword();
	mem_push( cpu.CS );
	mem_push( cpu.IP );
	cpu_jump(segment, newip);
}

void
_CALL_FAR_mem16()
{
	dword value = modrm_read32( cpu.rmbyte );
	mem_push( cpu.CS );
	mem_push( cpu.IP );
	cpu_jump( MSW(value), LSW(value) );
}

void
_CALL_RM16()
{
	word value = modrm_read16( cpu.rmbyte );
	mem_push( cpu.IP );
	cpu_jump_absolute16( value );
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
	cpu.regs.W.SP += imm;
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
	cpu.regs.W.SP += imm;
}
