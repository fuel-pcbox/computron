// Computron x86 PC Emulator
// Copyright (C) 2003-2018 Andreas Kling <awesomekling@gmail.com>
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY ANDREAS KLING ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANDREAS KLING OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "CPU.h"
#include "debugger.h"
#include "Tasking.h"

void CPU::_STR_RM16(Instruction& insn)
{
    if (!getPE() || getVM()) {
        throw InvalidOpcode("STR not recognized in real/VM86 mode");
    }
    insn.modrm().writeClearing16(TR.selector, o32());
}

void CPU::_LTR_RM16(Instruction& insn)
{
    if (!getPE() || getVM()) {
        throw InvalidOpcode("LTR not recognized in real/VM86 mode");
    }

    WORD selector = insn.modrm().read16();
    auto descriptor = getDescriptor(selector);

    if (getCPL() != 0) {
        throw GeneralProtectionFault(0, QString("LTR with CPL(%u)!=0").arg(getCPL()));
    }
    if (!descriptor.isGlobal()) {
        throw GeneralProtectionFault(selector, "LTR selector must reference GDT");
    }
    if (!descriptor.isTSS()) {
        throw GeneralProtectionFault(selector, "LTR with non-TSS descriptor");
    }
    auto& tssDescriptor = descriptor.asTSSDescriptor();
    if (tssDescriptor.isBusy()) {
        throw GeneralProtectionFault(selector, "LTR with busy TSS");
    }
    if (!tssDescriptor.present()) {
        throw NotPresent(selector, "LTR with non-present TSS");
    }

    tssDescriptor.setBusy();
    writeToGDT(tssDescriptor);

    TR.selector = selector;
    TR.base = tssDescriptor.base();
    TR.limit = tssDescriptor.limit();
    TR.is32Bit = tssDescriptor.is32Bit();
#ifdef DEBUG_TASK_SWITCH
    vlog(LogAlert, "LTR { segment: %04x => base:%08x, limit:%08x }", TR.selector, TR.base.get(), TR.limit);
#endif
}

#define EXCEPTION_ON(type, code, condition, reason) \
    do { \
        if ((condition)) { \
            throw type(code, reason); \
        } \
    } while(0)

