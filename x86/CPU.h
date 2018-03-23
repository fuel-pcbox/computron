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

#pragma once

#include "Common.h"
#include "debug.h"
#include <QtCore/QVector>
#include <set>
#include "OwnPtr.h"
#include "Instruction.h"
#include "Descriptor.h"

class Debugger;
class Machine;
class CPU;
class TSS;
class VGAMemory;

struct FarPointer {
    FarPointer() : segment(0), offset(0) { }
    FarPointer(WORD s, DWORD o) : segment(s), offset(o) { }

    WORD segment;
    DWORD offset;
};

struct WatchedAddress {
    WatchedAddress() { }
    WatchedAddress(QString n, DWORD a, ValueSize s, bool b = false) : name(n), address(a), size(s), breakOnChange(b) { }
    QString name;
    DWORD address { 0xBEEFBABE };
    ValueSize size { ByteSize };
    bool breakOnChange { false };
    static const QWORD neverSeen = 0xFFFFFFFFFFFFFFFF;
    QWORD lastSeenValue { neverSeen };
};

enum class JumpType { Internal, IRET, RETF, INT, CALL, JMP };


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

struct HardwareInterruptDuringREP { };

class Exception {
public:
    Exception(BYTE num, WORD code, DWORD address, const QString& reason)
        : m_num(num)
        , m_code(code)
        , m_address(address)
        , m_hasCode(true)
        , m_reason(reason)
    {
    }

    Exception(BYTE num, WORD code, const QString& reason)
        : m_num(num)
        , m_code(code)
        , m_hasCode(true)
        , m_reason(reason)
    {
    }

    Exception(BYTE num, const QString& reason)
        : m_num(num)
        , m_hasCode(false)
        , m_reason(reason)
    {
    }

    ~Exception() { }

    BYTE num() const { return m_num; }
    WORD code() const { return m_code; }
    bool hasCode() const { return m_hasCode; }
    DWORD address() const { return m_address; }
    QString reason() const { return m_reason; }

private:
    BYTE m_num { 0 };
    WORD m_code { 0 };
    DWORD m_address { 0 };
    bool m_hasCode { false };
    QString m_reason;
};

class CPU final : public InstructionStream {
    friend void buildOpcodeTablesIfNeeded();
public:
    explicit CPU(Machine&);
    ~CPU();

    struct Flag {
    enum Flags {
        CF = 0x0001,
        PF = 0x0004,
        AF = 0x0010,
        ZF = 0x0040,
        SF = 0x0080,
        TF = 0x0100,
        IF = 0x0200,
        DF = 0x0400,
        OF = 0x0800,
        IOPL = 0x3000, // Note: this is a 2-bit field
        NT = 0x4000,
        RF = 0x10000,
        VM = 0x20000,
        AC = 0x40000,
        VIF = 0x80000,
        VIP = 0x100000,
        ID = 0x200000,
    };
    };

    struct CR0 {
    enum Bits {
        PE = 1 << 0,
        TS = 1 << 3,
        WP = 1 << 16,
        PG = 1 << 31,
    };
    };

    void recomputeMainLoopNeedsSlowStuff();

    QWORD cycle() const { return m_cycle; }

    void reset();

    Machine& machine() const { return m_machine; }

    std::set<DWORD>& breakpoints() { return m_breakpoints; }

    enum class MemoryAccessType { Read, Write, Execute, InternalPointer };

    enum RegisterIndex8 {
        RegisterAL = 0,
        RegisterCL,
        RegisterDL,
        RegisterBL,
        RegisterAH,
        RegisterCH,
        RegisterDH,
        RegisterBH
    };

    enum RegisterIndex16 {
        RegisterAX = 0,
        RegisterCX,
        RegisterDX,
        RegisterBX,
        RegisterSP,
        RegisterBP,
        RegisterSI,
        RegisterDI
    };

    enum RegisterIndex32 {
        RegisterEAX = 0,
        RegisterECX,
        RegisterEDX,
        RegisterEBX,
        RegisterESP,
        RegisterEBP,
        RegisterESI,
        RegisterEDI
    };

    void dumpSegment(WORD index);
    void dumpDescriptor(const Descriptor&, const char* prefix = "");
    void dumpDescriptor(const Gate&, const char* prefix = "");
    void dumpDescriptor(const SegmentDescriptor&, const char* prefix = "");
    void dumpDescriptor(const SystemDescriptor&, const char* prefix = "");
    void dumpDescriptor(const CodeSegmentDescriptor&, const char* prefix = "");
    void dumpDescriptor(const DataSegmentDescriptor&, const char* prefix = "");
    Descriptor getDescriptor(WORD selector, SegmentRegisterIndex = SegmentRegisterIndex::None);
    SegmentDescriptor getSegmentDescriptor(WORD selector, SegmentRegisterIndex = SegmentRegisterIndex::None);
    Gate getInterruptGate(WORD index);
    Descriptor getDescriptor(const char* tableName, DWORD tableBase, DWORD tableLimit, WORD index, bool indexIsSelector);

    SegmentRegisterIndex currentSegment() const { return m_segmentPrefix == SegmentRegisterIndex::None ? SegmentRegisterIndex::DS : m_segmentPrefix; }
    bool hasSegmentPrefix() const { return m_segmentPrefix != SegmentRegisterIndex::None; }

    void setSegmentPrefix(SegmentRegisterIndex segment)
    {
        m_segmentPrefix = segment;
    }

    void clearPrefix()
    {
        m_segmentPrefix = SegmentRegisterIndex::None;
        m_effectiveAddressSize32 = m_addressSize32;
        m_effectiveOperandSize32 = m_operandSize32;
    }

    // Extended memory size in KiB (will be reported by CMOS)
    DWORD extendedMemorySize() const { return m_extendedMemorySize; }
    void setExtendedMemorySize(DWORD size) { m_extendedMemorySize = size; }

    // Conventional memory size in KiB (will be reported by CMOS)
    DWORD baseMemorySize() const { return m_baseMemorySize; }
    void setBaseMemorySize(DWORD size) { m_baseMemorySize = size; }

    void kill();

    void setA20Enabled(bool value) { m_a20Enabled = value; }
    bool isA20Enabled() const { return m_a20Enabled; }

    DWORD a20Mask() const { return isA20Enabled() ? 0xFFFFFFFF : 0xFFEFFFFF; }

    void jumpToInterruptHandler(int isr, bool requestedByPIC = false);

    Exception GeneralProtectionFault(WORD selector, const QString& reason);
    Exception StackFault(WORD selector, const QString& reason);
    Exception NotPresent(WORD selector, const QString& reason);
    Exception InvalidTSS(WORD selector, const QString& reason);
    Exception PageFault(DWORD linearAddress, PageFaultFlags::Flags, MemoryAccessType, bool inUserMode, const char* faultTable, DWORD pde, DWORD pte = 0);
    Exception DivideError(const QString& reason);
    Exception InvalidOpcode(const QString& reason = QString());
    Exception BoundRangeExceeded(const QString& reason);

    void raiseException(const Exception&);

