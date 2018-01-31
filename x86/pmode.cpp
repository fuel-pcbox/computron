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
#include "debugger.h"

void VCpu::_SGDT()
{
    VM_ASSERT(false);
    // FIXME: I don't think is implemented correctly.
    WORD tableAddress = fetchOpcodeWord();
    writeMemory32(currentSegment(), tableAddress + 2, GDTR.base);
    writeMemory16(currentSegment(), tableAddress, GDTR.limit);
}

void VCpu::_SIDT()
{
    VM_ASSERT(false);
    // FIXME: I don't think is implemented correctly.
    WORD tableAddress = fetchOpcodeWord();
    writeMemory32(currentSegment(), tableAddress + 2, IDTR.base);
    writeMemory16(currentSegment(), tableAddress, IDTR.limit);
}

void VCpu::_LGDT()
{
    vlog(LogAlert, "Begin LGDT");
    FarPointer ptr = readModRMFarPointerSegmentFirst(this->subrmbyte);
    DWORD baseMask = o32() ? 0xffffffff : 0x00ffffff;
    GDTR.base = ptr.offset & baseMask;
    GDTR.limit = ptr.segment;
    dumpAll();
    dumpMemory(getES(), getSI() + 8, 2);
    vlog(LogAlert, "LGDT { base:%08X, limit: %08X }", GDTR.base, GDTR.limit);

    for (unsigned i = 0; i < GDTR.limit; i += 8) {
        dumpSegment(i);
    }
}

void VCpu::_LIDT()
{
    vlog(LogAlert, "Begin LIDT");
    FarPointer ptr = readModRMFarPointerSegmentFirst(this->subrmbyte);
    DWORD baseMask = o32() ? 0xffffffff : 0x00ffffff;
    IDTR.base = ptr.offset & baseMask;
    IDTR.limit = ptr.segment;
    vlog(LogAlert, "LIDT { base:%08X, limit: %08X }", IDTR.base, IDTR.limit);
}

void VCpu::_LMSW_RM16()
{
    BYTE msw = readModRM16(subrmbyte);
    CR0 = (CR0 & 0xFFFFFFF0) | msw;
    vlog(LogCPU, "LMSW set CR0=%08X, PE=%u", CR0, getPE());
    //updateSizeModes();
}

void VCpu::_SMSW_RM16()
{
    vlog(LogCPU, "SMSW get LSW(CR0)=%04X, PE=%u", CR0 & 0xFFFF, getPE());
    writeModRM16(subrmbyte, CR0 & 0xFFFF);
}

VCpu::SegmentSelector VCpu::makeSegmentSelector(WORD index)
{
    if (index % 8) {
        vlog(LogCPU, "Segment selector index 0x%04X not divisible by 8.", index);
        debugger()->enter();
        vomit_exit(1);
    }

    if (index >= this->GDTR.limit) {
        vlog(LogCPU, "Segment selector index 0x%04X >= GDTR.limit (0x%04X).", index, GDTR.limit);
        debugger()->enter();
        //vomit_exit(1);
    }

    //vlog(LogAlert, "makeSegmentSelector: GDTR.base{%08X} + index{%04X}", GDTR.base, index);
    //dumpAll();

    DWORD hi = readMemory32(this->GDTR.base + index + 4);
    DWORD lo = readMemory32(this->GDTR.base + index);

    struct SegDescr{
        uint16_t limit_1;   // limit, bits 0..15
        uint16_t base_1;    // base, bits 0..15
        uint8_t base_2;     // base, bits 16..23
        uint8_t type_attr;  // type_attr
        uint8_t lim_attr;
          //^ bits 0..3: limit, bits 16..19
          //^ bits 4..7: additional data/code attributes
        uint8_t base_3;     // base, bits 24..31
    };

    SegmentSelector selector;

    selector.base = (hi & 0xFF000000) | ((hi & 0xFF) << 16) | ((lo >> 16) & 0xFFFF);
    selector.limit = (hi & 0xF0000) | (lo & 0xFFFF);
    selector.accessed = (hi >> 8) & 1;
    selector.RW = (hi >> 9) & 1; // Read/Write
    selector.DC = (hi >> 10) & 1; // Direction/Conforming
    selector.executable = (hi >> 11) & 1;
    selector.DPL = (hi >> 12) & 3; // Privilege (ring) level
    selector.present = (hi >> 14) & 1;
    selector._32bit = (hi >> 22) & 1;
    selector.granularity = (hi >> 23) & 1; // Limit granularity, 0=1b, 1=4kB
    return selector;
}

void VCpu::syncSegmentRegister(SegmentIndex segmentRegisterIndex)
{
    ASSERT_VALID_SEGMENT_INDEX(segmentRegisterIndex);
    VCpu::SegmentSelector& selector = m_selector[segmentRegisterIndex];
    selector = makeSegmentSelector(getSegment(segmentRegisterIndex));
}

