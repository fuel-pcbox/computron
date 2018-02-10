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
#include "machine.h"
#include "vomit.h"
#include "debug.h"
#include "debugger.h"
#include "vga_memory.h"
#include "pic.h"
#include "settings.h"
#include <QtCore/QStringList>
#include <unistd.h>
#include "pit.h"

#define CRASH_ON_OPCODE_00_00
//#define SLACKWARE33_DEBUG

inline bool hasA20Bit(DWORD address)
{
    return address & 0x100000;
}

static bool shouldLogAllMemoryAccesses(DWORD address)
{
#ifdef SLACKWARE33_DEBUG
    //if (address >= 0x100000) return true;
    //if (address >= 0x100000 && address <= 0x100100) return true;
    if (address == 0x070510) return true;
    //if (address >= 0x00004380 && address <= 0x00004480) return true;
#endif
#ifdef VOMIT_DETERMINISTIC
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

VCpu* g_cpu = 0;

template<typename T>
T VCpu::readRegister(int registerIndex)
{
    if (sizeof(T) == 1)
        return *treg8[registerIndex];
    if (sizeof(T) == 2)
        return *treg16[registerIndex];
    if (sizeof(T) == 4)
        return *treg32[registerIndex];
    VM_ASSERT(false);
}

template<typename T>
void VCpu::writeRegister(int registerIndex, T value)
{
    if (sizeof(T) == 1)
        *treg8[registerIndex] = value;
    else if (sizeof(T) == 2)
        *treg16[registerIndex] = value;
    else if (sizeof(T) == 4)
        *treg32[registerIndex] = value;
    else
        VM_ASSERT(false);
}

template BYTE VCpu::readRegister<BYTE>(int);
template WORD VCpu::readRegister<WORD>(int);
template DWORD VCpu::readRegister<DWORD>(int);
template void VCpu::writeRegister<BYTE>(int, BYTE);
template void VCpu::writeRegister<WORD>(int, WORD);
template void VCpu::writeRegister<DWORD>(int, DWORD);

void VCpu::_UD0(Instruction&)
{
    vlog(LogAlert, "Undefined opcode 0F FF (UD0)");
    exception(6);
}

void VCpu::_OperationSizeOverride(Instruction&)
{
    m_shouldRestoreSizesAfterOverride = true;
    bool prevOperationSize = m_operationSize32;
    m_operationSize32 = !m_operationSize32;

#ifdef VOMIT_DEBUG_OVERRIDE_OPCODES
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
        VM_ASSERT(m_operationSize32 != prevOperationSize);
        m_operationSize32 = prevOperationSize;
    }
}

void VCpu::_AddressSizeOverride(Instruction&)
{
    m_shouldRestoreSizesAfterOverride = true;
    bool prevAddressSize32 = m_addressSize32;
    m_addressSize32 = !m_addressSize32;

#ifdef VOMIT_DEBUG_OVERRIDE_OPCODES
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
        VM_ASSERT(m_addressSize32 != prevAddressSize32);
        m_addressSize32 = prevAddressSize32;
    }
}

BYTE VCpu::readInstruction8()
{
    return fetchOpcodeByte();
}

WORD VCpu::readInstruction16()
{
    return fetchOpcodeWord();
}

DWORD VCpu::readInstruction32()
{
    return fetchOpcodeDWord();
}

void VCpu::decodeNext()
{
#ifdef VOMIT_TRACE
    if (machine().isForAutotest())
        dumpTrace();
#endif

    auto insn = Instruction::fromStream(*this);
    if (!insn.isValid())
        exception(6);
    else
        execute(std::move(insn));
}

void VCpu::execute(Instruction&& insn)
{
#ifdef CRASH_ON_OPCODE_00_00
    if (insn.op() == 0 && insn.rm() == 0) {
        dumpTrace();
        VM_ASSERT(false);
    }
 #endif

    insn.execute(*this);
}

void VCpu::_VKILL(Instruction&)
{
    vlog(LogCPU, "0xF1: Secret shutdown command received!");
    //dumpAll();
    vomit_exit(0);
}

void VCpu::GP(int code)
{
    vlog(LogCPU, "#GP(%d) :-(", code);
    exception(13);
    debugger().enter();
    //vomit_exit(1);
}