    void setIF(bool value) { this->IF = value; }
    void setCF(bool value) { this->CF = value; }
    void setDF(bool value) { this->DF = value; }
    void setSF(bool value) { m_dirtyFlags &= ~Flag::SF; this->SF = value; }
    void setAF(bool value) { this->AF = value; }
    void setTF(bool value) { this->TF = value; }
    void setOF(bool value) { this->OF = value; }
    void setPF(bool value) { m_dirtyFlags &= ~Flag::PF; this->PF = value; }
    void setZF(bool value) { m_dirtyFlags &= ~Flag::ZF; this->ZF = value; }
    void setVIF(bool value) { this->VIF = value; }
    void setNT(bool value) { this->NT = value; }
    void setRF(bool value) { this->RF = value; }
    void setVM(bool value) { this->VM = value; }
    void setIOPL(unsigned int value) { this->IOPL = value; }

    bool getIF() const { return this->IF; }
    bool getCF() const { return this->CF; }
    bool getDF() const { return this->DF; }
    bool getSF() const;
    bool getAF() const { return this->AF; }
    bool getTF() const { return this->TF; }
    bool getOF() const { return this->OF; }
    bool getPF() const;
    bool getZF() const;

    unsigned int getIOPL() const { return this->IOPL; }

    BYTE getCPL() const { return cachedDescriptor(SegmentRegisterIndex::CS).RPL(); }
    void setCPL(BYTE);

    bool getNT() const { return this->NT; }
    bool getVIP() const { return this->VIP; }
    bool getVIF() const { return this->VIF; }
    bool getVM() const { return this->VM; }
    bool getPE() const { return m_CR0 & CR0::PE; }
    bool getPG() const { return m_CR0 & CR0::PG; }
    bool getVME() const { return m_CR4 & 0x01; }
    bool getPVI() const { return m_CR4 & 0x02; }

    WORD getCS() const { return this->CS; }
    WORD getIP() const { return this->IP; }
    DWORD getEIP() const { return this->EIP; }

    WORD getDS() const { return this->DS; }
    WORD getES() const { return this->ES; }
    WORD getSS() const { return this->SS; }
    WORD getFS() const { return this->FS; }
    WORD getGS() const { return this->GS; }

    void setCS(WORD cs);
    void setDS(WORD ds);
    void setES(WORD es);
    void setSS(WORD ss);
    void setFS(WORD fs);
    void setGS(WORD gs);

    void setIP(WORD ip) { this->IP = ip; }
    void setEIP(DWORD eip) { this->EIP = eip; }

    WORD readSegmentRegister(SegmentRegisterIndex segreg) const { return *m_segmentMap[static_cast<int>(segreg)]; }

    DWORD getControlRegister(int registerIndex) const { return *m_controlRegisterMap[registerIndex]; }
    void setControlRegister(int registerIndex, DWORD value) { *m_controlRegisterMap[registerIndex] = value; }

    DWORD getDebugRegister(int registerIndex) const { return *m_debugRegisterMap[registerIndex]; }
    void setDebugRegister(int registerIndex, DWORD value) { *m_debugRegisterMap[registerIndex] = value; }

    DWORD getEAX() const { return this->regs.D.EAX; }
    DWORD getEBX() const { return this->regs.D.EBX; }
    DWORD getECX() const { return this->regs.D.ECX; }
    DWORD getEDX() const { return this->regs.D.EDX; }
    DWORD getESI() const { return this->regs.D.ESI; }
    DWORD getEDI() const { return this->regs.D.EDI; }
    DWORD getESP() const { return this->regs.D.ESP; }
    DWORD getEBP() const { return this->regs.D.EBP; }

    WORD getAX() const { return this->regs.W.AX; }
    WORD getBX() const { return this->regs.W.BX; }
    WORD getCX() const { return this->regs.W.CX; }
    WORD getDX() const { return this->regs.W.DX; }
    WORD getSI() const { return this->regs.W.SI; }
    WORD getDI() const { return this->regs.W.DI; }
    WORD getSP() const { return this->regs.W.SP; }
    WORD getBP() const { return this->regs.W.BP; }

    BYTE getAL() const { return this->regs.B.AL; }
    BYTE getBL() const { return this->regs.B.BL; }
    BYTE getCL() const { return this->regs.B.CL; }
    BYTE getDL() const { return this->regs.B.DL; }
    BYTE getAH() const { return this->regs.B.AH; }
    BYTE getBH() const { return this->regs.B.BH; }
    BYTE getCH() const { return this->regs.B.CH; }
    BYTE getDH() const { return this->regs.B.DH; }

    void setAL(BYTE value) { this->regs.B.AL = value; }
    void setBL(BYTE value) { this->regs.B.BL = value; }
    void setCL(BYTE value) { this->regs.B.CL = value; }
    void setDL(BYTE value) { this->regs.B.DL = value; }
    void setAH(BYTE value) { this->regs.B.AH = value; }
    void setBH(BYTE value) { this->regs.B.BH = value; }
    void setCH(BYTE value) { this->regs.B.CH = value; }
    void setDH(BYTE value) { this->regs.B.DH = value; }

    void setAX(WORD value) { this->regs.W.AX = value; }
    void setBX(WORD value) { this->regs.W.BX = value; }
    void setCX(WORD value) { this->regs.W.CX = value; }
    void setDX(WORD value) { this->regs.W.DX = value; }
    void setSP(WORD value) { this->regs.W.SP = value; }
    void setBP(WORD value) { this->regs.W.BP = value; }
    void setSI(WORD value) { this->regs.W.SI = value; }
    void setDI(WORD value) { this->regs.W.DI = value; }

    void setEAX(DWORD value) { this->regs.D.EAX = value; }
    void setEBX(DWORD value) { this->regs.D.EBX = value; }
    void setECX(DWORD value) { this->regs.D.ECX = value; }
    void setEDX(DWORD value) { this->regs.D.EDX = value; }
    void setESP(DWORD value) { this->regs.D.ESP = value; }
    void setEBP(DWORD value) { this->regs.D.EBP = value; }
    void setESI(DWORD value) { this->regs.D.ESI = value; }
    void setEDI(DWORD value) { this->regs.D.EDI = value; }

    DWORD getCR0() const { return m_CR0; }
    DWORD getCR1() const { return m_CR1; }
    DWORD getCR2() const { return m_CR2; }
    DWORD getCR3() const { return m_CR3; }
    DWORD getCR4() const { return m_CR4; }
    DWORD getCR5() const { return m_CR5; }
    DWORD getCR6() const { return m_CR6; }
    DWORD getCR7() const { return m_CR7; }

    DWORD getDR0() const { return m_DR0; }
    DWORD getDR1() const { return m_DR1; }
    DWORD getDR2() const { return m_DR2; }
    DWORD getDR3() const { return m_DR3; }
    DWORD getDR4() const { return m_DR4; }
    DWORD getDR5() const { return m_DR5; }
    DWORD getDR6() const { return m_DR6; }
    DWORD getDR7() const { return m_DR7; }

    // Base CS:EIP is the start address of the currently executing instruction
    WORD getBaseCS() const { return m_baseCS; }
    WORD getBaseIP() const { return m_baseEIP & 0xFFFF; }
    DWORD getBaseEIP() const { return m_baseEIP; }