void CPU::taskSwitch(TSSDescriptor& incomingTSSDescriptor, JumpType source)
{
    ASSERT(incomingTSSDescriptor.is32Bit());

    EXCEPTION_ON(GeneralProtectionFault, 0, incomingTSSDescriptor.isNull(), "Incoming TSS descriptor is null");
    EXCEPTION_ON(GeneralProtectionFault, 0, !incomingTSSDescriptor.isGlobal(), "Incoming TSS descriptor is not from GDT");
    EXCEPTION_ON(NotPresent, 0, !incomingTSSDescriptor.present(), "Incoming TSS descriptor is not present");
    EXCEPTION_ON(GeneralProtectionFault, 0, incomingTSSDescriptor.limit() < 103, "Incoming TSS descriptor limit too small");

    if (source == JumpType::IRET) {
        EXCEPTION_ON(GeneralProtectionFault, 0, incomingTSSDescriptor.isAvailable(), "Incoming TSS descriptor is available");
    } else {
        EXCEPTION_ON(GeneralProtectionFault, 0, incomingTSSDescriptor.isBusy(), "Incoming TSS descriptor is busy");
    }

    auto outgoingDescriptor = getDescriptor(TR.selector);
    if (!outgoingDescriptor.isTSS()) {
        // Hmm, what have we got ourselves into now?
        vlog(LogCPU, "Switching tasks and outgoing TSS is not a TSS:");
        dumpDescriptor(outgoingDescriptor);
    }

    TSSDescriptor outgoingTSSDescriptor = outgoingDescriptor.asTSSDescriptor();
    ASSERT(outgoingTSSDescriptor.isTSS());

    TSS outgoingTSS(*this, TR.base, outgoingTSSDescriptor.is32Bit());

    outgoingTSS.setEAX(getEAX());
    outgoingTSS.setEBX(getEBX());
    outgoingTSS.setECX(getECX());
    outgoingTSS.setEDX(getEDX());
    outgoingTSS.setEBP(getEBP());
    outgoingTSS.setESP(getESP());
    outgoingTSS.setESI(getESI());
    outgoingTSS.setEDI(getEDI());

    if (source == JumpType::JMP || source == JumpType::IRET) {
        outgoingTSSDescriptor.setAvailable();
        writeToGDT(outgoingTSSDescriptor);
    }

    DWORD outgoingEFlags = getEFlags();

    if (source == JumpType::IRET) {
        outgoingEFlags &= ~Flag::NT;
    }

    outgoingTSS.setEFlags(outgoingEFlags);

    outgoingTSS.setCS(getCS());
    outgoingTSS.setDS(getDS());
    outgoingTSS.setES(getES());
    outgoingTSS.setFS(getFS());
    outgoingTSS.setGS(getGS());
    outgoingTSS.setSS(getSS());
    outgoingTSS.setLDT(LDTR.selector);
    outgoingTSS.setEIP(getEIP());

    if (getPG())
        outgoingTSS.setCR3(getCR3());

    TSS incomingTSS(*this, incomingTSSDescriptor.base(), incomingTSSDescriptor.is32Bit());

#ifdef DEBUG_TASK_SWITCH
    vlog(LogCPU, "Outgoing TSS @ %08x:", outgoingTSSDescriptor.base());
    dumpTSS(outgoingTSS);
    vlog(LogCPU, "Incoming TSS @ %08x:", incomingTSSDescriptor.base());
    dumpTSS(incomingTSS);
#endif

    // First, load all registers from TSS without validating contents.
    if (getPG()) {
        m_CR3 = incomingTSS.getCR3();
    }

    LDTR.selector = incomingTSS.getLDT();
    LDTR.base = LinearAddress();
    LDTR.limit = 0;

    CS = incomingTSS.getCS();
    DS = incomingTSS.getDS();
    ES = incomingTSS.getES();
    FS = incomingTSS.getFS();
    GS = incomingTSS.getGS();
    SS = incomingTSS.getSS();

    DWORD incomingEFlags = incomingTSS.getEFlags();

    if (incomingEFlags & Flag::VM) {
        vlog(LogCPU, "Incoming task is in VM86 mode, this needs work!");
        ASSERT_NOT_REACHED();
    }

    if (source == JumpType::CALL || source == JumpType::INT) {
        incomingEFlags |= Flag::NT;
    }

    if (incomingTSS.is32Bit())
        setEFlags(incomingEFlags);
    else
        setFlags(incomingEFlags);

    setEAX(incomingTSS.getEAX());
    setEBX(incomingTSS.getEBX());
    setECX(incomingTSS.getECX());
    setEDX(incomingTSS.getEDX());
    setEBP(incomingTSS.getEBP());
    setESP(incomingTSS.getESP());
    setESI(incomingTSS.getESI());
    setEDI(incomingTSS.getEDI());

    if (source == JumpType::CALL || source == JumpType::INT) {
        incomingTSS.setBacklink(TR.selector);
    }

    TR.selector = incomingTSSDescriptor.index();
    TR.base = incomingTSSDescriptor.base();
    TR.limit = incomingTSSDescriptor.limit();
    TR.is32Bit = incomingTSSDescriptor.is32Bit();

    if (source != JumpType::IRET) {
        incomingTSSDescriptor.setBusy();
        writeToGDT(incomingTSSDescriptor);
    }

    m_CR0 |= CR0::TS; // Task Switched

    // Now, let's validate!
    auto ldtDescriptor = getDescriptor(LDTR.selector);
    if (!ldtDescriptor.isNull()) {
        if (!ldtDescriptor.isGlobal())
            throw InvalidTSS(LDTR.selector & 0xfffc, "Incoming LDT is not in GDT");
        if (!ldtDescriptor.isLDT())
            throw InvalidTSS(LDTR.selector & 0xfffc, "Incoming LDT is not an LDT");
    }

    unsigned incomingCPL = getCS() & 3;

    auto csDescriptor = getDescriptor(CS);
    if (csDescriptor.isCode()) {
        if (csDescriptor.isNonconformingCode()) {
            if (csDescriptor.DPL() != (getCS() & 3))
                throw InvalidTSS(getCS(), QString("CS is non-conforming with DPL(%1) != RPL(%2)").arg(csDescriptor.DPL()).arg(getCS() & 3));
        } else if (csDescriptor.isConformingCode()) {
            if (csDescriptor.DPL() > (getCS() & 3))
                throw InvalidTSS(getCS(), "CS is conforming with DPL > RPL");
        }
    }
    auto ssDescriptor = getDescriptor(getSS());
    if (!ssDescriptor.isNull()) {
        if (ssDescriptor.isOutsideTableLimits())
            throw InvalidTSS(getSS(), "SS outside table limits");
        if (!ssDescriptor.isData())
            throw InvalidTSS(getSS(), "SS is not a data segment");
        if (!ssDescriptor.asDataSegmentDescriptor().writable())
            throw InvalidTSS(getSS(), "SS is not writable");
        if (!ssDescriptor.present())
            throw StackFault(getSS(), "SS is not present");
        if (ssDescriptor.DPL() != incomingCPL)
            throw InvalidTSS(getSS(), QString("SS DPL(%1) != CPL(%2)").arg(ssDescriptor.DPL()).arg(incomingCPL));
    }

    if (!ldtDescriptor.isNull()) {
        if (!ldtDescriptor.present())
            throw InvalidTSS(LDTR.selector & 0xfffc, "Incoming LDT is not present");
    }

    if (!csDescriptor.isCode())
        throw InvalidTSS(getCS(), "CS is not a code segment");
    if (!csDescriptor.present())
        throw InvalidTSS(getCS(), "CS is not present");

    if (ssDescriptor.DPL() != (getSS() & 3))
        throw InvalidTSS(getSS(), "SS DPL != RPL");

    auto validateDataSegment = [&] (SegmentRegisterIndex segreg) {
        WORD selector = readSegmentRegister(segreg);
        auto descriptor = getDescriptor(selector);
        if (descriptor.isNull())
            return;
        if (descriptor.isOutsideTableLimits())
            throw InvalidTSS(selector, "DS/ES/FS/GS outside table limits");
        if (!descriptor.isSegmentDescriptor())
            throw InvalidTSS(selector, "DS/ES/FS/GS is a system segment");
        if (!descriptor.present())
            throw NotPresent(selector, "DS/ES/FS/GS is not present");
        if (!descriptor.isConformingCode() && descriptor.DPL() < incomingCPL)
            throw InvalidTSS(selector, "DS/ES/FS/GS has DPL < CPL and is not a conforming code segment");
    };

    validateDataSegment(SegmentRegisterIndex::DS);
    validateDataSegment(SegmentRegisterIndex::ES);
    validateDataSegment(SegmentRegisterIndex::FS);
    validateDataSegment(SegmentRegisterIndex::GS);

    EXCEPTION_ON(GeneralProtectionFault, 0, getEIP() > cachedDescriptor(SegmentRegisterIndex::CS).effectiveLimit(), "Task switch to EIP outside CS limit");

    setLDT(incomingTSS.getLDT());
    setCS(incomingTSS.getCS());
    setES(incomingTSS.getES());
    setDS(incomingTSS.getDS());
    setFS(incomingTSS.getFS());
    setGS(incomingTSS.getGS());
    setSS(incomingTSS.getSS());
    setEIP(incomingTSS.getEIP());

    if (getTF()) {
        vlog(LogCPU, "Leaving task switch with TF=1");
    }

    if (getVM()) {
        vlog(LogCPU, "Leaving task switch with VM=1");
    }

#ifdef DEBUG_TASK_SWITCH
    vlog(LogCPU, "Task switched to %08x, cpl=%u, iopl=%u", incomingTSSDescriptor.base(), getCPL(), getIOPL());
#endif
}

