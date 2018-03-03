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

void CPU::setLDT(WORD selector)
{
    auto descriptor = getDescriptor(selector);
    DWORD base = 0;
    DWORD limit = 0;
    if (!descriptor.isNull()) {
        // FIXME: Generate exception?
        if (descriptor.isLDT()) {
            auto& ldtDescriptor = descriptor.asLDTDescriptor();
            if (!descriptor.present()) {
                triggerNP(selector, "LDT segment not present");
                return;
            }
            base = ldtDescriptor.base();
            limit = ldtDescriptor.limit();
        } else {
            // FIXME: What do when non-LDT descriptor loaded?
            dumpDescriptor(descriptor);
        }
    }
    LDTR.segment = selector;
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
            triggerGP(0, QString("CLTS with CPL!=0(%1)").arg(getCPL()));
            return;
        }
    }
    CR0 &= ~(1 << 3);
}

void CPU::_LMSW_RM16(Instruction& insn)
{
    if (getPE()) {
        if (getCPL() != 0) {
            triggerGP(0, QString("LMSW with CPL!=0(%1)").arg(getCPL()));
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

void CPU::exception(BYTE num)
{
    switch (m_exceptionState) {
    case NoException:
        m_exceptionState = SingleFault;
        break;
    case SingleFault:
        vlog(LogAlert, "D-D-Double Fault!");
        m_exceptionState = DoubleFault;
        num = 0x8; // Reroute to #DF handler
        break;
    case DoubleFault:
        vlog(LogAlert, "T-T-Triple Fault!");
        m_exceptionState = TripleFault;
        CRASH();
        break;
    case TripleFault:
        // Yikes...
        break;
    }
    setEIP(getBaseEIP());
    jumpToInterruptHandler(num);
}

void CPU::exception(BYTE num, WORD error)
{
    exception(num);
    if (o32())
        push32(error);
    else
        push16(error);
}

void CPU::triggerGP(WORD code, const QString& reason)
{
    WORD selector = code & 0xfff8;
    bool TI = code & 4;
    bool I = code & 2;
    bool EX = code & 1;
    vlog(LogCPU, "Exception: #GP(%04x) selector=%04X, TI=%u, I=%u, EX=%u :: %s", code, selector, TI, I, EX, qPrintable(reason));
    if (options.crashOnGPF)
        ASSERT_NOT_REACHED();
    exception(13, code);
}

void CPU::triggerSS(WORD selector, const QString& reason)
{
    vlog(LogCPU, "Exception: #SS(%04x) :: %s", selector, qPrintable(reason));
    exception(0xc, selector);
}

void CPU::triggerNP(WORD selector, const QString& reason)
{
    vlog(LogCPU, "Exception: #NP(%04x) :: %s", selector, qPrintable(reason));
    exception(0xb, selector);
}

void CPU::triggerTS(WORD selector, const QString& reason)
{
    vlog(LogCPU, "Exception: #TS(%04x) :: %s", selector, qPrintable(reason));
    exception(0xa, selector);
}

void CPU::triggerPF(DWORD address, WORD error, const QString& reason)
{
    vlog(LogCPU, "Exception: #PF(%04x) address=%08x :: %s", error, address, qPrintable(reason));
    CR2 = address;
    exception(0x0e, error);
}

bool CPU::validateSegmentLoad(SegmentRegisterIndex reg, WORD selector, const Descriptor& descriptor)
{
    if (!getPE())
        return true;

    BYTE selectorRPL = selector & 3;

    if (descriptor.isError()) {
        triggerGP(selector, "Selector outside table limits");
        return false;
    }

    if (reg == SegmentRegisterIndex::SS) {
        if (descriptor.isNull()) {
            triggerGP(0, "ss loaded with null descriptor");
            return false;
        }
        if (selectorRPL != getCPL()) {
            dumpDescriptor(descriptor);
            triggerGP(0, QString("ss selector RPL(%1) != CPL(%2)").arg(selectorRPL).arg(getCPL()));
            return false;
        }
        if (!descriptor.isData() || !descriptor.asDataSegmentDescriptor().writable()) {
            triggerGP(selector, "ss loaded with something other than a writable data segment");
            return false;
        }
        if (!descriptor.present()) {
            triggerSS(selector, "ss loaded with non-present segment");
            return false;
        }
        return true;
    }

    if (descriptor.isNull())
        return true;

    if (reg == SegmentRegisterIndex::DS
        || reg == SegmentRegisterIndex::ES
        || reg == SegmentRegisterIndex::FS
        || reg == SegmentRegisterIndex::GS) {
        if (!descriptor.isData() && (descriptor.isCode() && !descriptor.asCodeSegmentDescriptor().readable())) {
            triggerGP(selector, QString("%1 loaded with non-data or non-readable code segment").arg(registerName(reg)));
            return false;
        }
        if (descriptor.isData() || descriptor.isNonconformingCode()) {
            if (selectorRPL > descriptor.DPL()) {
                triggerGP(selector, QString("%1 loaded with data or non-conforming code segment and RPL > DPL").arg(registerName(reg)));
                return false;
            }
            if (getCPL() > descriptor.DPL()) {
                triggerGP(selector, QString("%1 loaded with data or non-conforming code segment and RPL > DPL").arg(registerName(reg)));
                return false;
            }
        }
        if (!descriptor.present()) {
            triggerNP(selector, QString("%1 loaded with non-present segment").arg(registerName(reg)));
            return false;
        }
    }

    if (!descriptor.isNull() && !descriptor.isSegmentDescriptor()) {
        dumpDescriptor(descriptor);
        triggerGP(0, QString("%1 loaded with system segment").arg(registerName(reg)));
        return false;
    }

    return true;
}

void CPU::setSegmentRegister(SegmentRegisterIndex segmentRegisterIndex, WORD selector)
{
    auto& descriptorCache = static_cast<Descriptor&>(m_descriptor[(int)segmentRegisterIndex]);
    auto descriptor = getDescriptor(selector);

    if (!validateSegmentLoad(segmentRegisterIndex, selector, descriptor)) {
        dumpDescriptor(descriptor);
        CRASH();
        return;
    }

    *m_segmentMap[(int)segmentRegisterIndex] = selector;

    if (descriptor.isNull()) {
        descriptorCache = descriptor;
        return;
    }

    ASSERT(descriptor.isSegmentDescriptor());

    if (!getPE() || segmentRegisterIndex == SegmentRegisterIndex::SS) {
        // HACK: In PE=0 mode, mark SS descriptors as "expand down"
        descriptor.m_type |= 0x4;
    }

    descriptorCache = descriptor.asSegmentDescriptor();
    if (options.pedebug) {
        if (getPE()) {
            vlog(LogCPU, "%s loaded with %04x { type:%02X, base:%08X, limit:%08X }",
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
        if (getPE()) {
            setCPL(descriptor.DPL());
        }
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
