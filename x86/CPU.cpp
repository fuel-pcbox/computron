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
#include "machine.h"
#include "Common.h"
#include "debug.h"
#include "debugger.h"
#include "vga_memory.h"
#include "pic.h"
#include "settings.h"
#include <QtCore/QStringList>
#include <unistd.h>
#include "pit.h"
#include "Tasking.h"

#define CRASH_ON_OPCODE_00_00
#define CRASH_ON_VM
#define A20_ENABLED
//#define LOG_FAR_JUMPS
//#define DEBUG_JUMPS
//#define DISASSEMBLE_EVERYTHING
//#define MEMORY_DEBUGGING

#ifdef MEMORY_DEBUGGING
static bool shouldLogAllMemoryAccesses(DWORD address)
{
    UNUSED_PARAM(address);
#ifdef CT_DETERMINISTIC
    return true;
#endif
    return false;
}

static bool shouldLogMemoryPointer(DWORD address)
{
    if (shouldLogAllMemoryAccesses(address))
        return true;
    return false;
}

static bool shouldLogMemoryWrite(DWORD address)
{
    if (shouldLogAllMemoryAccesses(address))
        return true;
    return false;
}

static bool shouldLogMemoryRead(DWORD address)
{
    if (shouldLogAllMemoryAccesses(address))
        return true;
    return false;
}
#endif

CPU* g_cpu = 0;

template<typename T>
T CPU::readRegister(int registerIndex)
{
    if (sizeof(T) == 1)
        return *treg8[registerIndex];
    if (sizeof(T) == 2)
        return *treg16[registerIndex];
    if (sizeof(T) == 4)
        return *treg32[registerIndex];
    ASSERT_NOT_REACHED();
}

template<typename T>
void CPU::writeRegister(int registerIndex, T value)
{
    if (sizeof(T) == 1)
        *treg8[registerIndex] = value;
    else if (sizeof(T) == 2)
        *treg16[registerIndex] = value;
    else if (sizeof(T) == 4)
        *treg32[registerIndex] = value;
    else
        ASSERT_NOT_REACHED();
}

template BYTE CPU::readRegister<BYTE>(int);
template WORD CPU::readRegister<WORD>(int);
template DWORD CPU::readRegister<DWORD>(int);
template void CPU::writeRegister<BYTE>(int, BYTE);
template void CPU::writeRegister<WORD>(int, WORD);
template void CPU::writeRegister<DWORD>(int, DWORD);

void CPU::_UD0(Instruction&)
{
    vlog(LogAlert, "Undefined opcode 0F FF (UD0)");
    throw InvalidOpcode();
}

void CPU::_OperandSizeOverride(Instruction&)
{
#ifdef DEBUG_PREFIXES
    ASSERT(!m_hasOperandSizePrefix);
    m_hasOperandSizePrefix = true;
#endif
    m_shouldRestoreSizesAfterOverride = true;
    bool prevOperandSize = m_operandSize32;
    m_operandSize32 = !m_operandSize32;

#ifdef CT_DEBUG_OVERRIDE_OPCODES
    vlog(LogCPU, "Operation size override detected! Opcode: db 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X ",
         readMemory8(getCS(), getEIP() - 1),
         readMemory8(getCS(), getEIP()),
         readMemory8(getCS(), getEIP() + 1),
         readMemory8(getCS(), getEIP() + 2),
         readMemory8(getCS(), getEIP() + 3),
         readMemory8(getCS(), getEIP() + 4),
         readMemory8(getCS(), getEIP() + 5)
    );
    dumpAll();
#endif

    decodeNext();

    if (m_shouldRestoreSizesAfterOverride) {
        ASSERT(m_operandSize32 != prevOperandSize);
        m_operandSize32 = prevOperandSize;
    }
}

void CPU::_AddressSizeOverride(Instruction&)
{
#ifdef DEBUG_PREFIXES
    ASSERT(!m_hasAddressSizePrefix);
    m_hasAddressSizePrefix = true;
#endif
    m_shouldRestoreSizesAfterOverride = true;
    bool prevAddressSize32 = m_addressSize32;
    m_addressSize32 = !m_addressSize32;

#ifdef CT_DEBUG_OVERRIDE_OPCODES
    vlog(LogCPU, "Address size override detected! Opcode: db 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X ",
         readMemory8(getCS(), getEIP() - 1),
         readMemory8(getCS(), getEIP()),
         readMemory8(getCS(), getEIP() + 1),
         readMemory8(getCS(), getEIP() + 2),
         readMemory8(getCS(), getEIP() + 3),
         readMemory8(getCS(), getEIP() + 4),
         readMemory8(getCS(), getEIP() + 5)
    );
    dumpAll();
#endif

    decodeNext();

    if (m_shouldRestoreSizesAfterOverride) {
        ASSERT(m_addressSize32 != prevAddressSize32);
        m_addressSize32 = prevAddressSize32;
    }
}

BYTE CPU::readInstruction8()
{
    return fetchOpcodeByte();
}

WORD CPU::readInstruction16()
{
    return fetchOpcodeWord();
}

DWORD CPU::readInstruction32()
{
    return fetchOpcodeDWord();
}

FLATTEN void CPU::decodeNext()
{
#ifdef CT_TRACE
    if (UNLIKELY(m_isForAutotest))
        dumpTrace();
#endif

    auto insn = Instruction::fromStream(*this, o32(), a32());
    if (!insn.isValid())
        throw InvalidOpcode();
    else
        execute(std::move(insn));
}

FLATTEN void CPU::execute(Instruction&& insn)
{
#ifdef CRASH_ON_VM
    if (UNLIKELY(getVM())) {
        dumpTrace();
        ASSERT_NOT_REACHED();
    }
#endif

#ifdef CRASH_ON_OPCODE_00_00
    if (UNLIKELY(insn.op() == 0 && insn.rm() == 0)) {
        dumpTrace();
        ASSERT_NOT_REACHED();
    }
#endif

#ifdef DISASSEMBLE_EVERYTHING
    vlog(LogCPU, "%s", qPrintable(insn.toString(m_baseEIP, x32())));
#endif
    insn.execute(*this);

    ++m_cycle;
}

void CPU::_RDTSC(Instruction&)
{
    setEDX(m_cycle >> 32);
    setEAX(m_cycle);
}

void CPU::_WBINVD(Instruction&)
{
    if (getPE() && getCPL() != 0) {
        throw GeneralProtectionFault(0, "WBINVD");
    }
}

void CPU::_VKILL(Instruction&)
{
    // FIXME: Maybe (0xf1) is a bad choice of opcode here, since that's also INT1 / ICEBP.
    if (!machine().isForAutotest()) {
        throw InvalidOpcode("VKILL (0xf1) is an invalid opcode outside of auto-test mode!");
    }
    vlog(LogCPU, "0xF1: Secret shutdown command received!");
    //dumpAll();
    hard_exit(0);
}

