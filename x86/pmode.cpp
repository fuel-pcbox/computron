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

void VCpu::_SGDT(Instruction& insn)
{
    auto& modrm = insn.modrm();
    BYTE* ptr = reinterpret_cast<BYTE*>(modrm.memoryPointer());
    DWORD* basePtr = reinterpret_cast<DWORD*>(ptr);
    WORD* limitPtr = reinterpret_cast<WORD*>(ptr + 4);
    *basePtr = GDTR.base;
    *limitPtr = GDTR.limit;
}

void VCpu::_SIDT(Instruction& insn)
{
    auto& modrm = insn.modrm();
    BYTE* ptr = reinterpret_cast<BYTE*>(modrm.memoryPointer());
    DWORD* basePtr = reinterpret_cast<DWORD*>(ptr);
    WORD* limitPtr = reinterpret_cast<WORD*>(ptr + 4);
    *basePtr = IDTR.base;
    *limitPtr = IDTR.limit;
}

void VCpu::_SLDT_RM16(Instruction& insn)
{
    insn.modrm().write16(LDTR.segment);
}

void VCpu::_LLDT_RM16(Instruction& insn)
{
    WORD segment = insn.modrm().read16();
    auto gdtEntry = makeSegmentSelector(segment);
    LDTR.segment = segment;
    LDTR.base = gdtEntry.base;
    LDTR.limit = gdtEntry.limit;
    vlog(LogAlert, "LLDT { segment: %04X => base:%08X, limit:%08X }", LDTR.segment, LDTR.base, LDTR.limit);
}

void VCpu::_LTR_RM16(Instruction& insn)
{
    WORD segment = insn.modrm().read16();
    auto gdtEntry = makeSegmentSelector(segment);
    TR.segment = segment;
    TR.base = gdtEntry.base;
    TR.limit = gdtEntry.limit;
    vlog(LogAlert, "LTR { segment: %04X => base:%08X, limit:%08X }", TR.segment, TR.base, TR.limit);
}

void VCpu::_LGDT(Instruction& insn)
{
    vlog(LogAlert, "Begin LGDT");
    FarPointer ptr = readModRMFarPointerSegmentFirst(insn.modrm());
    DWORD baseMask = o32() ? 0xffffffff : 0x00ffffff;
    GDTR.base = ptr.offset & baseMask;
    GDTR.limit = ptr.segment;
    vlog(LogAlert, "LGDT { base:%08X, limit:%08X }", GDTR.base, GDTR.limit);

    for (unsigned i = 0; i < GDTR.limit; i += 8) {
        dumpSegment(i);
    }
}

void VCpu::_LIDT(Instruction& insn)
{
    vlog(LogAlert, "Begin LIDT");
    FarPointer ptr = readModRMFarPointerSegmentFirst(insn.modrm());
    DWORD baseMask = o32() ? 0xffffffff : 0x00ffffff;
    IDTR.base = ptr.offset & baseMask;
    IDTR.limit = ptr.segment;
    vlog(LogAlert, "LIDT { base:%08X, limit:%08X }", IDTR.base, IDTR.limit);
}

void VCpu::_LMSW_RM16(Instruction& insn)
{
    if (getCPL()) {
        GP(0);
    }

    WORD msw = insn.modrm().read16();
    CR0 = (CR0 & 0xFFFFFFF0) | (msw & 0x0F);
    vlog(LogCPU, "LMSW set CR0=%08X, PE=%u", CR0, getPE());
}

void VCpu::_SMSW_RM16(Instruction& insn)
{
    auto& modrm = insn.modrm();
    vlog(LogCPU, "SMSW get LSW(CR0)=%04X, PE=%u", CR0 & 0xFFFF, getPE());
    if (o32() && modrm.isRegister())
        modrm.write32(CR0);
    else
        modrm.write16(CR0 & 0xFFFF);
}

VCpu::SegmentSelector VCpu::makeSegmentSelector(WORD index)
{
    if (!getPE()) {
        SegmentSelector selector;
        selector.index = index;
        selector.base = (DWORD)index << 4;
        selector.limit = 0xFFFFF;
        return selector;
    }

    if (index % 8) {
        vlog(LogCPU, "Segment selector index 0x%04X not divisible by 8.", index);
        dumpAll();
        VM_ASSERT(false);
        debugger().enter();
        //vomit_exit(1);
    }

    if (index >= this->GDTR.limit) {
        vlog(LogCPU, "Segment selector index 0x%04X >= GDTR.limit (0x%04X).", index, GDTR.limit);
        dumpAll();
        VM_ASSERT(false);
        debugger().enter();
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

    selector.index = index;
    selector.base = (hi & 0xFF000000) | ((hi & 0xFF) << 16) | ((lo >> 16) & 0xFFFF);
    selector.limit = (hi & 0xF0000) | (lo & 0xFFFF);
    selector.accessed = (hi >> 8) & 1;
    selector.RW = (hi >> 9) & 1; // Read/Write
    selector.DC = (hi >> 10) & 1; // Direction/Conforming
    selector.executable = (hi >> 11) & 1;
    selector.DPL = (hi >> 12) & 3; // Privilege (ring) level
    selector.present = (hi >> 14) & 1;
    selector.type = (hi >> 16) & 0xF;
    selector._32bit = (hi >> 22) & 1;
    selector.granularity = (hi >> 23) & 1; // Limit granularity, 0=1b, 1=4kB
    return selector;
}

const char* toString(SegmentRegisterIndex segment)
{
    switch (segment) {
    case SegmentRegisterIndex::CS: return "CS";
    case SegmentRegisterIndex::DS: return "DS";
    case SegmentRegisterIndex::ES: return "ES";
    case SegmentRegisterIndex::SS: return "SS";
    case SegmentRegisterIndex::FS: return "FS";
    case SegmentRegisterIndex::GS: return "GS";
    default: break;
    }
    return nullptr;
}

void VCpu::syncSegmentRegister(SegmentRegisterIndex segmentRegisterIndex)
{
    ASSERT_VALID_SEGMENT_INDEX(segmentRegisterIndex);
    VCpu::SegmentSelector& selector = m_selector[(int)segmentRegisterIndex];
    selector = makeSegmentSelector(getSegment(segmentRegisterIndex));

    if (getPE())
        vlog(LogCPU, "%s loaded with { type:%02X, base:%08X, limit:%08X }", toString(segmentRegisterIndex), selector.type, selector.base, selector.limit);
}