void CPU::dumpTSS(const TSS &tss)
{
    vlog(LogCPU, "TSS bits=%u", tss.is32Bit() ? 32 : 16);
    vlog(LogCPU, "eax=%08x ebx=%08x ecx=%08x edx=%08x", tss.getEAX(), tss.getEBX(), tss.getECX(), tss.getEDX());
    vlog(LogCPU, "esi=%08x edi=%08x ebp=%08x esp=%08x", tss.getESI(), tss.getEDI(), tss.getEBP(), tss.getESP());
    vlog(LogCPU, "ldt=%04x backlink=%04x cr3=%08x", tss.getLDT(), tss.getBacklink(), getPG() ? tss.getCR3() : 0);
    vlog(LogCPU, "ds=%04x ss=%04x es=%04x fs=%04x gs=%04x", tss.getDS(), tss.getSS(), tss.getES(), tss.getFS(), tss.getGS());
    vlog(LogCPU, "cs=%04x eip=%08x eflags=%08x", tss.getCS(), tss.getEIP(), tss.getEFlags());
    vlog(LogCPU, "stack0 { %04x:%08x }", tss.getSS0(), tss.getESP0());
    vlog(LogCPU, "stack1 { %04x:%08x }", tss.getSS1(), tss.getESP1());
    vlog(LogCPU, "stack2 { %04x:%08x }", tss.getSS2(), tss.getESP2());

#ifdef DEBUG_TASK_SWITCH
    bool isGlobal = (tss.getCS() & 0x04) == 0;
    auto newCSDesc = isGlobal
            ? getDescriptor(tss.getCS(), SegmentRegisterIndex::CS)
            : getDescriptor("LDT", getDescriptor(tss.getLDT()).asLDTDescriptor().base(), getDescriptor(tss.getLDT()).asLDTDescriptor().limit(), tss.getCS(), true);

    vlog(LogCPU, "cpl=%u {%u} iopl=%u", tss.getCS() & 3, newCSDesc.DPL(), ((tss.getEFlags() >> 12) & 3));
#endif
}

