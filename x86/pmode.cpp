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

void _SGDT(VCpu* cpu)
{
    WORD tableAddress = cpu->fetchOpcodeWord();
    cpu->writeMemory32(cpu->currentSegment(), tableAddress + 2, cpu->GDTR.base);
    cpu->writeMemory16(cpu->currentSegment(), tableAddress, cpu->GDTR.limit);
}

void _SIDT(VCpu* cpu)
{
    WORD tableAddress = cpu->fetchOpcodeWord();
    cpu->writeMemory32(cpu->currentSegment(), tableAddress + 2, cpu->IDTR.base);
    cpu->writeMemory16(cpu->currentSegment(), tableAddress, cpu->IDTR.limit);
}

void _LGDT(VCpu* cpu)
{
    WORD tableAddress = cpu->fetchOpcodeWord();
    cpu->GDTR.base = cpu->readMemory32(cpu->currentSegment(), tableAddress + 2);
    cpu->GDTR.limit = cpu->readMemory16(cpu->currentSegment(), tableAddress);
}

void _LIDT(VCpu* cpu)
{
    WORD tableAddress = cpu->fetchOpcodeWord();
    cpu->IDTR.base = cpu->readMemory32(cpu->currentSegment(), tableAddress + 2);
    cpu->IDTR.limit = cpu->readMemory16(cpu->currentSegment(), tableAddress);
}

void _LMSW(VCpu* cpu)
{
    BYTE msw = cpu->readModRM16(cpu->subrmbyte);
    cpu->CR0 = (cpu->CR0 & 0xFFFFFFF0) | msw;
}
