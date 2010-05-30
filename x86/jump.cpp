// x86/jump.cpp
// Jump, call & return instructions

#include "vcpu.h"

void _JCXZ_imm8(VCpu* cpu)
{
    SIGNED_BYTE imm = cpu->fetchOpcodeByte();
    if (cpu->regs.W.CX == 0)
        cpu->jumpRelative8(imm);
}

void _JMP_imm16(VCpu* cpu)
{
    SIGNED_WORD imm = cpu->fetchOpcodeWord();
    cpu->jumpRelative16(imm);
}

void _JMP_imm16_imm16(VCpu* cpu)
{
    WORD newIP = cpu->fetchOpcodeWord();
    WORD newCS = cpu->fetchOpcodeWord();
    cpu->jump(newCS, newIP);
}

void _JMP_short_imm8(VCpu* cpu)
{
    SIGNED_BYTE imm = cpu->fetchOpcodeByte();
    cpu->jumpRelative8(imm);
}

void _JMP_RM16(VCpu* cpu)
{
    cpu->jumpAbsolute16(vomit_cpu_modrm_read16(cpu, cpu->rmbyte));
}

void _JMP_FAR_mem16(VCpu* cpu)
{
    DWORD value = vomit_cpu_modrm_read32(cpu, cpu->rmbyte);
    cpu->jump(MSW(value), LSW(value));
}

#define DO_JCC_imm8(name, condition) \
void _ ## name ## _imm8(VCpu* cpu) { \
    SIGNED_BYTE imm = cpu->fetchOpcodeByte(); \
    if ((condition)) \
        cpu->jumpRelative8(imm); \
}

DO_JCC_imm8(JO,   cpu->getOF())
DO_JCC_imm8(JNO,  !cpu->getOF())
DO_JCC_imm8(JC,   cpu->getCF())
DO_JCC_imm8(JNC,  !cpu->getCF())
DO_JCC_imm8(JZ,   cpu->getZF())
DO_JCC_imm8(JNZ,  !cpu->getZF())
DO_JCC_imm8(JNA,  cpu->getCF() | cpu->getZF())
DO_JCC_imm8(JA,   !(cpu->getCF() | cpu->getZF()))
DO_JCC_imm8(JS,   cpu->getSF())
DO_JCC_imm8(JNS,  !cpu->getSF())
DO_JCC_imm8(JP,   cpu->getPF())
DO_JCC_imm8(JNP,  !cpu->getPF())
DO_JCC_imm8(JL,   cpu->getSF() ^ cpu->getOF())
DO_JCC_imm8(JNL,  !(cpu->getSF() ^ cpu->getOF()))
DO_JCC_imm8(JNG,  (cpu->getSF() ^ cpu->getOF()) | cpu->getZF())
DO_JCC_imm8(JG,   !((cpu->getSF() ^ cpu->getOF()) | cpu->getZF()))

void _CALL_imm16(VCpu* cpu)
{
    SIGNED_WORD imm = cpu->fetchOpcodeWord();
    cpu->push(cpu->IP);
    cpu->jumpRelative16(imm);
}

void _CALL_imm16_imm16(VCpu* cpu)
{
    WORD newip = cpu->fetchOpcodeWord();
    WORD segment = cpu->fetchOpcodeWord();
    cpu->push(cpu->getCS());
    cpu->push(cpu->getIP());
    cpu->jump(segment, newip);
}

void _CALL_FAR_mem16(VCpu* cpu)
{
    DWORD value = vomit_cpu_modrm_read32(cpu, cpu->rmbyte);
    cpu->push(cpu->getCS());
    cpu->push(cpu->getIP());
    cpu->jump(MSW(value), LSW(value));
}

void _CALL_RM16(VCpu* cpu)
{
    WORD value = vomit_cpu_modrm_read16(cpu, cpu->rmbyte);
    cpu->push(cpu->getIP());
    cpu->jumpAbsolute16(value);
}

void _RET(VCpu* cpu)
{
    cpu->jumpAbsolute16(cpu->pop());
}

void _RET_imm16(VCpu* cpu)
{
    WORD imm = cpu->fetchOpcodeWord();
    cpu->jumpAbsolute16(cpu->pop());
    cpu->regs.W.SP += imm;
}

void _RETF(VCpu* cpu)
{
    WORD nip = cpu->pop();
    cpu->jump(cpu->pop(), nip);
}

void _RETF_imm16(VCpu* cpu)
{
    WORD nip = cpu->pop();
    WORD imm = cpu->fetchOpcodeWord();
    cpu->jump(cpu->pop(), nip);
    cpu->regs.W.SP += imm;
}