    DWORD currentStackPointer() const
    {
        if (s32())
            return getESP();
        return getSP();
    }
    DWORD currentBasePointer() const
    {
        if (s32())
            return getEBP();
        return getBP();
    }
    void setCurrentStackPointer(DWORD value)
    {
        if (s32())
            setESP(value);
        else
            setSP(value);
    }
    void setCurrentBasePointer(DWORD value)
    {
        if (s32())
            setEBP(value);
        else
            setBP(value);
    }
    void adjustStackPointer(int delta)
    {
        setCurrentStackPointer(currentStackPointer() + delta);
    }

    void jump32(WORD segment, DWORD offset, JumpType, BYTE isr = 0, DWORD flags = 0, Gate* = nullptr);
    void jump16(WORD segment, WORD offset, JumpType, BYTE isr = 0, DWORD flags = 0, Gate* = nullptr);
    void jumpRelative8(SIGNED_BYTE displacement);
    void jumpRelative16(SIGNED_WORD displacement);
    void jumpRelative32(SIGNED_DWORD displacement);
    void jumpAbsolute16(WORD offset);
    void jumpAbsolute32(DWORD offset);

    void decodeNext();
    void execute(Instruction&);

    void executeOneInstruction();

    // CPU main loop - will fetch & decode until stopped
    void mainLoop();
    bool mainLoopSlowStuff();

    // CPU main loop when halted (HLT) - will do nothing until an IRQ is raised
    void haltedLoop();

    void push32(DWORD value);
    DWORD pop32();

    void push16(WORD value);
    WORD pop16();

    Debugger& debugger() { return *m_debugger; }

    template<typename T> T in(WORD port);
    template<typename T> void out(WORD port, T data);

    BYTE in8(WORD port);
    WORD in16(WORD port);
    DWORD in32(WORD port);
    void out8(WORD port, BYTE value);
    void out16(WORD port, WORD value);
    void out32(WORD port, DWORD value);

    BYTE* memoryPointer(DWORD address);
    BYTE* memoryPointer(WORD segment, DWORD offset);
    BYTE* memoryPointer(SegmentRegisterIndex, DWORD offset);
    BYTE* memoryPointer(const SegmentDescriptor&, DWORD offset);

    DWORD getEFlags() const;
    WORD getFlags() const;
    void setEFlags(DWORD flags);
    void setFlags(WORD flags);
    void setEFlagsRespectfully(DWORD flags);

    inline bool evaluate(BYTE) const;

    void updateFlags(DWORD value, BYTE bits);
    void updateFlags32(DWORD value);
    void updateFlags16(WORD value);
    void updateFlags8(BYTE value);
    void mathFlags8(WORD result, BYTE dest, BYTE src);
    void mathFlags16(DWORD result, WORD dest, WORD src);
    void mathFlags32(QWORD result, DWORD dest, DWORD src);
    void cmpFlags8(DWORD result, BYTE dest, BYTE src);
    void cmpFlags16(DWORD result, WORD dest, WORD src);
    void cmpFlags32(QWORD result, DWORD dest, DWORD src);

    void adjustFlag32(QWORD result, DWORD dest, DWORD src);

    template<typename T> void cmpFlags(QWORD result, T, T);

    template<typename T> T readRegister(int registerIndex);
    template<typename T> void writeRegister(int registerIndex, T value);

    DWORD readRegisterForAddressSize(int registerIndex);
    void writeRegisterForAddressSize(int registerIndex, DWORD);
    void stepRegisterForAddressSize(int registerIndex, DWORD stepSize);
    bool decrementCXForAddressSize();

    // These are faster than readMemory*() but will not access VGA memory, etc.
    inline BYTE readUnmappedMemory8(DWORD address) const;
    inline WORD readUnmappedMemory16(DWORD address) const;
    inline DWORD readUnmappedMemory32(DWORD address) const;
    inline void writeUnmappedMemory8(DWORD address, BYTE data);
    inline void writeUnmappedMemory16(DWORD address, WORD data);

    template<typename T> bool validatePhysicalAddress(DWORD, MemoryAccessType);
    template<typename T> void validateAddress(const SegmentDescriptor&, DWORD offset, MemoryAccessType);
    template<typename T> void validateAddress(SegmentRegisterIndex, DWORD offset, MemoryAccessType);
    template<typename T> T readMemory(DWORD address);
    template<typename T, MemoryAccessType accessType = MemoryAccessType::Read> T readMemory(const SegmentDescriptor&, DWORD address);
    template<typename T> T readMemory(SegmentRegisterIndex, DWORD address);
    template<typename T> void writeMemory(DWORD address, T data);
    template<typename T> void writeMemory(const SegmentDescriptor&, DWORD address, T data);
    template<typename T> void writeMemory(SegmentRegisterIndex, DWORD address, T data);

    void translateAddress(DWORD linearAddress, DWORD& physicalAddress, MemoryAccessType);
    void snoop(DWORD linearAddress, MemoryAccessType);
    void snoop(SegmentRegisterIndex, DWORD offset, MemoryAccessType);

    BYTE readMemory8(DWORD address);
    BYTE readMemory8(SegmentRegisterIndex, DWORD offset);
    WORD readMemory16(DWORD address);
    WORD readMemory16(SegmentRegisterIndex, DWORD offset);
    DWORD readMemory32(DWORD address);
    DWORD readMemory32(SegmentRegisterIndex, DWORD offset);
    void writeMemory8(DWORD address, BYTE data);
    void writeMemory8(SegmentRegisterIndex, DWORD offset, BYTE data);
    void writeMemory16(DWORD address, WORD data);
    void writeMemory16(SegmentRegisterIndex, DWORD offset, WORD data);
    void writeMemory32(DWORD address, DWORD data);
    void writeMemory32(SegmentRegisterIndex, DWORD offset, DWORD data);

    BYTE readModRM8(BYTE rmbyte);
    WORD readModRM16(BYTE rmbyte);
    DWORD readModRM32(BYTE rmbyte);
    void writeModRM8(BYTE rmbyte, BYTE value);
    void writeModRM16(BYTE rmbyte, WORD value);
    void writeModRM32(BYTE rmbyte, DWORD value);

    MemoryOrRegisterReference resolveModRM(BYTE rmbyte);

    enum State { Dead, Alive, Halted };
    State state() const { return m_state; }
    void setState(State s) { m_state = s; }

    SegmentDescriptor& cachedDescriptor(SegmentRegisterIndex index) { return m_descriptor[(int)index]; }
    const SegmentDescriptor& cachedDescriptor(SegmentRegisterIndex index) const { return m_descriptor[(int)index]; }

    // Dumps registers, flags & stack
    void dumpAll();

    void dumpWatches();

    void dumpIVT();
    void dumpIDT();
    void dumpLDT();
    void dumpGDT();

    void dumpMemory(WORD segment, DWORD offset, int rows);
    void dumpFlatMemory(DWORD address);
    void dumpRawMemory(BYTE*);
    unsigned dumpDisassembled(WORD segment, DWORD offset, unsigned count = 1);