VCpu::VCpu(Machine& m)
    : m_machine(m)
    , m_shouldBreakOutOfMainLoop(false)
{
    buildOpcodeTablesIfNeeded();

    VM_ASSERT(!g_cpu);
    g_cpu = this;

    m_memory = new BYTE[m_memorySize];
    if (!m_memory) {
        vlog(LogInit, "Insufficient memory available.");
        vomit_exit(1);
    }

    memset(m_memory, 0, 1048576 + 65536);

    m_debugger = make<Debugger>(*this);

    m_controlRegisterMap[0] = &this->CR0;
    m_controlRegisterMap[1] = &this->CR1;
    m_controlRegisterMap[2] = &this->CR2;
    m_controlRegisterMap[3] = &this->CR3;
    m_controlRegisterMap[4] = &this->CR4;
    m_controlRegisterMap[5] = &this->CR5;
    m_controlRegisterMap[6] = &this->CR6;
    m_controlRegisterMap[7] = &this->CR7;

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

void VCpu::reset()
{
    m_a20Enabled = false;

    memset(&regs, 0, sizeof(regs));
    this->CR0 = 0;
    setCS(0);
    setDS(0);
    setES(0);
    setSS(0);
    setFS(0);
    setGS(0);
    this->CR1 = 0;
    this->CR2 = 0;
    this->CR3 = 0;
    this->CR4 = 0;
    this->CR5 = 0;
    this->CR6 = 0;
    this->CR7 = 0;
    this->DR0 = 0;
    this->DR1 = 0;
    this->DR2 = 0;
    this->DR3 = 0;
    this->DR4 = 0;
    this->DR5 = 0;
    this->DR6 = 0;
    this->DR7 = 0;

    this->IOPL = 0;
    this->VM = 0;
    this->VIP = 0;
    this->VIF = 0;
    this->CPL = 0;

    this->GDTR.base = 0;
    this->GDTR.limit = 0;
    this->IDTR.base = 0;
    this->IDTR.limit = 0;
    this->LDTR.base = 0;
    this->LDTR.limit = 0;
    this->LDTR.segment = 0;

    memset(m_selector, 0, sizeof(m_selector));

    m_segmentPrefix = SegmentRegisterIndex::None;

    if (machine().isForAutotest())
        jump32(machine().settings().entryCS(), machine().settings().entryIP());
    else
        jump32(0xF000, 0x00000000);

    setFlags(0x0200);

    setIOPL(3);

    m_state = Alive;

    m_addressSize32 = false;
    m_operationSize32 = false;

    initWatches();
}

VCpu::~VCpu()
{
    delete [] m_memory;
    m_memory = nullptr;
    m_codeMemory = nullptr;
}

void VCpu::exec()
{
    saveBaseAddress();
    decodeNext();
}

void VCpu::haltedLoop()
{
    while (state() == VCpu::Halted) {
#ifdef HAVE_USLEEP
        usleep(500);
#endif
        if (PIC::hasPendingIRQ())
            PIC::serviceIRQ(*this);
    }
}

void VCpu::queueCommand(Command command)
{
    QMutexLocker locker(&m_commandMutex);
    m_commandQueue.enqueue(command);
}

void VCpu::flushCommandQueue()
{
    // FIXME: Blocking on the mutex on every pass through here is dumb.
    //return;
    static int xxx = 0;
    if (++xxx >= 100) {
        usleep(1);
        xxx = 0;
    }

    QMutexLocker locker(&m_commandMutex);
    while (!m_commandQueue.isEmpty()) {
        switch (m_commandQueue.dequeue()) {
        case ExitMainLoop:
            m_shouldBreakOutOfMainLoop = true;
            break;
        case EnterMainLoop:
            m_shouldBreakOutOfMainLoop = false;
            break;
        case HardReboot:
            m_shouldHardReboot = true;
            break;
        case SoftReboot:
            m_shouldSoftReboot = true;
            break;
        }
    }
}

void VCpu::mainLoop()
{
    forever {

        // FIXME: Throttle this so we don't spend the majority of CPU time in locking/unlocking this mutex.
        flushCommandQueue();
        if (m_shouldBreakOutOfMainLoop)
            return;

        if (m_shouldSoftReboot) {
            setA20Enabled(false);
            setControlRegister(0, 0);
            jump32(0xF000, 0x0);
            m_shouldSoftReboot = false;
            continue;
        }

        if (m_shouldHardReboot) {
            machine().resetAllIODevices();
            reset();
            m_shouldHardReboot = false;
            continue;
        }

#ifdef VOMIT_DEBUG

        if (!m_breakpoints.empty()) {
            DWORD flatPC = vomit_toFlatAddress(getCS(), getEIP());
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
#endif

#ifdef VOMIT_TRACE
        if (options.trace)
            dumpTrace();
#endif

        if (!m_watches.isEmpty())
            dumpWatches();

#ifdef SLACKWARE33_DEBUG
        if (1 && CS == 0x10 && getBaseEIP() == 0x00001A41) {
            DWORD src = 0x70510 + getESI();
            VM_ASSERT(src == getEAX());
            DWORD dest = getEDX();
            DWORD n = getEBX();
            BYTE* m = &m_memory[src];
            DWORD inflate_dynamic_EBP = 0x0007C8B4;
            DWORD nlAddr = inflate_dynamic_EBP;
            nlAddr += 0xfffffaf0;
            DWORD ndAddr = inflate_dynamic_EBP;
            ndAddr += 0xfffffaec;
            vlog(LogCPU, "memcpy(%08X, %08X, %u) { %02X %02X %02X %02X %02X %02X %02X %02X } w=%08X, d=%08X, EBP=%08X, nl=%u, nd=%u",
                 src, dest, n,
                 m[0], m[1], m[2], m[3],
                 m[4], m[5], m[6], m[7],
                 getESI(),
                 readMemory32(getEBP() - 8),
                 getEBP(),
                 readMemory32(nlAddr),
                 readMemory32(ndAddr)
            );
        }
#endif

        // Fetch & decode AKA execute the next instruction.
        exec();

        if (getTF()) {
            // The Trap Flag is set, so we'll execute one instruction and
            // call ISR 1 as soon as it's finished.
            //
            // This is used by tools like DEBUG to implement step-by-step
            // execution :-)
            jumpToInterruptHandler(1);

            // NOTE: jumpToInterruptHandler() just set IF=0.
        }

        if (getIF() && PIC::hasPendingIRQ())
            PIC::serviceIRQ(*this);

#ifdef VOMIT_DETERMINISTIC
        if (getIF() && ((cycle() + 1) % 100 == 0)) {
            machine().pit().raiseIRQ();
        }
#endif
    }
}

void VCpu::jumpRelative8(SIGNED_BYTE displacement)
{
    if (x32())
        this->EIP += displacement;
    else
        this->IP += displacement;
}

void VCpu::jumpRelative16(SIGNED_WORD displacement)
{
    if (x32())
        this->EIP += displacement;
    else
        this->IP += displacement;
}

void VCpu::jumpRelative32(SIGNED_DWORD displacement)
{
    this->EIP += displacement;
}

void VCpu::jumpAbsolute16(WORD address)
{
    if (x32())
        this->EIP = address;
    else
        this->IP = address;
#ifdef VOMIT_DETERMINISTIC
    ++m_cycle;
#endif
}

void VCpu::jumpAbsolute32(DWORD address)
{
//    vlog(LogCPU, "[PE=%u] Abs jump to %08X", getPE(), address);
    this->EIP = address;
}

void VCpu::jump32(WORD segment, DWORD offset)
{
    //vlog(LogCPU, "[PE=%u] Far jump to %04X:%08X", getPE(), segment, offset);
    // Jump to specified modrm.
    setCS(segment);
    this->EIP = offset;

    // Point m_codeMemory to CS:0 for fast opcode fetching.
    m_codeMemory = memoryPointer(SegmentRegisterIndex::CS, 0);

    updateSizeModes();
}

void VCpu::jump16(WORD segment, WORD offset)
{
    jump32(segment, offset);
}

void VCpu::_UNSUPP(Instruction& insn)
{
    // We've come across an unsupported instruction, log it, then vector to the "illegal instruction" ISR.
    vlog(LogAlert, "Unsupported opcode %02X", insn.op());
    QString ndis = "db ";
    DWORD baseEIP = getBaseEIP();
    QStringList dbs;
    for (int i = 0; i < 16; ++i) {
        QString s;
        s.sprintf("0x%02X", codeMemory()[baseEIP + i]);
        dbs.append(s);
    }
    ndis.append(dbs.join(", "));
    vlog(LogAlert, qPrintable(ndis));
    dumpAll();
    exception(6);
}

void VCpu::_NOP(Instruction&)
{
}

void VCpu::_HLT(Instruction&)
{
    setState(VCpu::Halted);

    if (!getIF()) {
        vlog(LogCPU, "Halted with IF=0");
        vomit_exit(0);
    } else
        vlog(LogCPU, "Halted");

    haltedLoop();
}

void VCpu::_XLAT(Instruction&)
{
    if (a32())
        setAL(readMemory8(currentSegment(), getEBX() + getAL()));
    else
        setAL(readMemory8(currentSegment(), getBX() + getAL()));
}

void VCpu::_CS(Instruction&)
{
    setSegmentPrefix(SegmentRegisterIndex::CS);
    decodeNext();
    resetSegmentPrefix();
}

void VCpu::_DS(Instruction&)
{
    setSegmentPrefix(SegmentRegisterIndex::DS);
    decodeNext();
    resetSegmentPrefix();
}

void VCpu::_ES(Instruction&)
{
    setSegmentPrefix(SegmentRegisterIndex::ES);
    decodeNext();
    resetSegmentPrefix();
}

void VCpu::_SS(Instruction&)
{
    setSegmentPrefix(SegmentRegisterIndex::SS);
    decodeNext();
    resetSegmentPrefix();
}

void VCpu::_FS(Instruction&)
{
    setSegmentPrefix(SegmentRegisterIndex::FS);
    decodeNext();
    resetSegmentPrefix();
}

void VCpu::_GS(Instruction&)
{
    setSegmentPrefix(SegmentRegisterIndex::GS);
    decodeNext();
    resetSegmentPrefix();
}

void VCpu::_XCHG_AX_reg16(Instruction& insn)
{
    qSwap(insn.reg16(), regs.W.AX);
}

void VCpu::_XCHG_EAX_reg32(Instruction& insn)
{
    qSwap(insn.reg32(), regs.D.EAX);
}

void VCpu::_XCHG_reg8_RM8(Instruction& insn)
{
    auto& modrm = insn.modrm();
    auto tmp = insn.reg8();
    insn.reg8() = modrm.read8();
    modrm.write8(tmp);
}

void VCpu::_XCHG_reg16_RM16(Instruction& insn)
{
    auto& modrm = insn.modrm();
    auto tmp = insn.reg16();
    insn.reg16() = modrm.read16();
    modrm.write16(tmp);
}

void VCpu::_XCHG_reg32_RM32(Instruction& insn)
{
    auto& modrm = insn.modrm();
    auto tmp = insn.reg32();
    insn.reg32() = modrm.read32();
    modrm.write32(tmp);
}

void VCpu::_DEC_reg16(Instruction& insn)
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

void VCpu::_DEC_reg32(Instruction& insn)
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

void VCpu::_INC_reg16(Instruction& insn)
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

void VCpu::_INC_reg32(Instruction& insn)
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

void VCpu::_INC_RM16(Instruction& insn)
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

void VCpu::_INC_RM32(Instruction& insn)
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

void VCpu::_DEC_RM16(Instruction& insn)
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

void VCpu::_DEC_RM32(Instruction& insn)
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

void VCpu::_INC_RM8(Instruction& insn)
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

void VCpu::_DEC_RM8(Instruction& insn)
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

void VCpu::_LDS_reg16_mem16(Instruction& insn)
{
    VM_ASSERT(a16());
    WORD* ptr = static_cast<WORD*>(insn.modrm().memoryPointer());
    insn.reg16() = vomit_read16FromPointer(ptr);
    setDS(vomit_read16FromPointer(ptr + 1));
}

void VCpu::_LDS_reg32_mem32(Instruction&)
{
#warning FIXME: need readModRM48
    vlog(LogAlert, "LDS reg32 mem32");
    vomit_exit(0);
}

bool VCpu::x32() const
{
    if (!getPE())
        return false;

    return m_selector[(int)SegmentRegisterIndex::CS]._32bit;
}

void VCpu::pushInstructionPointer()
{
    if (o32())
        push32(getEIP());
    else
        push(getIP());
}

void VCpu::_LES_reg16_mem16(Instruction& insn)
{
    VM_ASSERT(a16());
    WORD* ptr = static_cast<WORD*>(insn.modrm().memoryPointer());
    insn.reg16() = vomit_read16FromPointer(ptr);
    setES(vomit_read16FromPointer(ptr + 1));
}

void VCpu::_LES_reg32_mem32(Instruction&)
{
#warning FIXME: need readModRM48
    vlog(LogAlert, "LES reg32 mem32");
    vomit_exit(0);
}

void VCpu::_LFS_reg16_mem16(Instruction& insn)
{
    VM_ASSERT(a16());
    WORD* ptr = static_cast<WORD*>(insn.modrm().memoryPointer());
    insn.reg16() = vomit_read16FromPointer(ptr);
    setFS(vomit_read16FromPointer(ptr + 1));
}

void VCpu::_LFS_reg32_mem32(Instruction&)
{
#warning FIXME: need readModRM48
    vlog(LogAlert, "LFS reg32 mem32");
    vomit_exit(0);
}

void VCpu::_LSS_reg16_mem16(Instruction& insn)
{
    VM_ASSERT(a16());
    WORD* ptr = static_cast<WORD*>(insn.modrm().memoryPointer());
    insn.reg16() = vomit_read16FromPointer(ptr);
    setSS(vomit_read16FromPointer(ptr + 1));
}

void VCpu::_LSS_reg32_mem32(Instruction& insn)
{
    VM_ASSERT(a32());
    FarPointer ptr = readModRMFarPointerOffsetFirst(insn.modrm());
    insn.reg32() = ptr.offset;
    setSS(ptr.segment);
}

void VCpu::_LGS_reg16_mem16(Instruction& insn)
{
    VM_ASSERT(a16());
    WORD* ptr = static_cast<WORD*>(insn.modrm().memoryPointer());
    insn.reg16() = vomit_read16FromPointer(ptr);
    setGS(vomit_read16FromPointer(ptr + 1));
}

void VCpu::_LGS_reg32_mem32(Instruction&)
{
#warning FIXME: need readModRM48
    vlog(LogAlert, "LGS reg32 mem32");
    vomit_exit(0);
}

void VCpu::_LEA_reg32_mem32(Instruction& insn)
{
    auto& modrm = insn.modrm();
    if (modrm.isRegister()) {
        vlog(LogAlert, "LEA_reg32_mem32 with register source!");
        exception(6);
        return;
    }

    insn.reg32() = modrm.offset();
}

void VCpu::_LEA_reg16_mem16(Instruction& insn)
{
    auto& modrm = insn.modrm();
    if (modrm.isRegister()) {
        vlog(LogAlert, "LEA_reg16_mem16 with register source!");
        exception(6);
        return;
    }

    insn.reg16() = modrm.offset();
}
static const char* toString(VCpu::MemoryAccessType type)
{
    switch (type) {
    case VCpu::MemoryAccessType::Read: return "Read";
    case VCpu::MemoryAccessType::Write: return "Write";
    default: return "(wat)";
    }
}

template<typename T>
bool VCpu::validateAddress(const SegmentSelector& selector, DWORD offset, MemoryAccessType accessType)
{
    VM_ASSERT(getPE());

    if (offset > selector.effectiveLimit()) {
        vlog(LogAlert, "FUG! offset %08X outside limit", offset);
        dumpSegment(selector);
        //dumpAll();
        debugger().enter();
        //GP(0);
        return false;
    }
    assert(offset <= selector.effectiveLimit());

    DWORD flatAddress = selector.base + offset;
    VM_ASSERT(isA20Enabled() || !hasA20Bit(flatAddress));

    if (flatAddress >= m_memorySize) {
        vlog(LogCPU, "OOB %u-bit %s access @ %04x:%08x {base:%08x,limit:%08x,gran:%s} (flat: 0x%08x) [A20=%s]",
             sizeof(T) * 8,
             toString(accessType),
             selector.index,
             offset,
             selector.base,
             selector.limit,
             selector.granularity ? "4k" : "1b",
             flatAddress,
             isA20Enabled() ? "on" : "off"
        );
        return false;
    }
    return true;
}

template<typename T>
bool VCpu::validateAddress(SegmentRegisterIndex registerIndex, DWORD offset, MemoryAccessType accessType)
{
    return validateAddress<T>(m_selector[(int)registerIndex], offset, accessType);
}

template<typename T>
bool VCpu::validateAddress(WORD segmentIndex, DWORD offset, MemoryAccessType accessType)
{
    return validateAddress<T>(makeSegmentSelector(segmentIndex), offset, accessType);
}

template<typename T>
T VCpu::readMemory(DWORD address)
{
    address &= a20Mask();
    T value;
    if (addressIsInVGAMemory(address))
        value = machine().vgaMemory().read<T>(address);
    else
        value = *reinterpret_cast<T*>(&m_memory[address]);
    if (options.memdebug || shouldLogMemoryRead(address)) {
        if (options.novlog)
            printf("%04X:%04X: %u-bit read [A20=%s] 0x%08X, value: %08X\n", getBaseCS(), getBaseEIP(), sizeof(T) * 8, isA20Enabled() ? "on" : "off", address, value);
        else
            vlog(LogCPU, "%u-bit read [A20=%s] 0x%08X, value: %08X", sizeof(T) * 8, isA20Enabled() ? "on" : "off", address, value);
    }
    return value;
}

template<typename T>
T VCpu::readMemory(const SegmentSelector& selector, DWORD offset)
{
    if (getPE()) {
        if (!validateAddress<T>(selector, offset, MemoryAccessType::Read)) {
            //VM_ASSERT(false);
            return 0;
        }
        DWORD flatAddress = selector.base + offset;
        T value = *reinterpret_cast<T*>(&m_memory[flatAddress]);
        if (options.memdebug || shouldLogMemoryRead(flatAddress)) {
            if (options.novlog)
                printf("%04X:%08X: %u-bit PE read [A20=%s] %04X:%08X (flat: %08X), value: %08X\n", getBaseCS(), getBaseEIP(), sizeof(T) * 8, isA20Enabled() ? "on" : "off", selector.index, offset, flatAddress, value);
            else
                vlog(LogCPU, "%u-bit PE read [A20=%s] %04X:%08X (flat: %08X), value: %08X", sizeof(T) * 8, isA20Enabled() ? "on" : "off", selector.index, offset, flatAddress, value);
        }
        return value;
    }
    return readMemory<T>(vomit_toFlatAddress(selector.index, offset));
}

template<typename T>
T VCpu::readMemory(WORD segmentIndex, DWORD offset)
{
    return readMemory<T>(makeSegmentSelector(segmentIndex), offset);
}

template<typename T>
T VCpu::readMemory(SegmentRegisterIndex segment, DWORD offset)
{
    auto& selector = m_selector[(int)segment];
    if (!getPE())
        return readMemory<T>(selector.base + offset);
    return readMemory<T>(selector, offset);
}

BYTE VCpu::readMemory8(DWORD address) { return readMemory<BYTE>(address); }
WORD VCpu::readMemory16(DWORD address) { return readMemory<WORD>(address); }
DWORD VCpu::readMemory32(DWORD address) { return readMemory<DWORD>(address); }
BYTE VCpu::readMemory8(WORD segmentIndex, DWORD offset) { return readMemory<BYTE>(segmentIndex, offset); }
WORD VCpu::readMemory16(WORD segmentIndex, DWORD offset) { return readMemory<WORD>(segmentIndex, offset); }
DWORD VCpu::readMemory32(WORD segmentIndex, DWORD offset) { return readMemory<DWORD>(segmentIndex, offset); }
BYTE VCpu::readMemory8(SegmentRegisterIndex segment, DWORD offset) { return readMemory<BYTE>(segment, offset); }
WORD VCpu::readMemory16(SegmentRegisterIndex segment, DWORD offset) { return readMemory<WORD>(segment, offset); }
DWORD VCpu::readMemory32(SegmentRegisterIndex segment, DWORD offset) { return readMemory<DWORD>(segment, offset); }

template<typename T>
void VCpu::writeMemory(DWORD address, T value)
{
    assert(!getPE());
    address &= a20Mask();

    if (options.memdebug || shouldLogMemoryWrite(address)) {
        if (options.novlog)
            printf("%04X:%08X: %u-bit write [A20=%s] 0x%08X, value: %08X\n", getBaseCS(), getBaseEIP(), sizeof(T) * 8, isA20Enabled() ? "on" : "off", address, value);
        else
            vlog(LogCPU, "%u-bit write [A20=%s] 0x%08X, value: %08X", sizeof(T) * 8, isA20Enabled() ? "on" : "off", address, value);
    }

    if (addressIsInVGAMemory(address)) {
        machine().vgaMemory().write(address, value);
        return;
    }

    *reinterpret_cast<T*>(&m_memory[address]) = value;
}

template<typename T>
void VCpu::writeMemory(WORD segmentIndex, DWORD offset, T value)
{
    if (getPE()) {
        if (!validateAddress<T>(segmentIndex, offset, MemoryAccessType::Write)) {
            //VM_ASSERT(false);
            return;
        }
        SegmentSelector segment = makeSegmentSelector(segmentIndex);
        DWORD flatAddress = segment.base + offset;
        assert(!addressIsInVGAMemory(flatAddress));
        if (options.memdebug || shouldLogMemoryWrite(flatAddress))
            vlog(LogCPU, "%u-bit PE write [A20=%s] %04X:%08X (flat: %08X), value: %08X", sizeof(T) * 8, isA20Enabled() ? "on" : "off", segmentIndex, offset, flatAddress, value);
        *reinterpret_cast<T*>(&m_memory[flatAddress]) = value;
        return;
    }
    writeMemory(vomit_toFlatAddress(segmentIndex, offset), value);
}

template<typename T>
void VCpu::writeMemory(SegmentRegisterIndex segment, DWORD offset, T value)
{
    auto& selector = m_selector[(int)segment];
    if (!getPE())
        return writeMemory<T>(selector.base + offset, value);
    return writeMemory<T>(getSegment(segment), offset, value);
}

void VCpu::writeMemory8(DWORD address, BYTE value) { writeMemory(address, value); }
void VCpu::writeMemory16(DWORD address, WORD value) { writeMemory(address, value); }
void VCpu::writeMemory32(DWORD address, DWORD value) { writeMemory(address, value); }
void VCpu::writeMemory8(WORD segment, DWORD offset, BYTE value) { writeMemory(segment, offset, value); }
void VCpu::writeMemory16(WORD segment, DWORD offset, WORD value) { writeMemory(segment, offset, value); }
void VCpu::writeMemory32(WORD segment, DWORD offset, DWORD value) { writeMemory(segment, offset, value); }
void VCpu::writeMemory8(SegmentRegisterIndex segment, DWORD offset, BYTE value) { writeMemory(segment, offset, value); }
void VCpu::writeMemory16(SegmentRegisterIndex segment, DWORD offset, WORD value) { writeMemory(segment, offset, value); }
void VCpu::writeMemory32(SegmentRegisterIndex segment, DWORD offset, DWORD value) { writeMemory(segment, offset, value); }

void VCpu::updateSizeModes()
{
    m_shouldRestoreSizesAfterOverride = false;

    if (!getPE()) {
        m_addressSize32 = false;
        m_operationSize32 = false;
        return;
    }

    //SegmentSelector& codeSegment = m_selector[SegmentRegisterIndex::CS];
    auto codeSegment = makeSegmentSelector(CS);
    m_addressSize32 = codeSegment._32bit;
    m_operationSize32 = codeSegment._32bit;
    vlog(LogCPU, "PE=%u X:%u O:%u A:%u (newCS: %04X)", getPE(), x16() ? 16 : 32, o16() ? 16 : 32, a16() ? 16 : 32, CS);
}

void VCpu::setCS(WORD value)
{
    this->CS = value;
    syncSegmentRegister(SegmentRegisterIndex::CS);
}

void VCpu::setDS(WORD value)
{
    this->DS = value;
    syncSegmentRegister(SegmentRegisterIndex::DS);
}

void VCpu::setES(WORD value)
{
    this->ES = value;
    syncSegmentRegister(SegmentRegisterIndex::ES);
}

void VCpu::setSS(WORD value)
{
    this->SS = value;
    syncSegmentRegister(SegmentRegisterIndex::SS);
}

void VCpu::setFS(WORD value)
{
    this->FS = value;
    syncSegmentRegister(SegmentRegisterIndex::FS);
}

void VCpu::setGS(WORD value)
{
    this->GS = value;
    syncSegmentRegister(SegmentRegisterIndex::GS);
}

BYTE* VCpu::memoryPointer(SegmentRegisterIndex segment, DWORD offset)
{
    auto& selector = m_selector[(int)segment];
    if (!getPE())
        return memoryPointer(selector.base + offset);
    return memoryPointer(getSegment(segment), offset);
}

BYTE* VCpu::memoryPointer(WORD segmentIndex, DWORD offset)
{
    if (getPE()) {
        if (!validateAddress<BYTE>(segmentIndex, offset, MemoryAccessType::Read)) {
            //VM_ASSERT(false);
            return nullptr;
        }
        SegmentSelector segment = makeSegmentSelector(segmentIndex);
        DWORD flatAddress = segment.base + offset;
        if (options.memdebug || shouldLogMemoryPointer(flatAddress))
            vlog(LogCPU, "MemoryPointer PE [A20=%s] %04X:%08X (flat: %08X)", isA20Enabled() ? "on" : "off", segmentIndex, offset, flatAddress);
        return &m_memory[flatAddress];
    }
    return memoryPointer(vomit_toFlatAddress(segmentIndex, offset));
}

BYTE* VCpu::memoryPointer(DWORD address)
{
    address &= a20Mask();
    return &m_memory[address];
}

BYTE VCpu::fetchOpcodeByte()
{
    if (x32())
        return m_codeMemory[this->EIP++];
    else
        return m_codeMemory[this->IP++];
#if 0
    BYTE b = readMemory8(getCS(), getEIP());
    this->IP += 1;
    return b;
#endif
}

WORD VCpu::fetchOpcodeWord()
{
    WORD w;
    if (x32()) {
        w = *reinterpret_cast<WORD*>(&m_codeMemory[getEIP()]);
        this->EIP += 2;
    } else {
        w = *reinterpret_cast<WORD*>(&m_codeMemory[getIP()]);
        this->IP += 2;
    }
    return w;
#if 0
    WORD w = readMemory16(getCS(), getEIP());
    this->IP += 2;
    return w;
#endif
}

DWORD VCpu::fetchOpcodeDWord()
{
    DWORD d;
    if (x32()) {
        d = *reinterpret_cast<DWORD*>(&m_codeMemory[getEIP()]);
        this->EIP += 4;
    } else {
        d = *reinterpret_cast<DWORD*>(&m_codeMemory[getIP()]);
        this->IP += 4;
    }
    return d;
#if 0
    DWORD d = readMemory32(getCS(), getEIP());
    this->IP += 4;
    return d;
#endif
}

void VCpu::_CPUID(Instruction&)
{
    if (getEAX() == 0) {
        // 56 6f 6d 69 74 4d 61 63 68 69 6e 65
        setEBX(0x566f6d69);
        setEDX(0x744d6163);
        setECX(0x68696e65);
        return;
    }
}

void VCpu::initWatches()
{
#ifdef SLACKWARE33_DEBUG
    //m_watches.append(WatchedAddress{ name, address, size, breakOnChange? });
    m_watches.append(WatchedAddress{ "inbuf", 0x7050C, DWordSize });
    m_watches.append(WatchedAddress{ "input_len", 0x437C, DWordSize });
    m_watches.append(WatchedAddress{ "insize", 0x41A8, DWordSize });
    //m_watches.append(WatchedAddress{ "inptr", 0x41AC, DWordSize });
    //m_watches.append(WatchedAddress{ "wp (outcnt)", 0x41B0, DWordSize });
    m_watches.append(WatchedAddress{ "bk (bb bits)", 0x78528, DWordSize });
    m_watches.append(WatchedAddress{ "bb (bit buffer)", 0x78524, DWordSize });
    m_watches.append(WatchedAddress{ "hufts (mem)", 0x7852c, DWordSize });
    //m_watches.append(WatchedAddress{ "bytes_out", 0x41b4, DWordSize });
    m_watches.append(WatchedAddress{ "output_ptr", 0x41b8, DWordSize });
    m_watches.append(WatchedAddress{ "free_mem_ptr", 0x41bc, DWordSize });
    //m_watches.append(WatchedAddress{ "lbits", 0x4330, DWordSize });
    //m_watches.append(WatchedAddress{ "dbits", 0x4334, DWordSize });
    //m_watches.append(WatchedAddress{ "output_buffer", 0x78510, DWordSize });
#endif
}
