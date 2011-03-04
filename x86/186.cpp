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

#include "vomit.h"
#include "debug.h"

void _wrap_0x0F(VCpu* cpu)
{
    BYTE op = cpu->fetchOpcodeByte();
    switch (op) {
    case 0x01:
    {
        cpu->rmbyte = cpu->fetchOpcodeByte();
        switch (vomit_modRMRegisterPart(cpu->rmbyte)) {
        case 0: _SGDT(cpu); return;
        }
        (void) cpu->readModRM16(cpu->rmbyte);
        vlog(VM_ALERT, "Sliding by 0F 01 /%d\n", vomit_modRMRegisterPart(cpu->rmbyte));
        break;
    }
    case 0xFF: // UD0
    case 0xB9: // UD1
    case 0x0B: // UD2
        vlog(VM_ALERT, "Undefined opcode 0F %02X", op);
        cpu->exception(6);
        break;
    default:
        vlog(VM_CPUMSG, "_wrap_0x0F passing opcode to VCpu::decodeNext(): 0F %02X", op);
        cpu->EIP -= 2;
        cpu->decodeNext();
        break;
    }
}

void _BOUND(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    WORD* ptr = static_cast<WORD*>(cpu->resolveModRM8(rm));
    WORD index = *cpu->treg16[vomit_modRMRegisterPart(rm)];

    if (index < ptr[0] || index > ptr[1]) {
        /* Raise BR exception */
        cpu->exception(5);
    }
}

void _PUSH_imm8(VCpu* cpu)
{
    cpu->push(vomit_signExtend(cpu->fetchOpcodeByte()));
}

void _PUSH_imm16(VCpu* cpu)
{
    cpu->push(cpu->fetchOpcodeWord());
}

void _ENTER(VCpu* cpu)
{
    WORD Size = cpu->fetchOpcodeWord();
    BYTE NestingLevel = cpu->fetchOpcodeByte() % 32;
    WORD FrameTemp;
    cpu->push(cpu->regs.W.BP);
    FrameTemp = cpu->regs.W.SP;
    if (NestingLevel != 0) {
        for (WORD i = 1; i <= (NestingLevel - 1); ++i) {
            cpu->regs.W.BP -= 2;
            cpu->push(cpu->readMemory16(cpu->SS, cpu->regs.W.BP));
        }
    }
    cpu->push(FrameTemp);
    cpu->regs.W.BP = FrameTemp;
    cpu->regs.W.SP = cpu->regs.W.BP - Size;
}

void _LEAVE(VCpu* cpu)
{
    cpu->regs.W.SP = cpu->regs.W.BP;
    cpu->regs.W.BP = cpu->pop();
}

void _PUSHA(VCpu* cpu)
{
    WORD oldsp = cpu->regs.W.SP;
    cpu->push(cpu->regs.W.AX);
    cpu->push(cpu->regs.W.BX);
    cpu->push(cpu->regs.W.CX);
    cpu->push(cpu->regs.W.DX);
    cpu->push(cpu->regs.W.BP);
    cpu->push(oldsp);
    cpu->push(cpu->regs.W.SI);
    cpu->push(cpu->regs.W.DI);
}

void _PUSHAD(VCpu* cpu)
{
    DWORD oldesp = cpu->regs.D.ESP;
    cpu->push32(cpu->regs.D.EAX);
    cpu->push32(cpu->regs.D.EBX);
    cpu->push32(cpu->regs.D.ECX);
    cpu->push32(cpu->regs.D.EDX);
    cpu->push32(cpu->regs.D.EBP);
    cpu->push32(oldesp);
    cpu->push32(cpu->regs.D.ESI);
    cpu->push32(cpu->regs.D.EDI);
}

void _POPA(VCpu* cpu)
{
    cpu->regs.W.DI = cpu->pop();
    cpu->regs.W.SI = cpu->pop();
    (void) cpu->pop();
    cpu->regs.W.BP = cpu->pop();
    cpu->regs.W.DX = cpu->pop();
    cpu->regs.W.CX = cpu->pop();
    cpu->regs.W.BX = cpu->pop();
    cpu->regs.W.AX = cpu->pop();
}

void _POPAD(VCpu* cpu)
{
    cpu->regs.D.EDI = cpu->pop32();
    cpu->regs.D.ESI = cpu->pop32();
    (void) cpu->pop32();
    cpu->regs.D.EBP = cpu->pop32();
    cpu->regs.D.EDX = cpu->pop32();
    cpu->regs.D.ECX = cpu->pop32();
    cpu->regs.D.EBX = cpu->pop32();
    cpu->regs.D.EAX = cpu->pop32();
}
