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

//#define DEBUG_GDT
//#define DEBUG_LDT
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

void CPU::setLDT(WORD segment)
{
    auto descriptor = getDescriptor(segment);
    DWORD base = 0;
    DWORD limit = 0;
    if (!descriptor.isNull()) {
        // FIXME: Generate exception?
        if (descriptor.isLDT()) {
            ASSERT(descriptor.isLDT());
            auto& ldtDescriptor = descriptor.asLDTDescriptor();
            base = ldtDescriptor.base();
            limit = ldtDescriptor.limit();
        } else {
            // FIXME: What do when non-LDT descriptor loaded?
            dumpDescriptor(descriptor);
        }
    }
    LDTR.segment = segment;
    LDTR.base = base;
    LDTR.limit = limit;
    vlog(LogAlert, "setLDT { segment: %04X => base:%08X, limit:%08X }", LDTR.segment, LDTR.base, LDTR.limit);
}

void CPU::_LLDT_RM16(Instruction& insn)
{
    setLDT(insn.modrm().read16());

#ifdef DEBUG_LDT
    for (unsigned i = 0; i < LDTR.limit; i += 8) {
        dumpSegment(i | 4);
    }
#endif
}

void CPU::_LGDT(Instruction& insn)
{
    vlog(LogAlert, "Begin LGDT");
    FarPointer ptr = readModRMFarPointerSegmentFirst(insn.modrm());
    DWORD baseMask = o32() ? 0xffffffff : 0x00ffffff;
    GDTR.base = ptr.offset & baseMask;
    GDTR.limit = ptr.segment;
    vlog(LogAlert, "LGDT { base:%08X, limit:%08X }", GDTR.base, GDTR.limit);

#ifdef DEBUG_GDT
    for (unsigned i = 0; i < GDTR.limit; i += 8) {
        dumpSegment(i);
    }
#endif
}

void CPU::_LIDT(Instruction& insn)
{
    FarPointer ptr = readModRMFarPointerSegmentFirst(insn.modrm());
    DWORD baseMask = o32() ? 0xffffffff : 0x00ffffff;
    IDTR.base = ptr.offset & baseMask;
    IDTR.limit = ptr.segment;
    vlog(LogAlert, "LIDT { base:%08X, limit:%08X }", IDTR.base, IDTR.limit);

#if DEBUG_IVT
    if (getPE()) {
        for (DWORD isr = 0; isr < (IDTR.limit / 16); ++isr) {
            dumpDescriptor(getInterruptGate(isr));
        }
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
    // FIXME: This has various ways it can fail, implement them.
    WORD selector = insn.modrm().read16() & 0xffff;
    auto descriptor = getDescriptor(selector);
    insn.reg16() = descriptor.m_high & 0x00ffff00;
    setZF(1);
}

void CPU::_LAR_reg32_RM32(Instruction& insn)
{
    // FIXME: This has various ways it can fail, implement them.
    WORD selector = insn.modrm().read32() & 0xffff;
    auto descriptor = getDescriptor(selector);
    insn.reg32() = descriptor.m_high & 0x00ffff00;
    setZF(1);
}

void CPU::_LSL_reg16_RM16(Instruction& insn)
{
    WORD selector = insn.modrm().read16() & 0xffff;
    auto descriptor = getDescriptor(selector);
    if (descriptor.isError()) {
        setZF(0);
        return;
    }
    insn.reg16() = descriptor.asSegmentDescriptor().effectiveLimit();
    setZF(1);
}

void CPU::_LSL_reg32_RM32(Instruction& insn)
{
    WORD selector = insn.modrm().read16() & 0xffff;
    auto descriptor = getDescriptor(selector);
    if (descriptor.isError()) {
        setZF(0);
        return;
    }
    insn.reg32() = descriptor.asSegmentDescriptor().effectiveLimit();
    setZF(1);
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
    auto& descriptorCache = static_cast<Descriptor&>(m_descriptor[(int)segmentRegisterIndex]);

    WORD selector = getSegment(segmentRegisterIndex);
    auto descriptor = getDescriptor(selector);

    if (descriptor.isError()) {
        vlog(LogCPU, "Error when loading %s with %04x: m_error=%u",
            toString(segmentRegisterIndex),
            selector,
            (unsigned)descriptor.error()
        );
        GP(selector);
        return;
    }

    if (descriptor.isNull()) {
        descriptorCache = descriptor;
        return;
    }

    if (!descriptor.isSegmentDescriptor())
        dumpDescriptor(descriptor);
    ASSERT(descriptor.isSegmentDescriptor());

    if (!getPE() || segmentRegisterIndex == SegmentRegisterIndex::SS) {
        // HACK: In PE=0 mode, mark SS descriptors as "expand down"
        descriptor.m_type |= 0x4;
    }

    descriptorCache = descriptor.asSegmentDescriptor();
    if (options.pedebug) {
        if (getPE()) {
            vlog(LogCPU, "%s loaded with %04X { type:%02X, base:%08X, limit:%08X }",
                toString(segmentRegisterIndex),
                selector,
                descriptor.asSegmentDescriptor().type(),
                descriptor.asSegmentDescriptor().base(),
                descriptor.asSegmentDescriptor().limit()
            );
        }
    }

    switch (segmentRegisterIndex) {
    case SegmentRegisterIndex::CS:
        if (getPE())
            setCPL(descriptor.DPL());
        updateDefaultSizes();
        updateCodeSegmentCache();
        break;
    case SegmentRegisterIndex::SS:
        updateStackSize();
        break;
    default:
        break;
    }
}

void CPU::_VERR_RM16(Instruction& insn)
{
    if (!getPE()) {
        exception(6); // #UD
        return;
    }
    WORD selector = insn.modrm().read16();
    auto descriptor = getDescriptor(selector);
    if (descriptor.isError() || descriptor.isSystemDescriptor()) {
        setZF(0);
        return;
    }
    WORD RPL = selector & 3;

    bool isConformingCode = descriptor.isCode() && descriptor.asCodeSegmentDescriptor().conforming();
    if (!isConformingCode && (getCPL() > descriptor.DPL() || RPL > descriptor.DPL())) {
        setZF(0);
        return;
    }

    if (descriptor.isCode()) {
        setZF(descriptor.asCodeSegmentDescriptor().readable());
    } else {
        // FIXME: How do I know if a data segment is "readable"? Are they always?
        setZF(1);
    }
}

void CPU::_VERW_RM16(Instruction&)
{
    ASSERT_NOT_REACHED();
}
