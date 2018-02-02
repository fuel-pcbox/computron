/*
 * Copyright (C) 2003-2018 Andreas Kling <awesomekling@gmail.com>
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

void VCpu::_JCXZ_imm8()
{
    SIGNED_BYTE imm = fetchOpcodeByte();
    if (getCX() == 0)
        jumpRelative8(imm);
}

void VCpu::_JECXZ_imm8()
{
    SIGNED_BYTE imm = fetchOpcodeByte();
    if (getECX() == 0)
        jumpRelative8(imm);
}

void VCpu::_JMP_imm16()
{
    SIGNED_WORD imm = fetchOpcodeWord();
    jumpRelative16(imm);
}

void VCpu::_JMP_imm32()
{
    SIGNED_DWORD imm = fetchOpcodeDWord();
    jumpRelative32(imm);
}

void VCpu::_JMP_imm16_imm16()
{
    WORD newIP = fetchOpcodeWord();
    WORD newCS = fetchOpcodeWord();
    jump16(newCS, newIP);
}

void VCpu::_JMP_imm16_imm32()
{
    DWORD newEIP = fetchOpcodeDWord();
    WORD newCS = fetchOpcodeWord();
    jump32(newCS, newEIP);
}

void VCpu::_JMP_short_imm8()
{
    SIGNED_BYTE imm = fetchOpcodeByte();
    jumpRelative8(imm);
}

void VCpu::_JMP_RM16()
{
    jumpAbsolute16(readModRM16(rmbyte));
}

void VCpu::_JMP_RM32()
{
    jumpAbsolute32(readModRM32(rmbyte));
}

void VCpu::_JMP_FAR_mem16()
{
    WORD* ptr = static_cast<WORD*>(resolveModRM8(rmbyte).memoryPointer());
    jump16(ptr[1], ptr[0]);
}

void VCpu::_JMP_FAR_mem32()
{
    // FIXME: Implement!
    VM_ASSERT(false);
}

#define DO_JCC_imm(name, condition) \
void VCpu::_ ## name ## _imm8() { \
    SIGNED_BYTE imm = fetchOpcodeByte(); \
    if ((condition)) \
        jumpRelative8(imm); \
} \
void VCpu::_ ## name ## _NEAR_imm() { \
    if (a16()) { \
        SIGNED_WORD imm = fetchOpcodeWord(); \
        if ((condition)) \
            jumpRelative16(imm); \
    } else { \
        SIGNED_DWORD imm = fetchOpcodeDWord(); \
        if ((condition)) \
            jumpRelative32(imm); \
    } \
}

DO_JCC_imm(JO,   getOF())
DO_JCC_imm(JNO,  !getOF())
DO_JCC_imm(JC,   getCF())
DO_JCC_imm(JNC,  !getCF())
DO_JCC_imm(JZ,   getZF())
DO_JCC_imm(JNZ,  !getZF())
DO_JCC_imm(JNA,  getCF() | getZF())
DO_JCC_imm(JA,   !(getCF() | getZF()))
DO_JCC_imm(JS,   getSF())
DO_JCC_imm(JNS,  !getSF())
DO_JCC_imm(JP,   getPF())
DO_JCC_imm(JNP,  !getPF())
DO_JCC_imm(JL,   getSF() ^ getOF())
DO_JCC_imm(JNL,  !(getSF() ^ getOF()))
DO_JCC_imm(JNG,  (getSF() ^ getOF()) | getZF())
DO_JCC_imm(JG,   !((getSF() ^ getOF()) | getZF()))

void VCpu::_CALL_imm16()
{
    SIGNED_WORD imm = fetchOpcodeWord();
    pushInstructionPointer();
    jumpRelative16(imm);
}

void VCpu::_CALL_imm32()
{
    SIGNED_DWORD imm = fetchOpcodeDWord();
    pushInstructionPointer();
    jumpRelative32(imm);
}

void VCpu::_CALL_imm16_imm16()
{
    WORD newip = fetchOpcodeWord();
    WORD segment = fetchOpcodeWord();
    push(getCS());
    pushInstructionPointer();
    jump16(segment, newip);
}

void VCpu::_CALL_imm16_imm32()
{
    DWORD neweip = fetchOpcodeDWord();
    WORD segment = fetchOpcodeWord();
    push(getCS());
    pushInstructionPointer();
    jump32(segment, neweip);
}

void VCpu::_CALL_FAR_mem16()
{
    WORD* ptr = static_cast<WORD*>(resolveModRM8(rmbyte).memoryPointer());
    push(getCS());
    pushInstructionPointer();
    jump16(ptr[1], ptr[0]);
}

void VCpu::_CALL_FAR_mem32()
{
    // FIXME: Implement!
    VM_ASSERT(false);
}

void VCpu::_CALL_RM16()
{
    WORD value = readModRM16(rmbyte);
    pushInstructionPointer();
    jumpAbsolute16(value);
}

void VCpu::_CALL_RM32()
{
    DWORD value = readModRM32(rmbyte);
    pushInstructionPointer();
    jumpAbsolute32(value);
}

void VCpu::_RET()
{
    if (x32())
        jumpAbsolute32(pop32());
    else
        jumpAbsolute16(pop());
}

void VCpu::_RET_imm16()
{
    if (x32()) {
        WORD imm = fetchOpcodeWord();
        jumpAbsolute32(pop32());
        regs.D.ESP += imm;
    } else {
        WORD imm = fetchOpcodeWord();
        jumpAbsolute16(pop());
        regs.W.SP += imm;
    }
}

void VCpu::_RETF()
{
    if (x32()) {
        DWORD nip = pop32();
        jump32(pop(), nip);
    } else {
        WORD nip = pop();
        jump16(pop(), nip);
    }
}

void VCpu::_RETF_imm16()
{
    if (x32()) {
        DWORD nip = pop32();
        WORD imm = fetchOpcodeWord();
        jump32(pop(), nip);
        regs.D.ESP += imm;
    } else {
        WORD nip = pop();
        WORD imm = fetchOpcodeWord();
        jump16(pop(), nip);
        regs.W.SP += imm;
    }
}