CPU::CPU(Machine& m)
    : m_machine(m)
    , m_shouldBreakOutOfMainLoop(false)
{
    m_isForAutotest = machine().isForAutotest();

    buildOpcodeTablesIfNeeded();

    ASSERT(!g_cpu);
    g_cpu = this;

    m_memory = new BYTE[m_memorySize];
    if (!m_memory) {
        vlog(LogInit, "Insufficient memory available.");
        hard_exit(1);
    }

    memset(m_memory, 0, 1048576 + 65536);

    m_debugger = make<Debugger>(*this);

    m_controlRegisterMap[0] = &m_CR0;
    m_controlRegisterMap[1] = &m_CR1;
    m_controlRegisterMap[2] = &m_CR2;
    m_controlRegisterMap[3] = &m_CR3;
    m_controlRegisterMap[4] = &m_CR4;
    m_controlRegisterMap[5] = &m_CR5;
    m_controlRegisterMap[6] = &m_CR6;
    m_controlRegisterMap[7] = &m_CR7;

    m_debugRegisterMap[0] = &m_DR0;
    m_debugRegisterMap[1] = &m_DR1;
    m_debugRegisterMap[2] = &m_DR2;
    m_debugRegisterMap[3] = &m_DR3;
    m_debugRegisterMap[4] = &m_DR4;
    m_debugRegisterMap[5] = &m_DR5;
    m_debugRegisterMap[6] = &m_DR6;
    m_debugRegisterMap[7] = &m_DR7;

    this->treg32[RegisterEAX] = &this->regs.D.EAX;
    this->treg32[RegisterEBX] = &this->regs.D.EBX;
    this->treg32[RegisterECX] = &this->regs.D.ECX;
    this->treg32[RegisterEDX] = &this->regs.D.EDX;
    this->treg32[RegisterESP] = &this->regs.D.ESP;
    this->treg32[RegisterEBP] = &this->regs.D.EBP;
    this->treg32[RegisterESI] = &this->regs.D.ESI;
    this->treg32[RegisterEDI] = &this->regs.D.EDI;

    this->treg16[RegisterAX] = &this->regs.W.AX;
    this->treg16[RegisterBX] = &this->regs.W.BX;
    this->treg16[RegisterCX] = &this->regs.W.CX;
    this->treg16[RegisterDX] = &this->regs.W.DX;
    this->treg16[RegisterSP] = &this->regs.W.SP;
    this->treg16[RegisterBP] = &this->regs.W.BP;
    this->treg16[RegisterSI] = &this->regs.W.SI;
    this->treg16[RegisterDI] = &this->regs.W.DI;

    this->treg8[RegisterAH] = &this->regs.B.AH;
    this->treg8[RegisterBH] = &this->regs.B.BH;
    this->treg8[RegisterCH] = &this->regs.B.CH;
    this->treg8[RegisterDH] = &this->regs.B.DH;
    this->treg8[RegisterAL] = &this->regs.B.AL;
    this->treg8[RegisterBL] = &this->regs.B.BL;
    this->treg8[RegisterCL] = &this->regs.B.CL;
    this->treg8[RegisterDL] = &this->regs.B.DL;

    m_segmentMap[(int)SegmentRegisterIndex::CS] = &this->CS;
    m_segmentMap[(int)SegmentRegisterIndex::DS] = &this->DS;
    m_segmentMap[(int)SegmentRegisterIndex::ES] = &this->ES;
    m_segmentMap[(int)SegmentRegisterIndex::SS] = &this->SS;
    m_segmentMap[(int)SegmentRegisterIndex::FS] = &this->FS;
    m_segmentMap[(int)SegmentRegisterIndex::GS] = &this->GS;
    m_segmentMap[6] = nullptr;
    m_segmentMap[7] = nullptr;

    reset();
}

void CPU::reset()
{
    m_a20Enabled = false;

    memset(&regs, 0, sizeof(regs));
    m_CR0 = 0;
    m_CR1 = 0;
    m_CR2 = 0;
    m_CR3 = 0;
    m_CR4 = 0;
    m_CR5 = 0;
    m_CR6 = 0;
    m_CR7 = 0;
    m_DR0 = 0;
    m_DR1 = 0;
    m_DR2 = 0;
    m_DR3 = 0;
    m_DR4 = 0;
    m_DR5 = 0;
    m_DR6 = 0;
    m_DR7 = 0;

    this->IOPL = 0;
    this->VM = 0;
    this->VIP = 0;
    this->VIF = 0;
    this->NT = 0;
    this->RF = 0;
    this->AC = 0;
    this->ID = 0;

    this->GDTR.base = 0;
    this->GDTR.limit = 0;
    this->IDTR.base = 0;
    this->IDTR.limit = 0;
    this->LDTR.base = 0;
    this->LDTR.limit = 0;
    this->LDTR.segment = 0;
    this->TR.segment = 0;
    this->TR.limit = 0;
    this->TR.base = 0;
    this->TR.is32Bit = false;

    memset(m_descriptor, 0, sizeof(m_descriptor));

    m_segmentPrefix = SegmentRegisterIndex::None;

    setCS(0);
    setDS(0);
    setES(0);
    setSS(0);
    setFS(0);
    setGS(0);

    if (m_isForAutotest)
        jump32(machine().settings().entryCS(), machine().settings().entryIP(), JumpType::Internal);
    else
        jump32(0xF000, 0x00000000, JumpType::Internal);

    setFlags(0x0200);

    setIOPL(3);

    m_state = Alive;

    m_addressSize32 = false;
    m_operandSize32 = false;

    m_dirtyFlags = 0;
    m_lastResult = 0;
    m_lastOpSize = ByteSize;

    initWatches();

    recomputeMainLoopNeedsSlowStuff();
}

CPU::~CPU()
{
    delete [] m_memory;
    m_memory = nullptr;
}

FLATTEN void CPU::executeOneInstruction()
{
    try {
#ifdef DEBUG_PREFIXES
        m_hasAddressSizePrefix = false;
        m_hasOperandSizePrefix = false;
#endif
        resetSegmentPrefix();
        saveBaseAddress();
        decodeNext();
    } catch(Exception e) {
        dumpDisassembled(cachedDescriptor(SegmentRegisterIndex::CS), m_baseEIP, 3);
        resetSegmentPrefix();
        raiseException(e);
    }
}

void CPU::haltedLoop()
{
    while (state() == CPU::Halted) {
#ifdef HAVE_USLEEP
        usleep(100);
#endif
        if (m_shouldHardReboot) {
            hardReboot();
            return;
        }
        if (PIC::hasPendingIRQ() && getIF())
            PIC::serviceIRQ(*this);
    }
}

void CPU::queueCommand(Command command)
{
    switch (command) {
    case ExitMainLoop:
        m_shouldBreakOutOfMainLoop = true;
        break;
    case EnterMainLoop:
        m_shouldBreakOutOfMainLoop = false;
        break;
    case HardReboot:
        m_shouldHardReboot = true;
        break;
    }
    recomputeMainLoopNeedsSlowStuff();
}

void CPU::hardReboot()
{
    machine().resetAllIODevices();
    reset();
    m_shouldHardReboot = false;
}

void CPU::recomputeMainLoopNeedsSlowStuff()
{
    m_mainLoopNeedsSlowStuff = m_shouldBreakOutOfMainLoop ||
                               m_shouldHardReboot ||
                               options.trace ||
                               !m_breakpoints.empty() ||
                               debugger().isActive() ||
                               !m_watches.isEmpty();
}

