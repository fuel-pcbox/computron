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

void CPU::_SGDT(Instruction& insn)
{
    auto& modrm = insn.modrm();
    BYTE* ptr = reinterpret_cast<BYTE*>(modrm.memoryPointer());
    DWORD* basePtr = reinterpret_cast<DWORD*>(ptr);
    WORD* limitPtr = reinterpret_cast<WORD*>(ptr + 4);
    *basePtr = GDTR.base;
    *limitPtr = GDTR.limit;
}

void CPU::_SIDT(Instruction& insn)
{
    auto& modrm = insn.modrm();
    BYTE* ptr = reinterpret_cast<BYTE*>(modrm.memoryPointer());
    DWORD* basePtr = reinterpret_cast<DWORD*>(ptr);
    WORD* limitPtr = reinterpret_cast<WORD*>(ptr + 4);
    *basePtr = IDTR.base;
    *limitPtr = IDTR.limit;
}

void CPU::_SLDT_RM16(Instruction& insn)
{
    insn.modrm().writeClearing16(LDTR.segment, o32());
}

void CPU::_STR_RM16(Instruction& insn)
{
    insn.modrm().writeClearing16(TR.segment, o32());
}

void CPU::setLDT(WORD segment)
{
    auto gdtEntry = makeSegmentSelector(segment);
    LDTR.segment = segment;
    LDTR.base = gdtEntry.base;
    LDTR.limit = gdtEntry.limit;
    vlog(LogAlert, "setLDT { segment: %04X => base:%08X, limit:%08X }", LDTR.segment, LDTR.base, LDTR.limit);
}

void CPU::_LLDT_RM16(Instruction& insn)
{
    setLDT(insn.modrm().read16());
    for (unsigned i = 0; i < LDTR.limit; i += 8) {
        dumpSegment(i | 4);
    }
}

void CPU::_LTR_RM16(Instruction& insn)
{
    WORD segment = insn.modrm().read16();
    auto gdtEntry = makeSegmentSelector(segment);
    TR.segment = segment;
    TR.base = gdtEntry.base;
    TR.limit = gdtEntry.limit;
    vlog(LogAlert, "LTR { segment: %04X => base:%08X, limit:%08X }", TR.segment, TR.base, TR.limit);
}

void CPU::_LGDT(Instruction& insn)
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

void CPU::_LIDT(Instruction& insn)
{
    vlog(LogAlert, "Begin LIDT");
    FarPointer ptr = readModRMFarPointerSegmentFirst(insn.modrm());
    DWORD baseMask = o32() ? 0xffffffff : 0x00ffffff;
    IDTR.base = ptr.offset & baseMask;
    IDTR.limit = ptr.segment;
    vlog(LogAlert, "LIDT { base:%08X, limit:%08X }", IDTR.base, IDTR.limit);
}

void CPU::_CLTS(Instruction&)
{
    if (getPE()) {
        if (getCPL() != 0) {
            GP(0);
            return;
        }
    }
    CR0 &= ~(1 << 3);
}

void CPU::_LMSW_RM16(Instruction& insn)
{
    if (getPE()) {
        if (getCPL() != 0) {
            GP(0);
            return;
        }
    }

    WORD msw = insn.modrm().read16();
    CR0 = (CR0 & 0xFFFFFFF0) | (msw & 0x0F);
    vlog(LogCPU, "LMSW set CR0=%08X, PE=%u", CR0, getPE());
}

void CPU::_SMSW_RM16(Instruction& insn)
{
    auto& modrm = insn.modrm();
    vlog(LogCPU, "SMSW get LSW(CR0)=%04X, PE=%u", CR0 & 0xFFFF, getPE());
    if (o32() && modrm.isRegister())
        modrm.write32(CR0);
    else
        modrm.write16(CR0 & 0xFFFF);
}

void CPU::_LAR_reg16_RM16(Instruction& insn)
{
    VM_ASSERT(false);
}

void CPU::_LAR_reg32_RM32(Instruction& insn)
{
    VM_ASSERT(false);
}

CPU::SegmentSelector CPU::makeSegmentSelector(WORD index)
{
    SegmentSelector selector;

    if (!getPE()) {
        selector.index = index;
        selector.base = (DWORD)index << 4;
        selector.limit = 0xFFFFF;
        selector._32bit = false;
        selector.isGlobal = true;
        return selector;
    }

    selector.isGlobal = (index & 0x04) == 0;
    selector.RPL = index & 3;
    index &= 0xfffffff8;
    selector.index = index;
    WORD tableLimit = selector.isGlobal ? GDTR.limit : LDTR.limit;
    if (index >= tableLimit) {
        vlog(LogCPU, "Segment selector index 0x%04x >= %s.limit (0x%04x).", index, selector.isGlobal ? "GDTR" : "LDTR", tableLimit);
        VM_ASSERT(false);
        //dumpAll();
        debugger().enter();
        //vomit_exit(1);
    }

    DWORD descriptorTableBase = selector.isGlobal ? GDTR.base : LDTR.base;

    DWORD hi = readMemory32(descriptorTableBase + index + 4);
    DWORD lo = readMemory32(descriptorTableBase + index);

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

    if (selector.type == 9 || selector.type == 11) {
        selector.isTask = true;
        //VM_ASSERT(false);
    }

    //vlog(LogCPU, "makeSegmentSelector: GDTR.base{%08X} + index{%04X} => base:%08X, limit:%08X", GDTR.base, index, selector.base, selector.limit);
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

void CPU::syncSegmentRegister(SegmentRegisterIndex segmentRegisterIndex)
{
    ASSERT_VALID_SEGMENT_INDEX(segmentRegisterIndex);
    CPU::SegmentSelector& selector = m_selector[(int)segmentRegisterIndex];
    selector = makeSegmentSelector(getSegment(segmentRegisterIndex));

    if (options.pedebug) {
        if (getPE())
            vlog(LogCPU, "%s loaded with %04X { type:%02X, base:%08X, limit:%08X }", toString(segmentRegisterIndex), getSegment(segmentRegisterIndex), selector.type, selector.base, selector.limit);
    }
}

void CPU::taskSwitch(WORD task)
{
    // FIXME: This should mark the outgoing task as non-busy.

    auto selector = makeSegmentSelector(task);
    TSS& tss = *reinterpret_cast<TSS*>(memoryPointer(selector.base));

    setES(tss.ES);
    setCS(tss.CS);
    setDS(tss.DS);
    setFS(tss.FS);
    setGS(tss.GS);
    setSS(tss.SS);
    EIP = tss.EIP;

    if (getPG())
        CR3 = tss.CR3;

    setLDT(tss.LDT);

    setEFlags(tss.EFlags);
    VM_ASSERT(!getNT()); // I think we shouldn't be able to unnest more than once.

    regs.D.EAX = tss.EAX;
    regs.D.EBX = tss.EBX;
    regs.D.ECX = tss.ECX;
    regs.D.EDX = tss.EDX;
    regs.D.EBP = tss.EBP;
    regs.D.ESP = tss.ESP;
    regs.D.ESI = tss.ESI;
    regs.D.EDI = tss.EDI;

    CR0 |= 0x04; // TS (Task Switched)
}

TSS* CPU::currentTSS()
{
    return reinterpret_cast<TSS*>(memoryPointer(TR.base));
}
