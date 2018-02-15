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

#include "CPU.h"
#include "debugger.h"

//#define DEBUG_IVT

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
    auto gdtEntry = getDescriptor(segment);
    LDTR.segment = segment;
    LDTR.base = gdtEntry.base();
    LDTR.limit = gdtEntry.limit();
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
    auto gdtEntry = getDescriptor(segment);
    TR.segment = segment;
    TR.base = gdtEntry.base();
    TR.limit = gdtEntry.limit();
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
    FarPointer ptr = readModRMFarPointerSegmentFirst(insn.modrm());
    DWORD baseMask = o32() ? 0xffffffff : 0x00ffffff;
    IDTR.base = ptr.offset & baseMask;
    IDTR.limit = ptr.segment;
    vlog(LogAlert, "LIDT { base:%08X, limit:%08X }", IDTR.base, IDTR.limit);
#if DEBUG_IVT
    for (DWORD isr = 0; isr < (IDTR.limit / 16); ++isr) {
        FarPointer vector;
        DWORD hi = readMemory32(IDTR.base + (isr * 8) + 4);
        DWORD lo = readMemory32(IDTR.base + (isr * 8));
        vector.segment = (lo >> 16) & 0xffff;
        vector.offset = (hi & 0xffff0000) | (lo & 0xffff);
        vlog(LogAlert, "Interrupt vector 0x%02x { 0x%04x:%08x }", isr, vector.segment, vector.offset);
    }
#endif
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
    auto& descriptor = m_descriptor[(int)segmentRegisterIndex];
    descriptor = getDescriptor(getSegment(segmentRegisterIndex));

    if (options.pedebug) {
        if (getPE())
            vlog(LogCPU, "%s loaded with %04X { type:%02X, base:%08X, limit:%08X }", toString(segmentRegisterIndex), getSegment(segmentRegisterIndex), descriptor.type(), descriptor.base(), descriptor.limit());
    }
}

void CPU::taskSwitch(WORD task)
{
    // FIXME: This should mark the outgoing task as non-busy.

    auto descriptor = getDescriptor(task);
    TSS& tss = *reinterpret_cast<TSS*>(memoryPointer(descriptor.base()));

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