NEVER_INLINE bool CPU::mainLoopSlowStuff()
{
    if (m_shouldBreakOutOfMainLoop)
        return false;

    if (m_shouldHardReboot) {
        hardReboot();
        return true;
    }

    if (!m_breakpoints.empty()) {
        DWORD flatPC = realModeAddressToPhysicalAddress(getCS(), getEIP());
        for (auto& breakpoint : m_breakpoints) {
            if (flatPC == breakpoint) {
                debugger().enter();
                break;
            }
        }
    }

    if (debugger().isActive()) {
        saveBaseAddress();
        debugger().doConsole();
    }

    if (options.trace)
        dumpTrace();

    if (!m_watches.isEmpty())
        dumpWatches();

    return true;
}

FLATTEN void CPU::mainLoop()
{
    forever {
        if (UNLIKELY(m_mainLoopNeedsSlowStuff)) {
            mainLoopSlowStuff();
        }

        executeOneInstruction();

        if (UNLIKELY(getTF())) {
            // The Trap Flag is set, so we'll execute one instruction and
            // call ISR 1 as soon as it's finished.
            //
            // This is used by tools like DEBUG to implement step-by-step
            // execution :-)
            jumpToInterruptHandler(1);

            // NOTE: jumpToInterruptHandler() just set IF=0.
        }

        if (PIC::hasPendingIRQ() && getIF())
            PIC::serviceIRQ(*this);

#ifdef CT_DETERMINISTIC
        if (getIF() && ((cycle() + 1) % 100 == 0)) {
            machine().pit().raiseIRQ();
        }
#endif
    }
}

void CPU::jumpRelative8(SIGNED_BYTE displacement)
{
    if (x32())
        this->EIP += displacement;
    else
        this->IP += displacement;
}

void CPU::jumpRelative16(SIGNED_WORD displacement)
{
    if (x32())
        this->EIP += displacement;
    else
        this->IP += displacement;
}

void CPU::jumpRelative32(SIGNED_DWORD displacement)
{
    this->EIP += displacement;
}

void CPU::jumpAbsolute16(WORD address)
{
    if (x32())
        this->EIP = address;
    else
        this->IP = address;
}

void CPU::jumpAbsolute32(DWORD address)
{
//    vlog(LogCPU, "[PE=%u] Abs jump to %08X", getPE(), address);
    this->EIP = address;
}

static const char* toString(JumpType type)
{
    switch (type) {
    case JumpType::CALL: return "CALL";
    case JumpType::RETF: return "RETF";
    case JumpType::IRET: return "IRET";
    case JumpType::INT: return "INT";
    case JumpType::JMP: return "JMP";
    case JumpType::Internal: return "Internal";
    case JumpType::GateEntry: return "GateEntry";
    default:
        ASSERT_NOT_REACHED();
        return nullptr;
    }
}

void CPU::jump32(WORD segment, DWORD offset, JumpType type, BYTE isr, DWORD flags)
{
    bool pushSize16 = o16();

    if (getPE() && type == JumpType::INT) {
        // INT through gate; respect bit size of gate descriptor!
        // FIXME: Don't reparse the gate here.
        pushSize16 = !getInterruptGate(isr).is32Bit();
    }

    WORD originalCPL = getCPL();
    WORD originalCS = getCS();
    DWORD originalEIP = getEIP();

    BYTE selectorRPL = segment & 3;

#ifdef LOG_FAR_JUMPS
    vlog(LogCPU, "[PE=%u, PG=%u] %s from %04x:%08x to %04x:%08x", getPE(), getPG(), toString(type), getBaseCS(), getBaseEIP(), segment, offset);
#endif

    auto descriptor = getDescriptor(segment, SegmentRegisterIndex::CS);

    if (getPE()) {
        if (type == JumpType::RETF) {
            if (!(selectorRPL >= getCPL())) {
                throw GeneralProtectionFault(segment, QString("RETF with !(RPL(%1) >= CPL(%2))").arg(selectorRPL).arg(getCPL()));
            }
        }
        if (descriptor.isNull()) {
            throw GeneralProtectionFault(segment, QString("%1 to null selector").arg(toString(type)));
        }
    }

    if (descriptor.isSystemDescriptor()) {
        auto& sys = descriptor.asSystemDescriptor();

#ifdef DEBUG_JUMPS
        vlog(LogCPU, "%s to %04x:%08x hit system descriptor type %s (%x)", toString(type), segment, offset, sys.typeName(), (unsigned)sys.type());
        dumpDescriptor(descriptor);
#endif
        if (sys.isGate()) {
            auto& gate = sys.asGate();
#ifdef DEBUG_JUMPS
            vlog(LogCPU, "Gate (%s) to %04x:%08x (count=%u)", gate.typeName(), gate.selector(), gate.offset(), gate.parameterCount());
#endif
            ASSERT(gate.isCallGate());
            ASSERT(!gate.parameterCount()); // FIXME: Implement
            // NOTE: We recurse here, jumping to the gate entry point.
            DWORD gateOffset = gate.offset();
            if (!gate.is32Bit())
                gateOffset &= 0xffff;
            jump32(gate.selector(), gateOffset, JumpType::GateEntry, isr);
            return;
        } else if (sys.isTSS()) {
            auto& tssDescriptor = sys.asTSSDescriptor();
#ifdef DEBUG_JUMPS
            vlog(LogCPU, "CS is this:");
            dumpDescriptor(cachedDescriptor(SegmentRegisterIndex::CS));
            vlog(LogCPU, "%s to TSS descriptor (%s) -> %08x", toString(type), tssDescriptor.typeName(), tssDescriptor.base());
#endif
            taskSwitch(tssDescriptor, type);
        } else {
            vlog(LogCPU, "%s to %04x:%08x hit unhandled descriptor type %s (%x)", toString(type), segment, offset, sys.typeName(), (unsigned)sys.type());
            dumpDescriptor(descriptor);
            ASSERT_NOT_REACHED();
        }
    } else { // it's a segment descriptor
        ASSERT(descriptor.isCode());
        setCS(segment);
        setEIP(offset);
    }

    if (getPE() && (type == JumpType::CALL || type == JumpType::INT)) {
    if (descriptor.isNonconformingCode() && descriptor.DPL() < originalCPL) {
#ifdef DEBUG_JUMPS
        vlog(LogCPU, "%s escalating privilege from ring%u to ring%u", toString(type), originalCPL, descriptor.DPL(), descriptor);
#endif
        WORD oldSS = getSS();
        DWORD oldESP = getESP();
        auto tss = currentTSS();
        setSS(tss.getRingSS(descriptor.DPL()));
        setESP(tss.getRingESP(descriptor.DPL()));
        if (pushSize16) {
            push16(oldSS);
            push16(oldESP);
#ifdef DEBUG_JUMPS
            vlog(LogCPU, "%s to inner ring, ss:sp %04x:%04x -> %04x:%04x", toString(type), oldSS, oldESP, getSS(), getSP());
#endif
        } else {
            push32(oldSS);
            push32(oldESP);
#ifdef DEBUG_JUMPS
            vlog(LogCPU, "%s to inner ring, ss:esp %04x:%08x -> %04x:%08x", toString(type), oldSS, oldESP, getSS(), getESP());
#endif
        }
        setCPL(descriptor.DPL());
    } else {
#ifdef DEBUG_JUMPS
        vlog(LogCPU, "%s same privilege from ring%u to ring%u", toString(type), originalCPL, descriptor.DPL());
#endif
        setCPL(originalCPL);
    }
    }

    if (type == JumpType::INT) {
        if (pushSize16)
            push16(flags);
        else
            push32(flags);
    }

    if (type == JumpType::CALL || type == JumpType::INT) {
        if (pushSize16) {
            push16(originalCS);
            push16(originalEIP);
        } else {
            push32(originalCS);
            push32(originalEIP);
        }
    }

    bool isReturnToOuterPrivilegeLevel = getPE() && getCPL() > originalCPL;
    if (isReturnToOuterPrivilegeLevel && (type == JumpType::IRET || type == JumpType::RETF)) {
        if (o16()) {
            WORD newSP = pop16();
            WORD newSS = pop16();
#ifdef DEBUG_JUMPS
            vlog(LogCPU, "%s from ring%u to ring%u, ss:sp %04x:%04x -> %04x:%04x", toString(type), originalCPL, getCPL(), getSS(), getSP(), newSS, newSP);
#endif
            setESP(newSP);
            setSS(newSS);
        } else {
            DWORD newESP = pop32();
            WORD newSS = pop32();
#ifdef DEBUG_JUMPS
            vlog(LogCPU, "%s from ring%u to ring%u, ss:esp %04x:%08x -> %04x:%08x", toString(type), originalCPL, getCPL(), getSS(), getESP(), newSS, newESP);
#endif
            setESP(newESP);
            setSS(newSS);
        }
    }
}