    void dumpMemory(SegmentDescriptor&, DWORD offset, int rows);
    unsigned dumpDisassembled(SegmentDescriptor&, DWORD offset, unsigned count = 1);;
    unsigned dumpDisassembledInternal(SegmentDescriptor&, DWORD offset);

    void dumpTSS(const TSS&);

#ifdef CT_TRACE
    // Dumps registers (used by --trace)
    void dumpTrace();
#endif

    QVector<WatchedAddress>& watches() { return m_watches; }

    // Current execution mode (16 or 32 bit)
    bool x16() const { return !x32(); }
    bool x32() const { return m_descriptor[(int)SegmentRegisterIndex::CS].D(); }

    bool a16() const { return !m_effectiveAddressSize32; }
    bool a32() const { return m_effectiveAddressSize32; }
    bool o16() const { return !m_effectiveOperandSize32; }
    bool o32() const { return m_effectiveOperandSize32; }

    bool s16() const { return !m_stackSize32; }
    bool s32() const { return m_stackSize32; }

    enum Command { EnterMainLoop, ExitMainLoop, HardReboot };
    void queueCommand(Command);

    static const char* registerName(CPU::RegisterIndex8) PURE;
    static const char* registerName(CPU::RegisterIndex16) PURE;
    static const char* registerName(CPU::RegisterIndex32) PURE;
    static const char* registerName(SegmentRegisterIndex) PURE;

protected:
    void _CPUID(Instruction&);
    void _UNSUPP(Instruction&);
    void _ESCAPE(Instruction&);
    void _WAIT(Instruction&);
    void _NOP(Instruction&);
    void _HLT(Instruction&);
    void _INT_imm8(Instruction&);
    void _INT3(Instruction&);
    void _INTO(Instruction&);
    void _IRET(Instruction&);

    void _AAA(Instruction&);
    void _AAM(Instruction&);
    void _AAD(Instruction&);
    void _AAS(Instruction&);

    void _DAA(Instruction&);
    void _DAS(Instruction&);

    void _STC(Instruction&);
    void _STD(Instruction&);
    void _STI(Instruction&);
    void _CLC(Instruction&);
    void _CLD(Instruction&);
    void _CLI(Instruction&);
    void _CMC(Instruction&);
    void _CLTS(Instruction&);
    void _LAR_reg16_RM16(Instruction&);
    void _LAR_reg32_RM32(Instruction&);
    void _LSL_reg16_RM16(Instruction&);
    void _LSL_reg32_RM32(Instruction&);
    void _VERR_RM16(Instruction&);
    void _VERW_RM16(Instruction&);

    void _WBINVD(Instruction&);

    void _CBW(Instruction&);
    void _CWD(Instruction&);
    void _CWDE(Instruction&);
    void _CDQ(Instruction&);

    void _XLAT(Instruction&);
    void _SALC(Instruction&);

    void _JMP_imm32(Instruction&);
    void _JMP_imm16(Instruction&);
    void _JMP_imm16_imm16(Instruction&);
    void _JMP_short_imm8(Instruction&);
    void _JCXZ_imm8(Instruction&);

    void _Jcc_imm8(Instruction&);
    void _Jcc_NEAR_imm(Instruction&);
    void _SETcc_RM8(Instruction&);

    void _CALL_imm16(Instruction&);
    void _CALL_imm32(Instruction&);
    void _RET(Instruction&);
    void _RET_imm16(Instruction&);
    void _RETF(Instruction&);
    void _RETF_imm16(Instruction&);

    void doLOOP(Instruction&, bool condition);
    void _LOOP_imm8(Instruction&);
    void _LOOPZ_imm8(Instruction&);
    void _LOOPNZ_imm8(Instruction&);

    void _XCHG_AX_reg16(Instruction&);
    void _XCHG_EAX_reg32(Instruction&);
    void _XCHG_reg8_RM8(Instruction&);
    void _XCHG_reg16_RM16(Instruction&);
    void _XCHG_reg32_RM32(Instruction&);

    template<typename F> void doOnceOrRepeatedly(Instruction&, bool careAboutZF, F);
    template<typename T> void doLODS(Instruction&);
    template<typename T> void doSTOS(Instruction&);
    template<typename T> void doMOVS(Instruction&);
    template<typename T> void doINS(Instruction&);
    template<typename T> void doOUTS(Instruction&);
    template<typename T, typename U> void doCMPS(Instruction&);
    template<typename T, typename U> void doSCAS(Instruction&);

    void _CMPSB(Instruction&);
    void _CMPSW(Instruction&);
    void _CMPSD(Instruction&);
    void _LODSB(Instruction&);
    void _LODSW(Instruction&);
    void _LODSD(Instruction&);
    void _SCASB(Instruction&);
    void _SCASW(Instruction&);
    void _SCASD(Instruction&);
    void _STOSB(Instruction&);
    void _STOSW(Instruction&);
    void _STOSD(Instruction&);
    void _MOVSB(Instruction&);
    void _MOVSW(Instruction&);
    void _MOVSD(Instruction&);

    void _VKILL(Instruction&);

    void _LEA_reg16_mem16(Instruction&);
    void _LEA_reg32_mem32(Instruction&);

    void _LDS_reg16_mem16(Instruction&);
    void _LDS_reg32_mem32(Instruction&);
    void _LES_reg16_mem16(Instruction&);
    void _LES_reg32_mem32(Instruction&);

    void _MOV_reg8_imm8(Instruction&);
    void _MOV_reg16_imm16(Instruction&);
    void _MOV_reg32_imm32(Instruction&);

    void _MOV_seg_RM16(Instruction&);
    void _MOV_RM16_seg(Instruction&);
    void _MOV_RM32_seg(Instruction&);
    void _MOV_AL_moff8(Instruction&);
    void _MOV_AX_moff16(Instruction&);
    void _MOV_EAX_moff32(Instruction&);
    void _MOV_moff8_AL(Instruction&);
    void _MOV_moff16_AX(Instruction&);
    void _MOV_reg8_RM8(Instruction&);
    void _MOV_reg16_RM16(Instruction&);
    void _MOV_RM8_reg8(Instruction&);
    void _MOV_RM16_reg16(Instruction&);
    void _MOV_RM8_imm8(Instruction&);
    void _MOV_RM16_imm16(Instruction&);
    void _MOV_RM32_imm32(Instruction&);

    void _XOR_RM8_reg8(Instruction&);
    void _XOR_RM16_reg16(Instruction&);
    void _XOR_reg8_RM8(Instruction&);
    void _XOR_reg16_RM16(Instruction&);
    void _XOR_reg32_RM32(Instruction&);
    void _XOR_RM8_imm8(Instruction&);
    void _XOR_RM16_imm16(Instruction&);
    void _XOR_RM16_imm8(Instruction&);
    void _XOR_AL_imm8(Instruction&);
    void _XOR_AX_imm16(Instruction&);
    void _XOR_EAX_imm32(Instruction&);

    void _OR_RM8_reg8(Instruction&);
    void _OR_RM16_reg16(Instruction&);
    void _OR_RM32_reg32(Instruction&);
    void _OR_reg8_RM8(Instruction&);
    void _OR_reg16_RM16(Instruction&);
    void _OR_reg32_RM32(Instruction&);
    void _OR_RM8_imm8(Instruction&);
    void _OR_RM16_imm16(Instruction&);
    void _OR_RM16_imm8(Instruction&);
    void _OR_EAX_imm32(Instruction&);
    void _OR_AX_imm16(Instruction&);
    void _OR_AL_imm8(Instruction&);

