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

void VCpu::_JCXZ_imm8(Instruction& insn)
{
    SIGNED_BYTE imm = insn.imm8();
    bool shouldJump;
    if (a16())
        shouldJump = getCX() == 0;
    else
        shouldJump = getECX() == 0;
    if (shouldJump)
        jumpRelative8(imm);
}

void VCpu::_JMP_imm16(Instruction& insn)
{
    SIGNED_WORD imm = insn.imm16();
    jumpRelative16(imm);
}

void VCpu::_JMP_imm32(Instruction& insn)
{
    SIGNED_DWORD imm = insn.imm32();
    jumpRelative32(imm);
}

void VCpu::_JMP_imm16_imm16(Instruction& insn)
{
    WORD newCS = insn.imm16_1();
    WORD newIP = insn.imm16_2();
    jump16(newCS, newIP);
}

void VCpu::_JMP_imm16_imm32(Instruction& insn)
{
    WORD newCS = insn.imm16_1();
    DWORD newEIP = insn.imm32_2();
    jump32(newCS, newEIP);
}

void VCpu::_JMP_short_imm8(Instruction& insn)
{
    SIGNED_BYTE imm = insn.imm8();
    jumpRelative8(imm);
}

void VCpu::_JMP_RM16(Instruction& insn)
{
    jumpAbsolute16(insn.location().read16());
}

void VCpu::_JMP_RM32(Instruction& insn)
{
    jumpAbsolute32(insn.location().read32());
}

void VCpu::_JMP_FAR_mem16(Instruction& insn)
{
    WORD* ptr = static_cast<WORD*>(insn.location().memoryPointer());
    jump16(ptr[1], ptr[0]);
}

void VCpu::_JMP_FAR_mem32(Instruction&)
{
    // FIXME: Implement!
    VM_ASSERT(false);
}

#define DO_JCC_imm(name, condition) \
void VCpu::_ ## name ## _imm8(Instruction& insn) { \
    SIGNED_BYTE imm = insn.imm8(); \
    if ((condition)) \
        jumpRelative8(imm); \
} \
void VCpu::_ ## name ## _NEAR_imm(Instruction& insn) { \
    if (a16()) { \
        SIGNED_WORD imm = insn.imm16(); \
        if ((condition)) \
            jumpRelative16(imm); \
    } else { \
        SIGNED_DWORD imm = insn.imm32(); \
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

void VCpu::_CALL_imm16(Instruction& insn)
{
    SIGNED_WORD imm = insn.imm16();
    pushInstructionPointer();
    jumpRelative16(imm);
}

void VCpu::_CALL_imm32(Instruction& insn)
{
    SIGNED_DWORD imm = insn.imm32();
    pushInstructionPointer();
    jumpRelative32(imm);
}

void VCpu::_CALL_imm16_imm16(Instruction& insn)
{
    push(getCS());
    pushInstructionPointer();
    jump16(insn.imm16_1(), insn.imm16_2());
}

void VCpu::_CALL_imm16_imm32(Instruction& insn)
{
    push(getCS());
    pushInstructionPointer();
    jump32(insn.imm16_1(), insn.imm32_2());
}

void VCpu::_CALL_FAR_mem16(Instruction& insn)
{
    WORD* ptr = static_cast<WORD*>(insn.location().memoryPointer());
    push(getCS());
    pushInstructionPointer();
    jump16(ptr[1], ptr[0]);
}

void VCpu::_CALL_FAR_mem32(Instruction&)
{
    // FIXME: Implement!
    VM_ASSERT(false);
}

void VCpu::_CALL_RM16(Instruction& insn)
{
    pushInstructionPointer();
    jumpAbsolute16(insn.location().read16());
}

void VCpu::_CALL_RM32(Instruction& insn)
{
    pushInstructionPointer();
    jumpAbsolute32(insn.location().read32());
}

void VCpu::_RET(Instruction&)
{
    if (o32())
        jumpAbsolute32(pop32());
    else
        jumpAbsolute16(pop());
}

void VCpu::_RET_imm16(Instruction& insn)
{
    if (o32()) {
        jumpAbsolute32(pop32());
        regs.D.ESP += insn.imm16();
    } else {
        jumpAbsolute16(pop());
        regs.W.SP += insn.imm16();
    }
}

void VCpu::_RETF(Instruction&)
{
    if (o32()) {
        DWORD nip = pop32();
        jump32(pop(), nip);
    } else {
        WORD nip = pop();
        jump16(pop(), nip);
    }
}

void VCpu::_RETF_imm16(Instruction& insn)
{
    if (o32()) {
        DWORD nip = pop32();
        jump32(pop(), nip);
        regs.D.ESP += insn.imm16();
    } else {
        WORD nip = pop();
        jump16(pop(), nip);
        regs.W.SP += insn.imm16();
    }
}