void CPU::setCPL(BYTE cpl)
{
    ASSERT(getPE());
    CS = (CS & ~3) | cpl;
    cachedDescriptor(SegmentRegisterIndex::CS).m_RPL = cpl;
}

void CPU::jump16(WORD segment, WORD offset, JumpType type, BYTE isr, DWORD flags)
{
    jump32(segment, offset, type, isr, flags);
}

void CPU::_UNSUPP(Instruction& insn)
{
    // We've come across an unsupported instruction, log it, then vector to the "illegal instruction" ISR.
    vlog(LogAlert, "Unsupported opcode %02X", insn.op());
    QString ndis = "db ";
    DWORD baseEIP = getBaseEIP();
    QStringList dbs;
    for (int i = 0; i < 16; ++i) {
        QString s;
        s.sprintf("0x%02X", readMemory8(SegmentRegisterIndex::CS, baseEIP + i));
        dbs.append(s);
    }
    ndis.append(dbs.join(", "));
    vlog(LogAlert, qPrintable(ndis));
    dumpAll();
    throw InvalidOpcode();
}

void CPU::_NOP(Instruction&)
{
}

void CPU::_HLT(Instruction&)
{
    if (getCPL() != 0) {
        throw GeneralProtectionFault(0, QString("HLT with CPL!=0(%1)").arg(getCPL()));
    }

    setState(CPU::Halted);

    if (!getIF()) {
        vlog(LogCPU, "Halted with IF=0");
    } else {
#ifdef VERBOSE_DEBUG
        vlog(LogCPU, "Halted");
#endif
    }

    haltedLoop();
}

void CPU::_XLAT(Instruction&)
{
    if (a32())
        setAL(readMemory8(currentSegment(), getEBX() + getAL()));
    else
        setAL(readMemory8(currentSegment(), getBX() + getAL()));
}

void CPU::_CS(Instruction&)
{
    ASSERT(!hasSegmentPrefix());
    setSegmentPrefix(SegmentRegisterIndex::CS);
    decodeNext();
    resetSegmentPrefix();
}

void CPU::_DS(Instruction&)
{
    ASSERT(!hasSegmentPrefix());
    setSegmentPrefix(SegmentRegisterIndex::DS);
    decodeNext();
    resetSegmentPrefix();
}

void CPU::_ES(Instruction&)
{
    ASSERT(!hasSegmentPrefix());
    setSegmentPrefix(SegmentRegisterIndex::ES);
    decodeNext();
    resetSegmentPrefix();
}

void CPU::_SS(Instruction&)
{
    ASSERT(!hasSegmentPrefix());
    setSegmentPrefix(SegmentRegisterIndex::SS);
    decodeNext();
    resetSegmentPrefix();
}

void CPU::_FS(Instruction&)
{
    ASSERT(!hasSegmentPrefix());
    setSegmentPrefix(SegmentRegisterIndex::FS);
    decodeNext();
    resetSegmentPrefix();
}

void CPU::_GS(Instruction&)
{
    ASSERT(!hasSegmentPrefix());
    setSegmentPrefix(SegmentRegisterIndex::GS);
    decodeNext();
    resetSegmentPrefix();
}

void CPU::_XCHG_AX_reg16(Instruction& insn)
{
    qSwap(insn.reg16(), regs.W.AX);
}

void CPU::_XCHG_EAX_reg32(Instruction& insn)
{
    qSwap(insn.reg32(), regs.D.EAX);
}

void CPU::_XCHG_reg8_RM8(Instruction& insn)
{
    auto& modrm = insn.modrm();
    auto tmp = insn.reg8();
    insn.reg8() = modrm.read8();
    modrm.write8(tmp);
}

void CPU::_XCHG_reg16_RM16(Instruction& insn)
{
    auto& modrm = insn.modrm();
    auto tmp = insn.reg16();
    insn.reg16() = modrm.read16();
    modrm.write16(tmp);
}

void CPU::_XCHG_reg32_RM32(Instruction& insn)
{
    auto& modrm = insn.modrm();
    auto tmp = insn.reg32();
    insn.reg32() = modrm.read32();
    modrm.write32(tmp);
}

void CPU::_DEC_reg16(Instruction& insn)
{
    auto& reg = insn.reg16();
    DWORD i = reg;

    /* Overflow if we'll wrap. */
    setOF(reg == 0x8000);

    --i;
    adjustFlag32(i, reg, 1);
    updateFlags16(i);
    --reg;
}

void CPU::_DEC_reg32(Instruction& insn)
{
    auto& reg = insn.reg32();
    QWORD i = reg;

    /* Overflow if we'll wrap. */
    setOF(reg == 0x80000000);

    --i;
    adjustFlag32(i, reg, 1);
    updateFlags32(i);
    --reg;
}

void CPU::_INC_reg16(Instruction& insn)
{
    auto& reg = insn.reg16();
    DWORD i = reg;

    /* Overflow if we'll wrap. */
    setOF(i == 0x7FFF);

    ++i;
    adjustFlag32(i, reg, 1);
    updateFlags16(i);
    ++reg;
}

void CPU::_INC_reg32(Instruction& insn)
{
    auto& reg = insn.reg32();
    QWORD i = reg;

    /* Overflow if we'll wrap. */
    setOF(i == 0x7FFFFFFF);

    ++i;
    adjustFlag32(i, reg, 1);
    updateFlags32(i);
    ++reg;
}

void CPU::_INC_RM16(Instruction& insn)
{
    auto& modrm = insn.modrm();
    auto value = modrm.read16();
    DWORD i = value;

    /* Overflow if we'll wrap. */
    setOF(value == 0x7FFF);

    ++i;
    adjustFlag32(i, value, 1);
    updateFlags16(i);
    modrm.write16(value + 1);
}

void CPU::_INC_RM32(Instruction& insn)
{
    auto& modrm = insn.modrm();
    auto value = modrm.read32();
    QWORD i = value;

    /* Overflow if we'll wrap. */
    setOF(value == 0x7FFFFFFF);

    ++i;
    adjustFlag32(i, value, 1);
    updateFlags32(i);
    modrm.write32(value + 1);
}

