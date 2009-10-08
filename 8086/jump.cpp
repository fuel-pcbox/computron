/* 8086/jump.cpp
 * Jump instructions
 *
 */

#include "vomit.h"
#include <stdio.h>

void _JCXZ_imm8(vomit_cpu_t *cpu)
{
    SIGNED_BYTE imm = vomit_cpu_pfq_getbyte(cpu);

    if (cpu->regs.W.CX == 0) {
        vomit_cpu_jump_relative8(cpu, imm);
    }
}

void _JMP_imm16(vomit_cpu_t *cpu)
{
    SIGNED_WORD imm = vomit_cpu_pfq_getword(cpu);
    vomit_cpu_jump_relative16(cpu, imm);
}

void _JMP_imm16_imm16(vomit_cpu_t *cpu)
{
    WORD newIP = vomit_cpu_pfq_getword(cpu);
    WORD newCS = vomit_cpu_pfq_getword(cpu);
    vomit_cpu_jump(cpu, newCS, newIP);
}

void _JMP_short_imm8(vomit_cpu_t *cpu)
{
    SIGNED_BYTE imm = vomit_cpu_pfq_getbyte(cpu);
    vomit_cpu_jump_relative8(cpu, imm);
}

void _JMP_RM16(vomit_cpu_t *cpu)
{
    vomit_cpu_jump_absolute16(cpu, vomit_cpu_modrm_read16(cpu, cpu->rmbyte));
}

void _JMP_FAR_mem16(vomit_cpu_t *cpu)
{
    DWORD value = vomit_cpu_modrm_read32(cpu, cpu->rmbyte);
    vomit_cpu_jump(cpu, MSW(value), LSW(value));
}

#define DO_JCC_imm8(name, condition) \
void _ ## name ## _imm8(vomit_cpu_t *cpu) { \
	SIGNED_BYTE imm = vomit_cpu_pfq_getbyte(cpu); \
	if( (condition) ) \
		vomit_cpu_jump_relative8(cpu, imm); \
}

DO_JCC_imm8( JO,   cpu->OF )
DO_JCC_imm8( JNO,  !cpu->OF )
DO_JCC_imm8( JC,   cpu->CF )
DO_JCC_imm8( JNC,  !cpu->CF )
DO_JCC_imm8( JZ,   cpu->ZF )
DO_JCC_imm8( JNZ,  !cpu->ZF )
DO_JCC_imm8( JNA,  cpu->CF | cpu->ZF )
DO_JCC_imm8( JA,   !(cpu->CF | cpu->ZF) )
DO_JCC_imm8( JS,   cpu->SF )
DO_JCC_imm8( JNS,  !cpu->SF )
DO_JCC_imm8( JP,   cpu->PF )
DO_JCC_imm8( JNP,  !cpu->PF )
DO_JCC_imm8( JL,   cpu->SF ^ cpu->OF )
DO_JCC_imm8( JNL,  !(cpu->SF ^ cpu->OF) )
DO_JCC_imm8( JNG,  (cpu->SF ^ cpu->OF) | cpu->ZF )
DO_JCC_imm8( JG,   !((cpu->SF ^ cpu->OF) | cpu->ZF) )

void _CALL_imm16(vomit_cpu_t *cpu)
{
    SIGNED_WORD imm = vomit_cpu_pfq_getword(cpu);
    vomit_cpu_push(cpu, cpu->IP);
    vomit_cpu_jump_relative16(cpu, imm);
}

void _CALL_imm16_imm16(vomit_cpu_t *cpu)
{
    WORD newip = vomit_cpu_pfq_getword(cpu);
    WORD segment = vomit_cpu_pfq_getword(cpu);
    vomit_cpu_push(cpu, cpu->CS);
    vomit_cpu_push(cpu, cpu->IP);
    vomit_cpu_jump(cpu, segment, newip);
}

void _CALL_FAR_mem16(vomit_cpu_t *cpu)
{
    DWORD value = vomit_cpu_modrm_read32(cpu, cpu->rmbyte);
    vomit_cpu_push(cpu, cpu->CS);
    vomit_cpu_push(cpu, cpu->IP);
    vomit_cpu_jump(cpu, MSW(value), LSW(value));
}

void _CALL_RM16(vomit_cpu_t *cpu)
{
    WORD value = vomit_cpu_modrm_read16(cpu, cpu->rmbyte);
    vomit_cpu_push(cpu, cpu->IP);
    vomit_cpu_jump_absolute16(cpu, value);
}

void _RET(vomit_cpu_t *cpu)
{
    vomit_cpu_jump_absolute16(cpu, vomit_cpu_pop(cpu));
}

void _RET_imm16(vomit_cpu_t *cpu)
{
    WORD imm = vomit_cpu_pfq_getword(cpu);
    vomit_cpu_jump_absolute16(cpu, vomit_cpu_pop(cpu));
    cpu->regs.W.SP += imm;
}

void _RETF(vomit_cpu_t *cpu)
{
    WORD nip = vomit_cpu_pop(cpu);
    vomit_cpu_jump(cpu, vomit_cpu_pop(cpu), nip);
}

void _RETF_imm16(vomit_cpu_t *cpu)
{
    WORD nip = vomit_cpu_pop(cpu);
    WORD imm = vomit_cpu_pfq_getword(cpu);
    vomit_cpu_jump(cpu, vomit_cpu_pop(cpu), nip);
    cpu->regs.W.SP += imm;
}
