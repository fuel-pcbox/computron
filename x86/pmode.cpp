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

void VCpu::_SGDT()
{
    WORD tableAddress = fetchOpcodeWord();
    writeMemory32(currentSegment(), tableAddress + 2, GDTR.base);
    writeMemory16(currentSegment(), tableAddress, GDTR.limit);
}

void VCpu::_SIDT()
{
    WORD tableAddress = fetchOpcodeWord();
    writeMemory32(currentSegment(), tableAddress + 2, IDTR.base);
    writeMemory16(currentSegment(), tableAddress, IDTR.limit);
}

void VCpu::_LGDT()
{
    WORD tableAddress = fetchOpcodeWord();
    GDTR.base = readMemory32(currentSegment(), tableAddress + 2);
    GDTR.limit = readMemory16(currentSegment(), tableAddress);
}

void VCpu::_LIDT()
{
    WORD tableAddress = fetchOpcodeWord();
    IDTR.base = readMemory32(currentSegment(), tableAddress + 2);
    IDTR.limit = readMemory16(currentSegment(), tableAddress);
}

void VCpu::_LMSW_RM16()
{
    BYTE msw = readModRM16(subrmbyte);
    CR0 = (CR0 & 0xFFFFFFF0) | msw;
    updateSizeModes();
}

void VCpu::_SMSW_RM16()
{
    writeModRM16(subrmbyte, CR0 & 0xFFFF);
}