void CPU::_DEC_RM16(Instruction& insn)
{
    auto& modrm = insn.modrm();
    auto value = modrm.read16();
    DWORD i = value;

    /* Overflow if we'll wrap. */
    setOF(value == 0x8000);

    --i;
    adjustFlag32(i, value, 1); // XXX: i can be (dword)(-1)...
    updateFlags16(i);
    modrm.write16(value - 1);
}

void CPU::_DEC_RM32(Instruction& insn)
{
    auto& modrm = insn.modrm();
    auto value = modrm.read32();
    QWORD i = value;

    /* Overflow if we'll wrap. */
    setOF(value == 0x80000000);

    --i;
    adjustFlag32(i, value, 1); // XXX: i can be (dword)(-1)...
    updateFlags32(i);
    modrm.write32(value - 1);
}

void CPU::_INC_RM8(Instruction& insn)
{
    auto& modrm = insn.modrm();
    auto value = modrm.read8();
    WORD i = value;
    setOF(value == 0x7F);
    i++;
    adjustFlag32(i, value, 1);
    updateFlags8(i);
    modrm.write8(value + 1);
}

void CPU::_DEC_RM8(Instruction& insn)
{
    auto& modrm = insn.modrm();
    auto value = modrm.read8();
    WORD i = value;
    setOF(value == 0x80);
    i--;
    adjustFlag32(i, value, 1);
    updateFlags8(i);
    modrm.write8(value - 1);
}

void CPU::_LDS_reg16_mem16(Instruction& insn)
{
    WORD* ptr = static_cast<WORD*>(insn.modrm().memoryPointer());
    insn.reg16() = read16FromPointer(ptr);
    setDS(read16FromPointer(ptr + 1));
}

void CPU::_LDS_reg32_mem32(Instruction& insn)
{
    FarPointer ptr = readModRMFarPointerOffsetFirst(insn.modrm());
    insn.reg32() = ptr.offset;
    setDS(ptr.segment);
}

void CPU::_LES_reg16_mem16(Instruction& insn)
{
    WORD* ptr = static_cast<WORD*>(insn.modrm().memoryPointer());
    insn.reg16() = read16FromPointer(ptr);
    setES(read16FromPointer(ptr + 1));
}

void CPU::_LES_reg32_mem32(Instruction& insn)
{
    FarPointer ptr = readModRMFarPointerOffsetFirst(insn.modrm());
    insn.reg32() = ptr.offset;
    setES(ptr.segment);
}

void CPU::_LFS_reg16_mem16(Instruction& insn)
{
    WORD* ptr = static_cast<WORD*>(insn.modrm().memoryPointer());
    insn.reg16() = read16FromPointer(ptr);
    setFS(read16FromPointer(ptr + 1));
}

void CPU::_LFS_reg32_mem32(Instruction& insn)
{
    FarPointer ptr = readModRMFarPointerOffsetFirst(insn.modrm());
    insn.reg32() = ptr.offset;
    setFS(ptr.segment);
}

void CPU::_LSS_reg16_mem16(Instruction& insn)
{
    WORD* ptr = static_cast<WORD*>(insn.modrm().memoryPointer());
    insn.reg16() = read16FromPointer(ptr);
    setSS(read16FromPointer(ptr + 1));
}

void CPU::_LSS_reg32_mem32(Instruction& insn)
{
    FarPointer ptr = readModRMFarPointerOffsetFirst(insn.modrm());
    insn.reg32() = ptr.offset;
    setSS(ptr.segment);
}

void CPU::_LGS_reg16_mem16(Instruction& insn)
{
    WORD* ptr = static_cast<WORD*>(insn.modrm().memoryPointer());
    insn.reg16() = read16FromPointer(ptr);
    setGS(read16FromPointer(ptr + 1));
}

void CPU::_LGS_reg32_mem32(Instruction& insn)
{
    FarPointer ptr = readModRMFarPointerOffsetFirst(insn.modrm());
    insn.reg32() = ptr.offset;
    setGS(ptr.segment);
}

void CPU::_LEA_reg32_mem32(Instruction& insn)
{
    auto& modrm = insn.modrm();
    if (modrm.isRegister()) {
        vlog(LogAlert, "LEA_reg32_mem32 with register source!");
        throw InvalidOpcode();
    }

    insn.reg32() = modrm.offset();
}

void CPU::_LEA_reg16_mem16(Instruction& insn)
{
    auto& modrm = insn.modrm();
    if (modrm.isRegister()) {
        vlog(LogAlert, "LEA_reg16_mem16 with register source!");
        throw InvalidOpcode();
    }

    insn.reg16() = modrm.offset();
}

// FIXME: Make VGAMemory into some kind of "memory mapper" thingy and put this kind of logic in there.
inline void CPU::didTouchMemory(DWORD address)
{
    bool shouldNotifyScreen = false;
    if (addressIsInVGAMemory(address))
        shouldNotifyScreen = true;
    else if (address >= 0xB8000 && address < 0xC0000)
        shouldNotifyScreen = true;
    if (shouldNotifyScreen)
        machine().notifyScreen();
}

struct PageTableEntryFlags {
enum Flags {
    Present = 0x01,
    ReadWrite = 0x02,
    UserSupervisor = 0x04,
    Accessed = 0x20,
    Dirty = 0x40,
};
};

struct PageFaultFlags {
enum Flags {
    NotPresent = 0x00,
    ProtectionViolation = 0x01,
    Read = 0x00,
    Write = 0x02,
    UserMode = 0x04,
    SupervisorMode = 0x00,
    InstructionFetch = 0x08,
};
};

ALWAYS_INLINE void CPU::translateAddress(DWORD linearAddress, DWORD& physicalAddress, MemoryAccessType accessType)
{
    if (!getPE() || !getPG()) {
        physicalAddress = linearAddress;
        return;
    }
    translateAddressSlowCase(linearAddress, physicalAddress, accessType);
}

static WORD makePFErrorCode(PageFaultFlags::Flags flags, CPU::MemoryAccessType accessType, bool inUserMode)
{
    return flags
         | (accessType == CPU::MemoryAccessType::Write ? PageFaultFlags::Write : PageFaultFlags::Read)
         | (inUserMode ? PageFaultFlags::UserMode : PageFaultFlags::SupervisorMode)
         | (accessType == CPU::MemoryAccessType::Execute ? PageFaultFlags::InstructionFetch : 0);
}

