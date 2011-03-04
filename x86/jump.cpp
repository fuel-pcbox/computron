/*
 * Copyright (C) 2003-2011 Andreas Kling <kling@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ANDREAS KLING ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANDREAS KLING OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "vcpu.h"

void _JCXZ_imm8(VCpu* cpu)
{
    SIGNED_BYTE imm = cpu->fetchOpcodeByte();
    if (cpu->getCX() == 0)
        cpu->jumpRelative8(imm);
}

void _JECXZ_imm8(VCpu* cpu)
{
    SIGNED_BYTE imm = cpu->fetchOpcodeByte();
    if (cpu->getECX() == 0)
        cpu->jumpRelative8(imm);
}

void _JMP_imm16(VCpu* cpu)
{
    SIGNED_WORD imm = cpu->fetchOpcodeWord();
    cpu->jumpRelative16(imm);
}

void _JMP_imm32(VCpu* cpu)
{
    SIGNED_DWORD imm = cpu->fetchOpcodeDWord();
    cpu->jumpRelative32(imm);
}

void _JMP_imm16_imm16(VCpu* cpu)
{
    WORD newIP = cpu->fetchOpcodeWord();
    WORD newCS = cpu->fetchOpcodeWord();
    cpu->jump16(newCS, newIP);
}

void _JMP_imm16_imm32(VCpu* cpu)
{
    DWORD newEIP = cpu->fetchOpcodeDWord();
    WORD newCS = cpu->fetchOpcodeWord();
    cpu->jump32(newCS, newEIP);
}

void _JMP_short_imm8(VCpu* cpu)
{
    SIGNED_BYTE imm = cpu->fetchOpcodeByte();
    cpu->jumpRelative8(imm);
}

void _JMP_RM16(VCpu* cpu)
{
    cpu->jumpAbsolute16(cpu->readModRM16(cpu->rmbyte));
}

void _JMP_FAR_mem16(VCpu* cpu)
{
    WORD* ptr = static_cast<WORD*>(cpu->resolveModRM8(cpu->rmbyte));
    cpu->jump16(ptr[1], ptr[0]);
}

#define DO_JCC_imm(name, condition) \
void _ ## name ## _imm8(VCpu* cpu) { \
    SIGNED_BYTE imm = cpu->fetchOpcodeByte(); \
    if ((condition)) \
        cpu->jumpRelative8(imm); \
} \
void _ ## name ## _NEAR_imm(VCpu* cpu) { \
    if (cpu->a16()) { \
        SIGNED_WORD imm = cpu->fetchOpcodeWord(); \
        if ((condition)) \
            cpu->jumpRelative16(imm); \
    } else { \
        SIGNED_DWORD imm = cpu->fetchOpcodeDWord(); \
        if ((condition)) \
            cpu->jumpRelative32(imm); \
    } \
}

DO_JCC_imm(JO,   cpu->getOF())
DO_JCC_imm(JNO,  !cpu->getOF())
DO_JCC_imm(JC,   cpu->getCF())
DO_JCC_imm(JNC,  !cpu->getCF())
DO_JCC_imm(JZ,   cpu->getZF())
DO_JCC_imm(JNZ,  !cpu->getZF())
DO_JCC_imm(JNA,  cpu->getCF() | cpu->getZF())
DO_JCC_imm(JA,   !(cpu->getCF() | cpu->getZF()))
DO_JCC_imm(JS,   cpu->getSF())
DO_JCC_imm(JNS,  !cpu->getSF())
DO_JCC_imm(JP,   cpu->getPF())
DO_JCC_imm(JNP,  !cpu->getPF())
DO_JCC_imm(JL,   cpu->getSF() ^ cpu->getOF())
DO_JCC_imm(JNL,  !(cpu->getSF() ^ cpu->getOF()))
DO_JCC_imm(JNG,  (cpu->getSF() ^ cpu->getOF()) | cpu->getZF())
DO_JCC_imm(JG,   !((cpu->getSF() ^ cpu->getOF()) | cpu->getZF()))

void _CALL_imm16(VCpu* cpu)
{
    SIGNED_WORD imm = cpu->fetchOpcodeWord();
    cpu->push(cpu->IP);
    cpu->jumpRelative16(imm);
}

void _CALL_imm32(VCpu* cpu)
{
    SIGNED_DWORD imm = cpu->fetchOpcodeWord();
    cpu->push(cpu->EIP);
    cpu->jumpRelative32(imm);
}

void _CALL_imm16_imm16(VCpu* cpu)
{
    WORD newip = cpu->fetchOpcodeWord();
    WORD segment = cpu->fetchOpcodeWord();
    cpu->push(cpu->getCS());
    cpu->push(cpu->getIP());
    cpu->jump16(segment, newip);
}

void _CALL_imm16_imm32(VCpu* cpu)
{
    DWORD neweip = cpu->fetchOpcodeDWord();
    WORD segment = cpu->fetchOpcodeWord();
    cpu->push(cpu->getCS());
    cpu->push(cpu->getEIP());
    cpu->jump16(segment, neweip);
}

void _CALL_FAR_mem16(VCpu* cpu)
{
    WORD* ptr = static_cast<WORD*>(cpu->resolveModRM8(cpu->rmbyte));
    cpu->push(cpu->getCS());
    cpu->push(cpu->getIP());
    cpu->jump16(ptr[1], ptr[0]);
}

void _CALL_RM16(VCpu* cpu)
{
    WORD value = cpu->readModRM16(cpu->rmbyte);
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
    cpu->jump16(cpu->pop(), nip);
}

void _RETF_imm16(VCpu* cpu)
{
    WORD nip = cpu->pop();
    WORD imm = cpu->fetchOpcodeWord();
    cpu->jump16(cpu->pop(), nip);
    cpu->regs.W.SP += imm;
}