void CPU::taskSwitch(WORD task, JumpType source)
{
    auto descriptor = getDescriptor(task);
    auto& tssDescriptor = descriptor.asTSSDescriptor();
    taskSwitch(tssDescriptor, source);
}

TSS CPU::currentTSS()
{
    return TSS(*this, TR.base, TR.is32Bit);
}

TSS::TSS(CPU& cpu, LinearAddress linearAddress, bool is32Bit)
    : m_pointer(cpu.memoryPointer(linearAddress))
    , m_is32Bit(is32Bit)
{
    ASSERT(m_pointer);
}

TSS16& TSS::tss16()
{
    ASSERT(!m_is32Bit);
    return *reinterpret_cast<TSS16*>(m_pointer);
}
TSS32& TSS::tss32()
{
    ASSERT(m_is32Bit);
    return *reinterpret_cast<TSS32*>(m_pointer);
}

const TSS16& TSS::tss16() const
{
    ASSERT(!m_is32Bit);
    return *reinterpret_cast<const TSS16*>(m_pointer);
}
const TSS32& TSS::tss32() const
{
    ASSERT(m_is32Bit);
    return *reinterpret_cast<const TSS32*>(m_pointer);
}

void TSS::setBacklink(WORD value)
{
    if (m_is32Bit)
        tss32().backlink = value;
    else
        tss16().backlink = value;
}

WORD TSS::getBacklink() const
{
    if (m_is32Bit)
        return tss32().backlink;
    else
        return tss16().backlink;
}

void TSS::setLDT(WORD value)
{
    if (m_is32Bit)
        tss32().LDT = value;
    else
        tss16().LDT = value;
}

WORD TSS::getLDT() const
{
    if (m_is32Bit)
        return tss32().LDT;
    else
        return tss16().LDT;
}

DWORD TSS::getCR3() const
{
    ASSERT(m_is32Bit);
    return tss32().CR3;
}

void TSS::setCR3(DWORD value)
{
    ASSERT(m_is32Bit);
    tss32().CR3 = value;
}

DWORD TSS::getEAX() const
{
    if (m_is32Bit)
        return tss32().EAX;
    else
        return tss16().AX;
}

DWORD TSS::getEBX() const
{
    if (m_is32Bit)
        return tss32().EBX;
    else
        return tss16().BX;
}

DWORD TSS::getECX() const
{
    if (m_is32Bit)
        return tss32().ECX;
    else
        return tss16().CX;
}

DWORD TSS::getEDX() const
{
    if (m_is32Bit)
        return tss32().EDX;
    else
        return tss16().DX;
}

DWORD TSS::getESI() const
{
    if (m_is32Bit)
        return tss32().ESI;
    else
        return tss16().SI;
}

DWORD TSS::getEDI() const
{
    if (m_is32Bit)
        return tss32().EDI;
    else
        return tss16().DI;
}

DWORD TSS::getEBP() const
{
    if (m_is32Bit)
        return tss32().EBP;
    else
        return tss16().BP;
}

DWORD TSS::getESP() const
{
    if (m_is32Bit)
        return tss32().ESP;
    else
        return tss16().SP;
}

DWORD TSS::getEIP() const
{
    if (m_is32Bit)
        return tss32().EIP;
    else
        return tss16().IP;
}

DWORD TSS::getEFlags() const
{
    if (m_is32Bit)
        return tss32().EFlags;
    else
        return tss16().flags;
}

void TSS::setEAX(DWORD value)
{
    if (m_is32Bit)
        tss32().EAX = value;
    else
        tss16().AX = value;
}