void CPU::translateAddressSlowCase(DWORD linearAddress, DWORD& physicalAddress, MemoryAccessType accessType)
{
    ASSERT(getCR3() < m_memorySize);

    DWORD dir = (linearAddress >> 22) & 0x3FF;
    DWORD page = (linearAddress >> 12) & 0x3FF;
    DWORD offset = linearAddress & 0xFFF;

    DWORD* PDBR = reinterpret_cast<DWORD*>(&m_memory[getCR3()]);
    ASSERT(!(getCR3() & 0x03ff));
    DWORD& pageDirectoryEntry = PDBR[dir];

    DWORD* pageTable = reinterpret_cast<DWORD*>(&m_memory[pageDirectoryEntry & 0xfffff000]);
    DWORD& pageTableEntry = pageTable[page];

    bool inUserMode = getCPL() == 3;

    if (!(pageDirectoryEntry & PageTableEntryFlags::Present)) {
        vlog(LogCPU, "#PF Translating %08x {dir=%03x, page=%03x, offset=%03x} PDBR=%08x, PDE=%08x, PTE=%08x", linearAddress, dir, page, offset, getCR3(), pageDirectoryEntry, pageTableEntry);
        throw PageFault(linearAddress, makePFErrorCode(PageFaultFlags::NotPresent, accessType, inUserMode), QString("Page not present in PDE(%1)").arg(pageDirectoryEntry, 8, 16, QLatin1Char('0')));
    }
    if (!(pageTableEntry & PageTableEntryFlags::Present)) {
        vlog(LogCPU, "#PF Translating %08x {dir=%03x, page=%03x, offset=%03x} PDBR=%08x, PDE=%08x, PTE=%08x", linearAddress, dir, page, offset, getCR3(), pageDirectoryEntry, pageTableEntry);
        throw PageFault(linearAddress, makePFErrorCode(PageFaultFlags::NotPresent, accessType, inUserMode), QString("Page not present in PTE(%1)").arg(pageTableEntry, 8, 16, QLatin1Char('0')));
    }

    if (inUserMode) {
        if (!(pageDirectoryEntry & PageTableEntryFlags::UserSupervisor)) {
            vlog(LogCPU, "#PF Translating %08x {dir=%03x, page=%03x, offset=%03x} PDBR=%08x, PDE=%08x, PTE=%08x", linearAddress, dir, page, offset, getCR3(), pageDirectoryEntry, pageTableEntry);
            throw PageFault(linearAddress, makePFErrorCode(PageFaultFlags::ProtectionViolation, accessType, inUserMode), QString("Page not accessible in user mode in PDE(%1)").arg(pageDirectoryEntry, 8, 16, QLatin1Char('0')));
        }
        if (!(pageTableEntry & PageTableEntryFlags::UserSupervisor)) {
            vlog(LogCPU, "#PF Translating %08x {dir=%03x, page=%03x, offset=%03x} PDBR=%08x, PDE=%08x, PTE=%08x", linearAddress, dir, page, offset, getCR3(), pageDirectoryEntry, pageTableEntry);
            throw PageFault(linearAddress, makePFErrorCode(PageFaultFlags::ProtectionViolation, accessType, inUserMode), QString("Page not accessible in user mode in PTE(%1)").arg(pageTableEntry, 8, 16, QLatin1Char('0')));
        }
    }

    if ((inUserMode || getCR0() & CR0::WP) && accessType == MemoryAccessType::Write) {
        if (!(pageDirectoryEntry & PageTableEntryFlags::ReadWrite)) {
            vlog(LogCPU, "#PF Translating %08x {dir=%03x, page=%03x, offset=%03x} PDBR=%08x, PDE=%08x, PTE=%08x", linearAddress, dir, page, offset, getCR3(), pageDirectoryEntry, pageTableEntry);
            throw PageFault(linearAddress, makePFErrorCode(PageFaultFlags::ProtectionViolation, accessType, inUserMode), QString("Page not writable in PDE(%1)").arg(pageDirectoryEntry, 8, 16, QLatin1Char('0')));
        }
        if (!(pageTableEntry & PageTableEntryFlags::ReadWrite)) {
            vlog(LogCPU, "#PF Translating %08x {dir=%03x, page=%03x, offset=%03x} PDBR=%08x, PDE=%08x, PTE=%08x", linearAddress, dir, page, offset, getCR3(), pageDirectoryEntry, pageTableEntry);
            throw PageFault(linearAddress, makePFErrorCode(PageFaultFlags::ProtectionViolation, accessType, inUserMode), QString("Page not writable in PTE(%1)").arg(pageTableEntry, 8, 16, QLatin1Char('0')));
        }
    }

    if (accessType == MemoryAccessType::Write)
        pageTableEntry |= PageTableEntryFlags::Dirty;

    pageDirectoryEntry |= PageTableEntryFlags::Accessed;
    pageTableEntry |= PageTableEntryFlags::Accessed;

    physicalAddress = (pageTableEntry & 0xfffff000) | offset;

#ifdef DEBUG_PAGING
    vlog(LogCPU, "PG=1 Translating %08x {dir=%03x, page=%03x, offset=%03x} => %08x [%08x + %08x]", linearAddress, dir, page, offset, physicalAddress, pageDirectoryEntry, pageTableEntry);
#endif
}

static const char* toString(CPU::MemoryAccessType type)
{
    switch (type) {
    case CPU::MemoryAccessType::Read: return "Read";
    case CPU::MemoryAccessType::Write: return "Write";
    case CPU::MemoryAccessType::Execute: return "Execute";
    case CPU::MemoryAccessType::InternalPointer: return "InternalPointer";
    default: return "(wat)";
    }
}

template<typename T>
void CPU::validateAddress(const SegmentDescriptor& descriptor, DWORD offset, MemoryAccessType accessType)
{
    if (descriptor.isNull()) {
        vlog(LogAlert, "NULL! %s offset %08X into null selector (selector index: %04X)",
             toString(accessType),
             offset,
             descriptor.index());
        throw GeneralProtectionFault(0, "Access through null selector");
    }

    switch (accessType) {
    case MemoryAccessType::Read:
        if (descriptor.isCode() && !descriptor.asCodeSegmentDescriptor().readable()) {
            throw GeneralProtectionFault(0, "Attempt to read from non-readable code segment");
        }
        break;
    case MemoryAccessType::Write:
        if (!descriptor.isData()) {
            throw GeneralProtectionFault(0, "Attempt to write to non-data segment");
        }
#if 0
        // FIXME: Should we check this here? GazOS GPF's in a PUSH if we do this.
        if (!descriptor.asDataSegmentDescriptor().writable()) {
            throw GeneralProtectionFault(0, "Attempt to write to non-writable data segment");
        }
#endif
        break;
    case MemoryAccessType::Execute:
        if (!descriptor.isCode()) {
            throw GeneralProtectionFault(0, "Attempt to execute non-code segment");
        }
        break;
    default:
        break;
    }

#if 0
    // FIXME: Is this appropriate somehow? Need to figure it out. The code below as-is breaks IRET.
    if (getCPL() > descriptor.DPL()) {
        throw GeneralProtectionFault(0, QString("Insufficient privilege for access (CPL=%1, DPL=%2)").arg(getCPL()).arg(descriptor.DPL()));
    }
#endif

    DWORD offsetForLimitChecking = descriptor.granularity() ? (offset & 0xfffff000) : offset;

    if (offsetForLimitChecking > descriptor.effectiveLimit()) {
        vlog(LogAlert, "FUG! %s offset %08X outside limit (selector index: %04X, effective limit: %08X [%08X x %s])",
             toString(accessType),
             offset,
             descriptor.index(),
             descriptor.effectiveLimit(),
             descriptor.limit(),
             descriptor.granularity() ? "4K" : "1b"
             );
        //ASSERT_NOT_REACHED();
        dumpDescriptor(descriptor);
        //dumpAll();
        //debugger().enter();
        throw GeneralProtectionFault(descriptor.index(), "Access outside segment limit");
    }
}