    void _AND_RM8_reg8(Instruction&);
    void _AND_RM16_reg16(Instruction&);
    void _AND_reg8_RM8(Instruction&);
    void _AND_reg16_RM16(Instruction&);
    void _AND_RM8_imm8(Instruction&);
    void _AND_RM16_imm16(Instruction&);
    void _AND_RM16_imm8(Instruction&);
    void _AND_AL_imm8(Instruction&);
    void _AND_AX_imm16(Instruction&);
    void _AND_EAX_imm32(Instruction&);

    void _TEST_RM8_reg8(Instruction&);
    void _TEST_RM16_reg16(Instruction&);
    void _TEST_RM32_reg32(Instruction&);
    void _TEST_AL_imm8(Instruction&);
    void _TEST_AX_imm16(Instruction&);
    void _TEST_EAX_imm32(Instruction&);

    void _PUSH_SP_8086_80186(Instruction&);
    void _PUSH_CS(Instruction&);
    void _PUSH_DS(Instruction&);
    void _PUSH_ES(Instruction&);
    void _PUSH_SS(Instruction&);
    void _PUSHF(Instruction&);

    void _POP_DS(Instruction&);
    void _POP_ES(Instruction&);
    void _POP_SS(Instruction&);
    void _POPF(Instruction&);

    void _LAHF(Instruction&);
    void _SAHF(Instruction&);

    void _OUT_imm8_AL(Instruction&);
    void _OUT_imm8_AX(Instruction&);
    void _OUT_imm8_EAX(Instruction&);
    void _OUT_DX_AL(Instruction&);
    void _OUT_DX_AX(Instruction&);
    void _OUT_DX_EAX(Instruction&);
    void _OUTSB(Instruction&);
    void _OUTSW(Instruction&);
    void _OUTSD(Instruction&);

    void _IN_AL_imm8(Instruction&);
    void _IN_AX_imm8(Instruction&);
    void _IN_EAX_imm8(Instruction&);
    void _IN_AL_DX(Instruction&);
    void _IN_AX_DX(Instruction&);
    void _IN_EAX_DX(Instruction&);
    void _INSB(Instruction&);
    void _INSW(Instruction&);
    void _INSD(Instruction&);

    void _ADD_RM8_reg8(Instruction&);
    void _ADD_RM16_reg16(Instruction&);
    void _ADD_reg8_RM8(Instruction&);
    void _ADD_reg16_RM16(Instruction&);
    void _ADD_AL_imm8(Instruction&);
    void _ADD_AX_imm16(Instruction&);
    void _ADD_EAX_imm32(Instruction&);
    void _ADD_RM8_imm8(Instruction&);
    void _ADD_RM16_imm16(Instruction&);
    void _ADD_RM16_imm8(Instruction&);

    void _SUB_RM8_reg8(Instruction&);
    void _SUB_RM16_reg16(Instruction&);
    void _SUB_reg8_RM8(Instruction&);
    void _SUB_reg16_RM16(Instruction&);
    void _SUB_AL_imm8(Instruction&);
    void _SUB_AX_imm16(Instruction&);
    void _SUB_EAX_imm32(Instruction&);
    void _SUB_RM8_imm8(Instruction&);
    void _SUB_RM16_imm16(Instruction&);
    void _SUB_RM16_imm8(Instruction&);

    void _ADC_RM8_reg8(Instruction&);
    void _ADC_RM16_reg16(Instruction&);
    void _ADC_reg8_RM8(Instruction&);
    void _ADC_reg16_RM16(Instruction&);
    void _ADC_AL_imm8(Instruction&);
    void _ADC_AX_imm16(Instruction&);
    void _ADC_EAX_imm32(Instruction&);
    void _ADC_RM8_imm8(Instruction&);
    void _ADC_RM16_imm16(Instruction&);
    void _ADC_RM16_imm8(Instruction&);

    void _SBB_RM8_reg8(Instruction&);
    void _SBB_RM16_reg16(Instruction&);
    void _SBB_RM32_reg32(Instruction&);
    void _SBB_reg8_RM8(Instruction&);
    void _SBB_reg16_RM16(Instruction&);
    void _SBB_AL_imm8(Instruction&);
    void _SBB_AX_imm16(Instruction&);
    void _SBB_EAX_imm32(Instruction&);
    void _SBB_RM8_imm8(Instruction&);
    void _SBB_RM16_imm16(Instruction&);
    void _SBB_RM16_imm8(Instruction&);

    void _CMP_RM8_reg8(Instruction&);
    void _CMP_RM16_reg16(Instruction&);
    void _CMP_RM32_reg32(Instruction&);
    void _CMP_reg8_RM8(Instruction&);
    void _CMP_reg16_RM16(Instruction&);
    void _CMP_reg32_RM32(Instruction&);
    void _CMP_AL_imm8(Instruction&);
    void _CMP_AX_imm16(Instruction&);
    void _CMP_EAX_imm32(Instruction&);
    void _CMP_RM8_imm8(Instruction&);
    void _CMP_RM16_imm16(Instruction&);
    void _CMP_RM16_imm8(Instruction&);

    void _MUL_RM8(Instruction&);
    void _MUL_RM16(Instruction&);
    void _MUL_RM32(Instruction&);
    void _DIV_RM8(Instruction&);
    void _DIV_RM16(Instruction&);
    void _DIV_RM32(Instruction&);
    void _IMUL_RM8(Instruction&);
    void _IMUL_RM16(Instruction&);
    void _IMUL_RM32(Instruction&);
    void _IDIV_RM8(Instruction&);
    void _IDIV_RM16(Instruction&);
    void _IDIV_RM32(Instruction&);

    void _TEST_RM8_imm8(Instruction&);
    void _TEST_RM16_imm16(Instruction&);

    template<typename T> void doNEG(Instruction&);
    template<typename T> void doNOT(Instruction&);
    void _NOT_RM8(Instruction&);
    void _NOT_RM16(Instruction&);
    void _NOT_RM32(Instruction&);
    void _NEG_RM8(Instruction&);
    void _NEG_RM16(Instruction&);
    void _NEG_RM32(Instruction&);

    void _INC_RM8(Instruction&);
    void _INC_RM16(Instruction&);
    void _INC_RM32(Instruction&);
    void _INC_reg16(Instruction&);
    void _INC_reg32(Instruction&);
    void _DEC_RM8(Instruction&);
    void _DEC_RM16(Instruction&);
    void _DEC_RM32(Instruction&);
    void _DEC_reg16(Instruction&);
    void _DEC_reg32(Instruction&);

    template<typename T> void doFarJump(Instruction&, JumpType);

    void _CALL_RM16(Instruction&);
    void _CALL_RM32(Instruction&);
    void _CALL_FAR_mem16(Instruction&);
    void _CALL_FAR_mem32(Instruction&);
    void _CALL_imm16_imm16(Instruction&);
    void _CALL_imm16_imm32(Instruction&);