void TSS::setEBX(DWORD value)
{
    if (m_is32Bit)
        tss32().EBX = value;
    else
        tss16().BX = value;
}

void TSS::setECX(DWORD value)
{
    if (m_is32Bit)
        tss32().ECX = value;
    else
        tss16().CX = value;
}

void TSS::setEDX(DWORD value)
{
    if (m_is32Bit)
        tss32().EDX = value;
    else
        tss16().DX = value;
}

void TSS::setEBP(DWORD value)
{
    if (m_is32Bit)
        tss32().EBP = value;
    else
        tss16().BP = value;
}

void TSS::setESP(DWORD value)
{
    if (m_is32Bit)
        tss32().ESP = value;
    else
        tss16().SP = value;
}

void TSS::setESI(DWORD value)
{
    if (m_is32Bit)
        tss32().ESI = value;
    else
        tss16().SI = value;
}

void TSS::setEDI(DWORD value)
{
    if (m_is32Bit)
        tss32().EDI = value;
    else
        tss16().DI = value;
}

void TSS::setEIP(DWORD value)
{
    if (m_is32Bit)
        tss32().EIP = value;
    else
        tss16().IP = value;
}

void TSS::setEFlags(DWORD value)
{
    if (m_is32Bit)
        tss32().EFlags = value;
    else
        tss16().flags = value;
}

void TSS::setCS(WORD value)
{
    if (m_is32Bit)
        tss32().CS = value;
    else
        tss16().CS = value;
}

void TSS::setDS(WORD value)
{
    if (m_is32Bit)
        tss32().DS = value;
    else
        tss16().DS = value;
}

void TSS::setES(WORD value)
{
    if (m_is32Bit)
        tss32().ES = value;
    else
        tss16().ES = value;
}

void TSS::setSS(WORD value)
{
    if (m_is32Bit)
        tss32().SS = value;
    else
        tss16().SS = value;
}

void TSS::setFS(WORD value)
{
    if (m_is32Bit)
        tss32().FS = value;
    else
        tss16().FS = value;
}

void TSS::setGS(WORD value)
{
    if (m_is32Bit)
        tss32().GS = value;
    else
        tss16().GS = value;
}

WORD TSS::getCS() const
{
    if (m_is32Bit)
        return tss32().CS;
    else
        return tss16().CS;
}

WORD TSS::getDS() const
{
    if (m_is32Bit)
        return tss32().DS;
    else
        return tss16().DS;
}

WORD TSS::getES() const
{
    if (m_is32Bit)
        return tss32().ES;
    else
        return tss16().ES;
}

WORD TSS::getSS() const
{
    if (m_is32Bit)
        return tss32().SS;
    else
        return tss16().SS;
}

WORD TSS::getFS() const
{
    if (m_is32Bit)
        return tss32().FS;
    else
        return tss16().FS;
}

WORD TSS::getGS() const
{
    if (m_is32Bit)
        return tss32().GS;
    else
        return tss16().GS;
}

DWORD TSS::getESP0() const
{
    if (m_is32Bit)
        return tss32().esp0;
    else
        return tss16().sp0;
}

DWORD TSS::getESP1() const
{
    if (m_is32Bit)
        return tss32().esp1;
    else
        return tss16().sp1;
}

DWORD TSS::getESP2() const
{
    if (m_is32Bit)
        return tss32().esp2;
    else
        return tss16().sp2;
}

WORD TSS::getSS0() const
{
    if (m_is32Bit)
        return tss32().ss0;
    else
        return tss16().ss0;
}

WORD TSS::getSS1() const
{
    if (m_is32Bit)
        return tss32().ss1;
    else
        return tss16().ss1;
}

WORD TSS::getSS2() const
{
    if (m_is32Bit)
        return tss32().ss2;
    else
        return tss16().ss2;
}

DWORD TSS::getRingESP(BYTE ring) const
{
    if (ring == 0)
        return getESP0();
    if (ring == 1)
        return getESP1();
    if (ring == 2)
        return getESP2();
    ASSERT_NOT_REACHED();
    return 0;
}

WORD TSS::getRingSS(BYTE ring) const
{
    if (ring == 0)
        return getSS0();
    if (ring == 1)
        return getSS1();
    if (ring == 2)
        return getSS2();
    ASSERT_NOT_REACHED();
    return 0;
}

WORD TSS::getIOMapBase() const
{
    ASSERT(m_is32Bit);
    return tss32().iomapbase;
}