template<typename T>
void CPU::validateAddress(SegmentRegisterIndex registerIndex, DWORD offset, MemoryAccessType accessType)
{
    validateAddress<T>(m_descriptor[(int)registerIndex], offset, accessType);
}

template<typename T>
bool CPU::validatePhysicalAddress(DWORD address, MemoryAccessType accessType)
{
    UNUSED_PARAM(accessType);
    if (address < m_memorySize)
        return true;
#ifdef MEMORY_DEBUGGING
    if (options.memdebug) {
        vlog(LogCPU, "OOB %zu-bit %s access @ physical %08x [A20=%s] [PG=%u]",
            sizeof(T) * 8,
            toString(accessType),
            address,
            isA20Enabled() ? "on" : "off",
            getPG()
        );
    }
#endif
    return false;
}

template<typename T>
T CPU::readMemory(DWORD linearAddress)
{
    DWORD physicalAddress;
    translateAddress(linearAddress, physicalAddress, MemoryAccessType::Read);
#ifdef A20_ENABLED
    physicalAddress &= a20Mask();
#endif
    if (!validatePhysicalAddress<T>(physicalAddress, MemoryAccessType::Read))
        return 0;
    T value;
    if (addressIsInVGAMemory(physicalAddress))
        value = machine().vgaMemory().read<T>(physicalAddress);
    else
        value = *reinterpret_cast<T*>(&m_memory[physicalAddress]);
#ifdef MEMORY_DEBUGGING
    if (options.memdebug || shouldLogMemoryRead(physicalAddress)) {
        if (options.novlog)
            printf("%04X:%04X: %zu-bit read [A20=%s] 0x%08X, value: %08X\n", getBaseCS(), getBaseEIP(), sizeof(T) * 8, isA20Enabled() ? "on" : "off", physicalAddress, value);
        else
            vlog(LogCPU, "%zu-bit read [A20=%s] 0x%08X, value: %08X", sizeof(T) * 8, isA20Enabled() ? "on" : "off", physicalAddress, value);
    }
#endif
    return value;
}

template<typename T, CPU::MemoryAccessType accessType>
T CPU::readMemory(const SegmentDescriptor& descriptor, DWORD offset)
{
    DWORD linearAddress = descriptor.base() + offset;
    if (!getPE()) {
        return readMemory<T>(linearAddress);
    }

    validateAddress<T>(descriptor, offset, accessType);
    DWORD physicalAddress;
    translateAddress(linearAddress, physicalAddress, accessType);

#ifdef A20_ENABLED
    physicalAddress &= a20Mask();
#endif

    if (!validatePhysicalAddress<T>(physicalAddress, accessType))
        return 0;

    T value;
    if (addressIsInVGAMemory(physicalAddress))
        value = machine().vgaMemory().read<T>(physicalAddress);
    else
        value = *reinterpret_cast<T*>(&m_memory[physicalAddress]);
#ifdef MEMORY_DEBUGGING
    if (options.memdebug || shouldLogMemoryRead(physicalAddress)) {
        if (options.novlog)
            printf("%04X:%08X: %zu-bit PE read [A20=%s] %04X:%08X (phys: %08X), value: %08X\n", getBaseCS(), getBaseEIP(), sizeof(T) * 8, isA20Enabled() ? "on" : "off", descriptor.index(), offset, physicalAddress, value);
        else
            vlog(LogCPU, "%zu-bit PE read [A20=%s] %04X:%08X (phys: %08X), value: %08X", sizeof(T) * 8, isA20Enabled() ? "on" : "off", descriptor.index(), offset, physicalAddress, value);
    }
#endif
    return value;
}

template<typename T>
T CPU::readMemory(SegmentRegisterIndex segment, DWORD offset)
{
    auto& descriptor = m_descriptor[(int)segment];
    if (!getPE())
        return readMemory<T>(descriptor.base() + offset);
    return readMemory<T>(descriptor, offset);
}

template BYTE CPU::readMemory<BYTE>(SegmentRegisterIndex, DWORD);
template WORD CPU::readMemory<WORD>(SegmentRegisterIndex, DWORD);
template DWORD CPU::readMemory<DWORD>(SegmentRegisterIndex, DWORD);

template void CPU::writeMemory<BYTE>(SegmentRegisterIndex, DWORD, BYTE);
template void CPU::writeMemory<WORD>(SegmentRegisterIndex, DWORD, WORD);
template void CPU::writeMemory<DWORD>(SegmentRegisterIndex, DWORD, DWORD);

BYTE CPU::readMemory8(DWORD address) { return readMemory<BYTE>(address); }
WORD CPU::readMemory16(DWORD address) { return readMemory<WORD>(address); }
DWORD CPU::readMemory32(DWORD address) { return readMemory<DWORD>(address); }
BYTE CPU::readMemory8(SegmentRegisterIndex segment, DWORD offset) { return readMemory<BYTE>(segment, offset); }
WORD CPU::readMemory16(SegmentRegisterIndex segment, DWORD offset) { return readMemory<WORD>(segment, offset); }
DWORD CPU::readMemory32(SegmentRegisterIndex segment, DWORD offset) { return readMemory<DWORD>(segment, offset); }

template<typename T>
void CPU::writeMemory(DWORD linearAddress, T value)
{
    DWORD physicalAddress;
    translateAddress(linearAddress, physicalAddress, MemoryAccessType::Write);
#ifdef A20_ENABLED
    physicalAddress &= a20Mask();
#endif
#ifdef MEMORY_DEBUGGING
    if (options.memdebug || shouldLogMemoryWrite(physicalAddress)) {
        if (options.novlog)
            printf("%04X:%08X: %zu-bit write [A20=%s] 0x%08X, value: %08X\n", getBaseCS(), getBaseEIP(), sizeof(T) * 8, isA20Enabled() ? "on" : "off", physicalAddress, value);
        else
            vlog(LogCPU, "%zu-bit write [A20=%s] 0x%08X, value: %08X", sizeof(T) * 8, isA20Enabled() ? "on" : "off", physicalAddress, value);
    }
#endif

    if (addressIsInVGAMemory(physicalAddress)) {
        machine().vgaMemory().write(physicalAddress, value);
        return;
    }

    *reinterpret_cast<T*>(&m_memory[physicalAddress]) = value;
    didTouchMemory(physicalAddress);
}

template<typename T>
void CPU::writeMemory(const SegmentDescriptor& descriptor, DWORD offset, T value)
{
    DWORD linearAddress = descriptor.base() + offset;
    if (!getPE()) {
        writeMemory(linearAddress, value);
        return;
    }

    validateAddress<T>(descriptor, offset, MemoryAccessType::Write);
    DWORD physicalAddress;
    translateAddress(linearAddress, physicalAddress, MemoryAccessType::Write);

    if (!validatePhysicalAddress<T>(physicalAddress, MemoryAccessType::Write))
        return;

#ifdef MEMORY_DEBUGGING
    if (options.memdebug || shouldLogMemoryWrite(physicalAddress))
        vlog(LogCPU, "%zu-bit PE write [A20=%s] %04X:%08X (phys: %08X), value: %08X", sizeof(T) * 8, isA20Enabled() ? "on" : "off", descriptor.index(), offset, physicalAddress, value);
#endif

    if (addressIsInVGAMemory(physicalAddress)) {
        machine().vgaMemory().write(physicalAddress, value);
        return;
    }
    *reinterpret_cast<T*>(&m_memory[physicalAddress]) = value;
    didTouchMemory(physicalAddress);
}