    void _JMP_RM16(Instruction&);
    void _JMP_RM32(Instruction&);
    void _JMP_FAR_mem16(Instruction&);
    void _JMP_FAR_mem32(Instruction&);

    void _PUSH_RM16(Instruction&);
    void _PUSH_RM32(Instruction&);
    void _POP_RM16(Instruction&);
    void _POP_RM32(Instruction&);

    void _wrap_0xC0(Instruction&);
    void _wrap_0xC1_16(Instruction&);
    void _wrap_0xC1_32(Instruction&);
    void _wrap_0xD0(Instruction&);
    void _wrap_0xD1_16(Instruction&);
    void _wrap_0xD1_32(Instruction&);
    void _wrap_0xD2(Instruction&);
    void _wrap_0xD3_16(Instruction&);
    void _wrap_0xD3_32(Instruction&);

    void _BOUND(Instruction&);
    void _ENTER16(Instruction&);
    void _ENTER32(Instruction&);
    void _LEAVE16(Instruction&);
    void _LEAVE32(Instruction&);

    void _PUSHA(Instruction&);
    void _POPA(Instruction&);
    void _PUSH_imm8(Instruction&);
    void _PUSH_imm16(Instruction&);

    void _IMUL_reg16_RM16(Instruction&);
    void _IMUL_reg32_RM32(Instruction&);
    void _IMUL_reg16_RM16_imm8(Instruction&);
    void _IMUL_reg32_RM32_imm8(Instruction&);
    void _IMUL_reg16_RM16_imm16(Instruction&);
    void _IMUL_reg32_RM32_imm32(Instruction&);

    void _LMSW_RM16(Instruction&);
    void _SMSW_RM16(Instruction&);

    void _SGDT(Instruction&);
    void _LGDT(Instruction&);
    void _SIDT(Instruction&);
    void _LIDT(Instruction&);
    void _LLDT_RM16(Instruction&);
    void _SLDT_RM16(Instruction&);
    void _LTR_RM16(Instruction&);
    void _STR_RM16(Instruction&);

    void _PUSHAD(Instruction&);
    void _POPAD(Instruction&);
    void _PUSHFD(Instruction&);
    void _POPFD(Instruction&);
    void _PUSH_imm32(Instruction&);

    void _PUSH_reg16(Instruction&);
    void _PUSH_reg32(Instruction&);
    void _POP_reg16(Instruction&);
    void _POP_reg32(Instruction&);

    void _TEST_RM32_imm32(Instruction&);
    void _XOR_RM32_reg32(Instruction&);
    void _ADD_RM32_reg32(Instruction&);
    void _ADC_RM32_reg32(Instruction&);
    void _SUB_RM32_reg32(Instruction&);

    void _BT_RM16_imm8(Instruction&);
    void _BT_RM32_imm8(Instruction&);
    void _BT_RM16_reg16(Instruction&);
    void _BT_RM32_reg32(Instruction&);
    void _BTR_RM16_imm8(Instruction&);
    void _BTR_RM32_imm8(Instruction&);
    void _BTR_RM16_reg16(Instruction&);
    void _BTR_RM32_reg32(Instruction&);
    void _BTC_RM16_imm8(Instruction&);
    void _BTC_RM32_imm8(Instruction&);
    void _BTC_RM16_reg16(Instruction&);
    void _BTC_RM32_reg32(Instruction&);
    void _BTS_RM16_imm8(Instruction&);
    void _BTS_RM32_imm8(Instruction&);
    void _BTS_RM16_reg16(Instruction&);
    void _BTS_RM32_reg32(Instruction&);

    void _BSF_reg16_RM16(Instruction&);
    void _BSF_reg32_RM32(Instruction&);
    void _BSR_reg16_RM16(Instruction&);
    void _BSR_reg32_RM32(Instruction&);

    void _ROL_RM8_imm8(Instruction&);
    void _ROL_RM16_imm8(Instruction&);
    void _ROL_RM32_imm8(Instruction&);
    void _ROL_RM8_1(Instruction&);
    void _ROL_RM16_1(Instruction&);
    void _ROL_RM32_1(Instruction&);
    void _ROL_RM8_CL(Instruction&);
    void _ROL_RM16_CL(Instruction&);
    void _ROL_RM32_CL(Instruction&);

    void _ROR_RM8_imm8(Instruction&);
    void _ROR_RM16_imm8(Instruction&);
    void _ROR_RM32_imm8(Instruction&);
    void _ROR_RM8_1(Instruction&);
    void _ROR_RM16_1(Instruction&);
    void _ROR_RM32_1(Instruction&);
    void _ROR_RM8_CL(Instruction&);
    void _ROR_RM16_CL(Instruction&);
    void _ROR_RM32_CL(Instruction&);

    void _SHL_RM8_imm8(Instruction&);
    void _SHL_RM16_imm8(Instruction&);
    void _SHL_RM32_imm8(Instruction&);
    void _SHL_RM8_1(Instruction&);
    void _SHL_RM16_1(Instruction&);
    void _SHL_RM32_1(Instruction&);
    void _SHL_RM8_CL(Instruction&);
    void _SHL_RM16_CL(Instruction&);
    void _SHL_RM32_CL(Instruction&);

    void _SHR_RM8_imm8(Instruction&);
    void _SHR_RM16_imm8(Instruction&);
    void _SHR_RM32_imm8(Instruction&);
    void _SHR_RM8_1(Instruction&);
    void _SHR_RM16_1(Instruction&);
    void _SHR_RM32_1(Instruction&);
    void _SHR_RM8_CL(Instruction&);
    void _SHR_RM16_CL(Instruction&);
    void _SHR_RM32_CL(Instruction&);

    void _SAR_RM8_imm8(Instruction&);
    void _SAR_RM16_imm8(Instruction&);
    void _SAR_RM32_imm8(Instruction&);
    void _SAR_RM8_1(Instruction&);
    void _SAR_RM16_1(Instruction&);
    void _SAR_RM32_1(Instruction&);
    void _SAR_RM8_CL(Instruction&);
    void _SAR_RM16_CL(Instruction&);
    void _SAR_RM32_CL(Instruction&);

    void _RCL_RM8_imm8(Instruction&);
    void _RCL_RM16_imm8(Instruction&);
    void _RCL_RM32_imm8(Instruction&);
    void _RCL_RM8_1(Instruction&);
    void _RCL_RM16_1(Instruction&);
    void _RCL_RM32_1(Instruction&);
    void _RCL_RM8_CL(Instruction&);
    void _RCL_RM16_CL(Instruction&);
    void _RCL_RM32_CL(Instruction&);

    void _RCR_RM8_imm8(Instruction&);
    void _RCR_RM16_imm8(Instruction&);
    void _RCR_RM32_imm8(Instruction&);
    void _RCR_RM8_1(Instruction&);
    void _RCR_RM16_1(Instruction&);
    void _RCR_RM32_1(Instruction&);
    void _RCR_RM8_CL(Instruction&);
    void _RCR_RM16_CL(Instruction&);
    void _RCR_RM32_CL(Instruction&);

    void _SHLD_RM16_reg16_imm8(Instruction&);
    void _SHLD_RM32_reg32_imm8(Instruction&);
    void _SHLD_RM16_reg16_CL(Instruction&);
    void _SHLD_RM32_reg32_CL(Instruction&);
    void _SHRD_RM16_reg16_imm8(Instruction&);
    void _SHRD_RM32_reg32_imm8(Instruction&);
    void _SHRD_RM16_reg16_CL(Instruction&);
    void _SHRD_RM32_reg32_CL(Instruction&);

    void _MOVZX_reg16_RM8(Instruction&);
    void _MOVZX_reg32_RM8(Instruction&);
    void _MOVZX_reg32_RM16(Instruction&);

    void _MOVSX_reg16_RM8(Instruction&);
    void _MOVSX_reg32_RM8(Instruction&);
    void _MOVSX_reg32_RM16(Instruction&);

    template<typename T> void doLxS(Instruction&, SegmentRegisterIndex);
    void _LFS_reg16_mem16(Instruction&);
    void _LFS_reg32_mem32(Instruction&);
    void _LGS_reg16_mem16(Instruction&);
    void _LGS_reg32_mem32(Instruction&);
    void _LSS_reg16_mem16(Instruction&);
    void _LSS_reg32_mem32(Instruction&);

    void _PUSH_FS(Instruction&);
    void _PUSH_GS(Instruction&);
    void _POP_FS(Instruction&);
    void _POP_GS(Instruction&);

    void _MOV_RM32_reg32(Instruction&);
    void _MOV_reg32_RM32(Instruction&);
    void _MOV_reg32_CR(Instruction&);
    void _MOV_CR_reg32(Instruction&);
    void _MOV_reg32_DR(Instruction&);
    void _MOV_DR_reg32(Instruction&);
    void _MOV_moff32_EAX(Instruction&);

    void _MOV_seg_RM32(Instruction&);

    void _JMP_imm16_imm32(Instruction&);

    void _ADD_RM32_imm32(Instruction&);
    void _OR_RM32_imm32(Instruction&);
    void _ADC_RM32_imm32(Instruction&);
    void _SBB_RM32_imm32(Instruction&);
    void _AND_RM32_imm32(Instruction&);
    void _SUB_RM32_imm32(Instruction&);
    void _XOR_RM32_imm32(Instruction&);
    void _CMP_RM32_imm32(Instruction&);

    void _ADD_RM32_imm8(Instruction&);
    void _OR_RM32_imm8(Instruction&);
    void _ADC_RM32_imm8(Instruction&);
    void _SBB_RM32_imm8(Instruction&);
    void _AND_RM32_imm8(Instruction&);
    void _SUB_RM32_imm8(Instruction&);
    void _XOR_RM32_imm8(Instruction&);
    void _CMP_RM32_imm8(Instruction&);

    void _ADD_reg32_RM32(Instruction&);
    void _ADC_reg32_RM32(Instruction&);
    void _SBB_reg32_RM32(Instruction&);
    void _AND_reg32_RM32(Instruction&);
    void _SUB_reg32_RM32(Instruction&);
    void _AND_RM32_reg32(Instruction&);

    void _RDTSC(Instruction&);

    void _UD0(Instruction&);
    void _LOCK(Instruction&);

    void handleRepeatOpcode(Instruction&, bool conditionForZF);

private:
    friend class Instruction;

    BYTE readInstruction8() override;
    WORD readInstruction16() override;
    DWORD readInstruction32() override;

    void initWatches();
    void hardReboot();

    void updateDefaultSizes();
    void updateStackSize();
    void updateCodeSegmentCache();
    void makeNextInstructionUninterruptible();

    void didTouchMemory(DWORD address);

    void translateAddressSlowCase(DWORD linearAddress, DWORD& physicalAddress, MemoryAccessType);

    template<typename T> T doSAR(T, int steps);
    template<typename T> T doRCL(T, int steps);
    template<typename T> T doRCR(T, int steps);

    template<typename T> T doSHL(T, int steps);
    template<typename T> T doSHR(T, int steps);

    template<typename T> T doSHLD(T, T, int steps);
    template<typename T> T doSHRD(T, T, int steps);

    template<typename T> T doROL(T, int steps);
    template<typename T> T doROR(T, int steps);

    template<typename T> T doXor(T, T);
    template<typename T> T doOr(T, T);
    template<typename T> T doAnd(T, T);

    template<typename T> T doBt(T, int bitIndex);
    template<typename T> T doBtr(T, int bitIndex);
    template<typename T> T doBtc(T, int bitIndex);
    template<typename T> T doBts(T, int bitIndex);
    template<typename T> T doBSF(T);
    template<typename T> T doBSR(T);

    template<typename T> QWORD doAdd(T, T);
    template<typename T> QWORD doAdc(T, T);
    template<typename T> QWORD doSub(T, T);
    template<typename T> QWORD doSbb(T, T);
    template<typename T> QWORD doMul(T, T);
    template<typename T> SIGNED_QWORD doImul(T, T);

    void saveBaseAddress()
    {
        m_baseCS = getCS();
        m_baseEIP = getEIP();
    }

    void setLDT(WORD segment);
    void taskSwitch(WORD task, JumpType);
    void taskSwitch(TSSDescriptor&, JumpType);
    TSS currentTSS();

    void writeToGDT(Descriptor&);

    void dumpSelector(const char* prefix, SegmentRegisterIndex);
    void writeSegmentRegister(SegmentRegisterIndex, WORD selector);
    void validateSegmentLoad(SegmentRegisterIndex, WORD selector, const Descriptor&);

    SegmentDescriptor m_descriptor[6];

    union {
        struct {
            DWORD EAX, EBX, ECX, EDX;
            DWORD EBP, ESP, ESI, EDI;
            DWORD EIP;
        } D;
#ifdef CT_BIG_ENDIAN
        struct {
            WORD __EAX_high_word, AX;
            WORD __EBX_high_word, BX;
            WORD __ECX_high_word, CX;
            WORD __EDX_high_word, DX;
            WORD __EBP_high_word, BP;
            WORD __ESP_high_word, SP;
            WORD __ESI_high_word, SI;
            WORD __EDI_high_word, DI;
            WORD __EIP_high_word, IP;
        } W;
        struct {
            WORD __EAX_high_word;
            BYTE AH, AL;
            WORD __EBX_high_word;
            BYTE BH, BL;
            WORD __ECX_high_word;
            BYTE CH, CL;
            WORD __EDX_high_word;
            BYTE DH, DL;
            DWORD EBP;
            DWORD ESP;
            DWORD ESI;
            DWORD EDI;
            DWORD EIP;
        } B;
#else
        struct {
            WORD AX, __EAX_high_word;
            WORD BX, __EBX_high_word;
            WORD CX, __ECX_high_word;
            WORD DX, __EDX_high_word;
            WORD BP, __EBP_high_word;
            WORD SP, __ESP_high_word;
            WORD SI, __ESI_high_word;
            WORD DI, __EDI_high_word;
            WORD IP, __EIP_high_word;
        } W;
        struct {
            BYTE AL, AH;
            WORD __EAX_high_word;
            BYTE BL, BH;
            WORD __EBX_high_word;
            BYTE CL, CH;
            WORD __ECX_high_word;
            BYTE DL, DH;
            WORD __EDX_high_word;
            DWORD EBP;
            DWORD ESP;
            DWORD ESI;
            DWORD EDI;
            DWORD EIP;
        } B;
#endif
    } regs;