template<typename T>
void CPU::writeMemory(SegmentRegisterIndex segment, DWORD offset, T value)
{
    auto& descriptor = m_descriptor[(int)segment];
    if (!getPE())
        return writeMemory<T>(descriptor.base() + offset, value);
    return writeMemory<T>(descriptor, offset, value);
}

void CPU::writeMemory8(DWORD address, BYTE value) { writeMemory(address, value); }
void CPU::writeMemory16(DWORD address, WORD value) { writeMemory(address, value); }
void CPU::writeMemory32(DWORD address, DWORD value) { writeMemory(address, value); }
void CPU::writeMemory8(SegmentRegisterIndex segment, DWORD offset, BYTE value) { writeMemory(segment, offset, value); }
void CPU::writeMemory16(SegmentRegisterIndex segment, DWORD offset, WORD value) { writeMemory(segment, offset, value); }
void CPU::writeMemory32(SegmentRegisterIndex segment, DWORD offset, DWORD value) { writeMemory(segment, offset, value); }

void CPU::updateDefaultSizes()
{
    m_shouldRestoreSizesAfterOverride = false;

#ifdef VERBOSE_DEBUG
    bool oldO32 = m_operandSize32;
    bool oldA32 = m_addressSize32;
#endif

    auto& csDescriptor = m_descriptor[(int)SegmentRegisterIndex::CS];
    m_addressSize32 = csDescriptor.D();
    m_operandSize32 = csDescriptor.D();

#ifdef VERBOSE_DEBUG
    if (oldO32 != m_operandSize32 || oldA32 != m_addressSize32) {
        vlog(LogCPU, "updateDefaultSizes PE=%u X:%u O:%u A:%u (newCS: %04X)", getPE(), x16() ? 16 : 32, o16() ? 16 : 32, a16() ? 16 : 32, getCS());
        dumpDescriptor(csDescriptor);
    }
#endif
}

void CPU::updateStackSize()
{
#ifdef VERBOSE_DEBUG
    bool oldS32 = m_stackSize32;
#endif

    auto& ssDescriptor = m_descriptor[(int)SegmentRegisterIndex::SS];
    m_stackSize32 = ssDescriptor.D();
    ASSERT(ssDescriptor.asDataSegmentDescriptor().expandDown());

#ifdef VERBOSE_DEBUG
    if (oldS32 != m_stackSize32) {
        vlog(LogCPU, "updateStackSize PE=%u S:%u (newSS: %04x)", getPE(), s16() ? 16 : 32, getSS());
        dumpDescriptor(ssDescriptor);
    }
#endif
}

void CPU::updateCodeSegmentCache()
{
    // FIXME: We need some kind of fast pointer for fetching from CS:EIP.
}

void CPU::setCS(WORD value)
{
    setSegmentRegister(SegmentRegisterIndex::CS, value);
}

void CPU::setDS(WORD value)
{
    setSegmentRegister(SegmentRegisterIndex::DS, value);
}

void CPU::setES(WORD value)
{
    setSegmentRegister(SegmentRegisterIndex::ES, value);
}

void CPU::setSS(WORD value)
{
    setSegmentRegister(SegmentRegisterIndex::SS, value);
}

void CPU::setFS(WORD value)
{
    setSegmentRegister(SegmentRegisterIndex::FS, value);
}

void CPU::setGS(WORD value)
{
    setSegmentRegister(SegmentRegisterIndex::GS, value);
}

BYTE* CPU::memoryPointer(SegmentRegisterIndex segment, DWORD offset)
{
    auto& descriptor = m_descriptor[(int)segment];
    if (!getPE())
        return memoryPointer(descriptor.base() + offset);
    return memoryPointer(descriptor, offset);
}

BYTE* CPU::memoryPointer(const SegmentDescriptor& descriptor, DWORD offset)
{
    DWORD linearAddress = descriptor.base() + offset;
    if (!getPE())
        return memoryPointer(linearAddress);

    validateAddress<BYTE>(descriptor, offset, MemoryAccessType::InternalPointer);
    DWORD physicalAddress;
    translateAddress(linearAddress, physicalAddress, MemoryAccessType::InternalPointer);
#ifdef A20_ENABLED
    physicalAddress &= a20Mask();
#endif
#ifdef MEMORY_DEBUGGING
    if (options.memdebug || shouldLogMemoryPointer(physicalAddress))
        vlog(LogCPU, "MemoryPointer PE [A20=%s] %04X:%08X (phys: %08X)", isA20Enabled() ? "on" : "off", descriptor.index(), offset, physicalAddress);
#endif
    didTouchMemory(physicalAddress);
    return &m_memory[physicalAddress];
}

BYTE* CPU::memoryPointer(WORD segmentIndex, DWORD offset)
{
    return memoryPointer(getSegmentDescriptor(segmentIndex), offset);
}

BYTE* CPU::memoryPointer(DWORD linearAddress)
{
    DWORD physicalAddress;
    translateAddress(linearAddress, physicalAddress, MemoryAccessType::InternalPointer);
#ifdef A20_ENABLED
    physicalAddress &= a20Mask();
#endif
    didTouchMemory(physicalAddress);
#ifdef MEMORY_DEBUGGING
    if (options.memdebug || shouldLogMemoryPointer(physicalAddress)) {
        vlog(LogCPU, "MemoryPointer PE=%u [A20=%s] linear:%08x, phys:%08x",
             getPE(),
             isA20Enabled() ? "on" : "off",
             linearAddress,
             physicalAddress);
    }
#endif
    return &m_memory[physicalAddress];
}

BYTE CPU::fetchOpcodeByte()
{
    if (x32())
        return readMemory<BYTE, MemoryAccessType::Execute>(cachedDescriptor(SegmentRegisterIndex::CS), EIP++);
    else
        return readMemory<BYTE, MemoryAccessType::Execute>(cachedDescriptor(SegmentRegisterIndex::CS), IP++);
}

WORD CPU::fetchOpcodeWord()
{
    WORD w;
    if (x32()) {
        w = readMemory<WORD, MemoryAccessType::Execute>(cachedDescriptor(SegmentRegisterIndex::CS), getEIP());
        this->EIP += 2;
    } else {
        w = readMemory<WORD, MemoryAccessType::Execute>(cachedDescriptor(SegmentRegisterIndex::CS), getIP());
        this->IP += 2;
    }
    return w;
}

DWORD CPU::fetchOpcodeDWord()
{
    DWORD d;
    if (x32()) {
        d = readMemory<DWORD, MemoryAccessType::Execute>(cachedDescriptor(SegmentRegisterIndex::CS), getEIP());
        this->EIP += 4;
    } else {
        d = readMemory<DWORD, MemoryAccessType::Execute>(cachedDescriptor(SegmentRegisterIndex::CS), getIP());
        this->IP += 4;
    }
    return d;
}

void CPU::_CPUID(Instruction&)
{
    if (getEAX() == 0) {
        setEBX(0x706d6f43);
        setEDX(0x6f727475);
        setECX(0x3638586e);
        return;
    }
}

void CPU::_LOCK(Instruction&)
{
}

void CPU::initWatches()
{
}