    WORD CS, DS, ES, SS, FS, GS;
    mutable bool CF, PF, AF, ZF, SF, OF;
    bool DF, IF, TF;

    unsigned int IOPL;
    bool NT;
    bool RF;
    bool VM;
    bool AC;
    bool VIF;
    bool VIP;
    bool ID;

    struct {
        DWORD base;
        DWORD limit;
    } GDTR;

    struct {
        DWORD base;
        DWORD limit;
    } IDTR;

    struct {
        WORD segment { 0 };
        DWORD base { 0 };
        DWORD limit { 0 };
    } LDTR;

    DWORD m_CR0, m_CR1, m_CR2, m_CR3, m_CR4, m_CR5, m_CR6, m_CR7;
    DWORD m_DR0, m_DR1, m_DR2, m_DR3, m_DR4, m_DR5, m_DR6, m_DR7;

    union {
        struct {
#ifdef CT_BIG_ENDIAN
            WORD __EIP_high_word, IP;
#else
            WORD IP, __EIP_high_word;
#endif
        };
        DWORD EIP;
    };

    struct {
        WORD segment { 0 };
        DWORD base { 0 };
        DWORD limit { 0 };
        bool is32Bit { false };
    } TR;

    State m_state { Dead };

    // Actual CS:EIP (when we started fetching the instruction)
    WORD m_baseCS { 0 };
    DWORD m_baseEIP { 0 };

    SegmentRegisterIndex m_segmentPrefix { SegmentRegisterIndex::None };

    DWORD m_baseMemorySize { 0 };
    DWORD m_extendedMemorySize { 0 };

    std::set<DWORD> m_breakpoints;

    bool m_a20Enabled { false };
    bool m_nextInstructionIsUninterruptible { false };

    OwnPtr<Debugger> m_debugger;

    BYTE* m_memory { nullptr };
    size_t m_memorySize { 8192 * 1024 };

    WORD* m_segmentMap[8];
    DWORD* m_controlRegisterMap[8];
    DWORD* m_debugRegisterMap[8];

    // ID-to-Register maps
    DWORD* treg32[8];
    WORD* treg16[8];
    BYTE* treg8[8];

    Machine& m_machine;

    bool m_addressSize32 { false };
    bool m_operandSize32 { false };
    bool m_effectiveAddressSize32 { false };
    bool m_effectiveOperandSize32 { false };
    bool m_stackSize32 { false };

    std::atomic<bool> m_mainLoopNeedsSlowStuff { false };
    std::atomic<bool> m_shouldBreakOutOfMainLoop { false };
    std::atomic<bool> m_shouldHardReboot { false };

    QVector<WatchedAddress> m_watches;

    bool m_isForAutotest { false };

    QWORD m_cycle { 0 };

    mutable DWORD m_dirtyFlags { 0 };
    QWORD m_lastResult { 0 };
    unsigned m_lastOpSize { ByteSize };
};

extern CPU* g_cpu;

// INLINE IMPLEMENTATIONS

BYTE CPU::readUnmappedMemory8(DWORD address) const
{
    return m_memory[address];
}

WORD CPU::readUnmappedMemory16(DWORD address) const
{
    return read16FromPointer(reinterpret_cast<WORD*>(m_memory + address));
}

DWORD CPU::readUnmappedMemory32(DWORD address) const
{
    return read32FromPointer(reinterpret_cast<DWORD*>(m_memory + address));
}

void CPU::writeUnmappedMemory8(DWORD address, BYTE value)
{
    m_memory[address] = value;
}

void CPU::writeUnmappedMemory16(DWORD address, WORD value)
{
    write16ToPointer(reinterpret_cast<WORD*>(m_memory + address), value);
}

template<typename T>
inline void CPU::cmpFlags(QWORD result, T dest, T src)
{
    if (BitSizeOfType<T>::bits == 8)
        cmpFlags8(result, dest, src);
    else if (BitSizeOfType<T>::bits == 16)
        cmpFlags16(result, dest, src);
    else if (BitSizeOfType<T>::bits == 32)
        cmpFlags32(result, dest, src);
}

#include "debug.h"

ALWAYS_INLINE bool CPU::evaluate(BYTE conditionCode) const
{
    ASSERT(conditionCode <= 0xF);

    switch (conditionCode) {
    case  0: return this->OF;                            // O
    case  1: return !this->OF;                           // NO
    case  2: return this->CF;                            // B, C, NAE
    case  3: return !this->CF;                           // NB, NC, AE
    case  4: return getZF();                             // E, Z
    case  5: return !getZF();                            // NE, NZ
    case  6: return (this->CF | getZF());                // BE, NA
    case  7: return !(this->CF | getZF());               // NBE, A
    case  8: return getSF();                             // S
    case  9: return !getSF();                            // NS
    case 10: return getPF();                             // P, PE
    case 11: return !getPF();                            // NP, PO
    case 12: return getSF() ^ this->OF;                  // L, NGE
    case 13: return !(getSF() ^ this->OF);               // NL, GE
    case 14: return (getSF() ^ this->OF) | getZF();      // LE, NG
    case 15: return !((getSF() ^ this->OF) | getZF());   // NLE, G
    }
    return 0;
}

ALWAYS_INLINE BYTE& Instruction::reg8()
{
#ifdef DEBUG_INSTRUCTION
    ASSERT(m_cpu);
#endif
    return *m_cpu->treg8[registerIndex()];
}

ALWAYS_INLINE WORD& Instruction::reg16()
{
#ifdef DEBUG_INSTRUCTION
    ASSERT(m_cpu);
    ASSERT(m_cpu->o16());
#endif
    return *m_cpu->treg16[registerIndex()];
}

ALWAYS_INLINE WORD& Instruction::segreg()
{
#ifdef DEBUG_INSTRUCTION
    ASSERT(m_cpu);
    ASSERT(registerIndex() < 6);
#endif
    return *m_cpu->m_segmentMap[registerIndex()];
}

ALWAYS_INLINE DWORD& Instruction::reg32()
{
#ifdef DEBUG_INSTRUCTION
    ASSERT(m_cpu);
    ASSERT(m_cpu->o32());
#endif
    return *m_cpu->treg32[registerIndex()];
}

template<> ALWAYS_INLINE BYTE& Instruction::reg<BYTE>() { return reg8(); }
template<> ALWAYS_INLINE WORD& Instruction::reg<WORD>() { return reg16(); }
template<> ALWAYS_INLINE DWORD& Instruction::reg<DWORD>() { return reg32(); }

ALWAYS_INLINE void Instruction::execute(CPU& cpu)
{
    m_cpu = &cpu;
    cpu.setSegmentPrefix(m_segmentPrefix);
    cpu.m_effectiveOperandSize32 = m_o32;
    cpu.m_effectiveAddressSize32 = m_a32;
    if (m_hasRM)
        m_modrm.resolve(cpu);
    (cpu.*m_impl)(*this);
}
