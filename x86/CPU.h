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

#ifndef VCPU_H
#define VCPU_H

#include "Common.h"
#include "debug.h"
#include <QtCore/QMutex>
#include <QtCore/QQueue>
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

#define CALL_HANDLER(handler16, handler32) if (o16()) { handler16(insn); } else { handler32(insn); }

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

enum class JumpType { Internal, GateEntry, IRET, RETF, INT, CALL, JMP };

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

    QWORD cycle() const { return m_cycle; }

    void reset();

    Machine& machine() const { return m_machine; }

    std::set<DWORD>& breakpoints() { return m_breakpoints; }

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
    void dumpDescriptor(const Descriptor&);
    void dumpDescriptor(const Gate&);
    void dumpDescriptor(const SegmentDescriptor&);
    void dumpDescriptor(const SystemDescriptor&);
    void dumpDescriptor(const CodeSegmentDescriptor&);
    void dumpDescriptor(const DataSegmentDescriptor&);
    Descriptor getDescriptor(WORD selector);
    SegmentDescriptor getSegmentDescriptor(WORD selector);
    Gate getInterruptGate(WORD index);
    Descriptor getDescriptor(const char* tableName, DWORD tableBase, DWORD tableLimit, WORD index, bool indexIsSelector);

    SegmentRegisterIndex currentSegment() const { return m_segmentPrefix == SegmentRegisterIndex::None ? SegmentRegisterIndex::DS : m_segmentPrefix; }
    bool hasSegmentPrefix() const { return m_segmentPrefix != SegmentRegisterIndex::None; }

    void setSegmentPrefix(SegmentRegisterIndex segment)
    {
        m_segmentPrefix = segment;
    }

    void resetSegmentPrefix() { m_segmentPrefix = SegmentRegisterIndex::None; }

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
    Exception PageFault(DWORD address, WORD error, const QString& reason);
    Exception DivideError(const QString& reason);
    Exception InvalidOpcode(const QString& reason = QString());

    void raiseException(const Exception&);

    void setIF(bool value) { this->IF = value; }
    void setCF(bool value) { this->CF = value; }
    void setDF(bool value) { this->DF = value; }
    void setSF(bool value) { this->SF = value; }
    void setAF(bool value) { this->AF = value; }
    void setTF(bool value) { this->TF = value; }
    void setOF(bool value) { this->OF = value; }
    void setPF(bool value) { this->PF = value; }
    void setZF(bool value) { this->ZF = value; }
    void setVIF(bool value) { this->VIF = value; }
    void setNT(bool value) { this->NT = value; }
    void setRF(bool value) { this->RF = value; }
    void setIOPL(unsigned int value) { this->IOPL = value; }

    bool getIF() const { return this->IF; }
    bool getCF() const { return this->CF; }
    bool getDF() const { return this->DF; }
    bool getSF() const { return this->SF; }
    bool getAF() const { return this->AF; }
    bool getTF() const { return this->TF; }
    bool getOF() const { return this->OF; }
    bool getPF() const { return this->PF; }
    bool getZF() const { return this->ZF; }

    unsigned int getIOPL() const { return this->IOPL; }

    BYTE getCPL() const { return cachedDescriptor(SegmentRegisterIndex::CS).RPL(); }
    void setCPL(BYTE);

    bool getNT() const { return this->NT; }
    bool getVIP() const { return this->VIP; }
    bool getVIF() const { return this->VIF; }
    bool getVM() const { return this->VM; }
    bool getPE() const { return this->CR0 & 0x01; }
    bool getPG() const { return this->CR0 & 0x80000000; }
    bool getVME() const { return this->CR4 & 0x01; }
    bool getPVI() const { return this->CR4 & 0x02; }

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

    DWORD getRegister32(RegisterIndex32 registerIndex) const { return *treg32[registerIndex]; }
    WORD getRegister16(RegisterIndex16 registerIndex) const { return *treg16[registerIndex]; }
    BYTE getRegister8(RegisterIndex8 registerIndex) const { return *treg8[registerIndex]; }

    void setRegister32(RegisterIndex32 registerIndex, DWORD value) { *treg32[registerIndex] = value; }
    void setRegister16(RegisterIndex16 registerIndex, WORD value) { *treg16[registerIndex] = value; }
    void setRegister8(RegisterIndex8 registerIndex, BYTE value) { *treg8[registerIndex] = value; }

    WORD getSegment(SegmentRegisterIndex segmentIndex) const { return *m_segmentMap[static_cast<int>(segmentIndex)]; }
    void setSegment(SegmentRegisterIndex segmentIndex, WORD value) const { *m_segmentMap[static_cast<int>(segmentIndex)] = value; }

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

    DWORD getCR0() const { return this->CR0; }
    DWORD getCR1() const { return this->CR1; }
    DWORD getCR2() const { return this->CR2; }
    DWORD getCR3() const { return this->CR3; }
    DWORD getCR4() const { return this->CR4; }
    DWORD getCR5() const { return this->CR5; }
    DWORD getCR6() const { return this->CR6; }
    DWORD getCR7() const { return this->CR7; }

    DWORD getDR0() const { return this->DR0; }
    DWORD getDR1() const { return this->DR1; }
    DWORD getDR2() const { return this->DR2; }
    DWORD getDR3() const { return this->DR3; }
    DWORD getDR4() const { return this->DR4; }
    DWORD getDR5() const { return this->DR5; }
    DWORD getDR6() const { return this->DR6; }
    DWORD getDR7() const { return this->DR7; }

    // Base CS:EIP is the start address of the currently executing instruction
    WORD getBaseCS() const { return m_baseCS; }
    WORD getBaseIP() const { return m_baseEIP & 0xFFFF; }
    DWORD getBaseEIP() const { return m_baseEIP; }

    void jump32(WORD segment, DWORD offset, JumpType, BYTE isr = 0, DWORD flags = 0);
    void jump16(WORD segment, WORD offset, JumpType, BYTE isr = 0, DWORD flags = 0);
    void jumpRelative8(SIGNED_BYTE displacement);
    void jumpRelative16(SIGNED_WORD displacement);
    void jumpRelative32(SIGNED_DWORD displacement);
    void jumpAbsolute16(WORD offset);
    void jumpAbsolute32(DWORD offset);

    FarPointer getInterruptVector16(int isr);
    FarPointer getInterruptVector32(int isr);

    void decodeNext();
    void execute(Instruction&&);

    void executeOneInstruction();

    // CPU main loop - will fetch & decode until stopped
    void mainLoop();

    // CPU main loop when halted (HLT) - will do nothing until an IRQ is raised
    void haltedLoop();

    BYTE fetchOpcodeByte();
    WORD fetchOpcodeWord();
    DWORD fetchOpcodeDWord();

    void push32(DWORD value);
    DWORD pop32();

    void push16(WORD value);
    WORD pop16();

    Debugger& debugger() { return *m_debugger; }

    /*!
        Writes an 8-bit value to an output port.
     */
    void out(WORD port, BYTE value);

    /*!
        Reads an 8-bit value from an input port.
     */
    BYTE in(WORD port);

    BYTE* memoryPointer(DWORD address);
    BYTE* memoryPointer(WORD segment, DWORD offset);
    BYTE* memoryPointer(SegmentRegisterIndex, DWORD offset);
    BYTE* memoryPointer(const SegmentDescriptor&, DWORD offset);

    DWORD getEFlags() const;
    WORD getFlags() const;
    void setEFlags(DWORD flags);
    void setFlags(WORD flags);

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

    template<typename T> T readRegister(int registerIndex);
    template<typename T> void writeRegister(int registerIndex, T value);

    // These are faster than readMemory*() but will not access VGA memory, etc.
    inline BYTE readUnmappedMemory8(DWORD address) const;
    inline WORD readUnmappedMemory16(DWORD address) const;
    inline DWORD readUnmappedMemory32(DWORD address) const;
    inline void writeUnmappedMemory8(DWORD address, BYTE data);
    inline void writeUnmappedMemory16(DWORD address, WORD data);

    enum class MemoryAccessType { Read, Write, InternalPointer };

    template<typename T> bool validatePhysicalAddress(DWORD, MemoryAccessType);
    template<typename T> void validateAddress(const SegmentDescriptor&, DWORD offset, MemoryAccessType);
    template<typename T> void validateAddress(SegmentRegisterIndex, DWORD offset, MemoryAccessType);
    template<typename T> void validateAddress(WORD segment, DWORD offset, MemoryAccessType);
    template<typename T> T readMemory(DWORD address);
    template<typename T> T readMemory(WORD segment, DWORD address);
    template<typename T> T readMemory(const SegmentDescriptor&, DWORD address);
    template<typename T> T readMemory(SegmentRegisterIndex, DWORD address);
    template<typename T> void writeMemory(DWORD address, T data);
    template<typename T> void writeMemory(WORD segment, DWORD address, T data);
    template<typename T> void writeMemory(const SegmentDescriptor&, DWORD address, T data);
    template<typename T> void writeMemory(SegmentRegisterIndex, DWORD address, T data);

    void translateAddress(DWORD linearAddress, DWORD& physicalAddress, MemoryAccessType);

    BYTE readMemory8(DWORD address);
    BYTE readMemory8(WORD segment, DWORD offset);
    BYTE readMemory8(SegmentRegisterIndex, DWORD offset);
    WORD readMemory16(DWORD address);
    WORD readMemory16(WORD segment, DWORD offset);
    WORD readMemory16(SegmentRegisterIndex, DWORD offset);
    DWORD readMemory32(DWORD address);
    DWORD readMemory32(WORD segment, DWORD offset);
    DWORD readMemory32(SegmentRegisterIndex, DWORD offset);
    void writeMemory8(DWORD address, BYTE data);
    void writeMemory8(WORD segment, DWORD offset, BYTE data);
    void writeMemory8(SegmentRegisterIndex, DWORD offset, BYTE data);
    void writeMemory16(DWORD address, WORD data);
    void writeMemory16(WORD segment, DWORD offset, WORD data);
    void writeMemory16(SegmentRegisterIndex, DWORD offset, WORD data);
    void writeMemory32(DWORD address, DWORD data);
    void writeMemory32(WORD segment, DWORD offset, DWORD data);
    void writeMemory32(SegmentRegisterIndex, DWORD offset, DWORD data);

    BYTE readModRM8(BYTE rmbyte);
    WORD readModRM16(BYTE rmbyte);
    DWORD readModRM32(BYTE rmbyte);
    void readModRM48(BYTE rmbyte, WORD& segment, DWORD& offset);
    FarPointer readModRMFarPointerSegmentFirst(MemoryOrRegisterReference&);
    FarPointer readModRMFarPointerOffsetFirst(MemoryOrRegisterReference&);
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

    // Dumps all ISR handler pointers (0000:0000 - 0000:03FF)
    void dumpIVT();

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

    bool a16() const { return !m_addressSize32; }
    virtual bool a32() const override { return m_addressSize32; }
    bool o16() const { return !m_operandSize32; }
    virtual bool o32() const override { return m_operandSize32; }

    bool s16() const { return !m_stackSize32; }
    bool s32() const { return m_stackSize32; }

    void nextSI(int size) { this->regs.W.SI += (getDF() ? -size : size); }
    void nextDI(int size) { this->regs.W.DI += (getDF() ? -size : size); }
    void nextESI(int size) { this->regs.D.ESI += (getDF() ? -size : size); }
    void nextEDI(int size) { this->regs.D.EDI += (getDF() ? -size : size); }

    enum Command { EnterMainLoop, ExitMainLoop, SoftReboot, HardReboot };
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

    void _CS(Instruction&);
    void _DS(Instruction&);
    void _ES(Instruction&);
    void _SS(Instruction&);

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

    void _LOOP_imm8(Instruction&);
    void _LOOPE_imm8(Instruction&);
    void _LOOPNE_imm8(Instruction&);

    void _REP(Instruction&);
    void _REPNE(Instruction&);

    void _XCHG_AX_reg16(Instruction&);
    void _XCHG_EAX_reg32(Instruction&);
    void _XCHG_reg8_RM8(Instruction&);
    void _XCHG_reg16_RM16(Instruction&);
    void _XCHG_reg32_RM32(Instruction&);

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

    void _MOV_AL_imm8(Instruction&);
    void _MOV_BL_imm8(Instruction&);
    void _MOV_CL_imm8(Instruction&);
    void _MOV_DL_imm8(Instruction&);
    void _MOV_AH_imm8(Instruction&);
    void _MOV_BH_imm8(Instruction&);
    void _MOV_CH_imm8(Instruction&);
    void _MOV_DH_imm8(Instruction&);

    void _MOV_AX_imm16(Instruction&);
    void _MOV_BX_imm16(Instruction&);
    void _MOV_CX_imm16(Instruction&);
    void _MOV_DX_imm16(Instruction&);
    void _MOV_BP_imm16(Instruction&);
    void _MOV_SP_imm16(Instruction&);
    void _MOV_SI_imm16(Instruction&);
    void _MOV_DI_imm16(Instruction&);

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

    // 80186+ INSTRUCTIONS

    void _BOUND(Instruction&);
    void _ENTER_16(Instruction&);
    void _ENTER_32(Instruction&);
    void _LEAVE(Instruction&);

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

    // 80386+ INSTRUCTIONS

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

    void _FS(Instruction&);
    void _GS(Instruction&);

    void _MOV_RM32_reg32(Instruction&);
    void _MOV_reg32_RM32(Instruction&);
    void _MOV_reg32_CR(Instruction&);
    void _MOV_CR_reg32(Instruction&);
    void _MOV_reg32_DR(Instruction&);
    void _MOV_DR_reg32(Instruction&);
    void _MOV_moff32_EAX(Instruction&);
    void _MOV_EAX_imm32(Instruction&);
    void _MOV_EBX_imm32(Instruction&);
    void _MOV_ECX_imm32(Instruction&);
    void _MOV_EDX_imm32(Instruction&);
    void _MOV_EBP_imm32(Instruction&);
    void _MOV_ESP_imm32(Instruction&);
    void _MOV_ESI_imm32(Instruction&);
    void _MOV_EDI_imm32(Instruction&);

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
    void _AddressSizeOverride(Instruction&);
    void _OperandSizeOverride(Instruction&);

    void _LOCK(Instruction&);

    // REP* helper.
    void handleRepeatOpcode(Instruction&&, bool shouldEqual);

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

    void didTouchMemory(DWORD address);

    void translateAddressSlowCase(DWORD linearAddress, DWORD& physicalAddress, MemoryAccessType);

    template<typename T>
    T rightShift(T, int steps);

    template<typename T>
    T leftShift(T, int steps);

    template<typename T> T doSHLD(T, T, int steps);
    template<typename T> T doSHRD(T, T, int steps);

    template<typename T> T doRol(T, int steps);
    template<typename T> T doRor(T, int steps);

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

    void flushCommandQueue();

    void setLDT(WORD segment);
    void taskSwitch(WORD task, JumpType);
    void taskSwitch(TSSDescriptor&, JumpType);
    TSS currentTSS();

    void writeToGDT(Descriptor&);

    void dumpSelector(const char* segmentRegisterName, SegmentRegisterIndex);
    void setSegmentRegister(SegmentRegisterIndex, WORD selector);
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
    bool CF, DF, TF, PF, AF, ZF, SF, IF, OF;

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

    DWORD CR0, CR1, CR2, CR3, CR4, CR5, CR6, CR7;
    DWORD DR0, DR1, DR2, DR3, DR4, DR5, DR6, DR7;

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

    bool m_stackSize32 { false };

    bool m_shouldBreakOutOfMainLoop { false };
    bool m_shouldSoftReboot { false };
    bool m_shouldHardReboot { false };

    QVector<WatchedAddress> m_watches;

    QMutex m_commandMutex;
    bool m_hasCommands { false };
    QQueue<Command> m_commandQueue;

    bool m_shouldRestoreSizesAfterOverride { false };

    bool m_isForAutotest { false };

    QWORD m_cycle { 0 };
};

extern CPU* g_cpu;

DWORD cpu_sar(CPU&, DWORD, BYTE, BYTE);
DWORD cpu_rcl(CPU&, DWORD, BYTE, BYTE);
DWORD cpu_rcr(CPU&, DWORD, BYTE, BYTE);

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

#include "debug.h"

bool CPU::evaluate(BYTE conditionCode) const
{
    ASSERT(conditionCode <= 0xF);

    switch (conditionCode) {
    case  0: return this->OF;                            // O
    case  1: return !this->OF;                           // NO
    case  2: return this->CF;                            // B, C, NAE
    case  3: return !this->CF;                           // NB, NC, AE
    case  4: return this->ZF;                            // E, Z
    case  5: return !this->ZF;                           // NE, NZ
    case  6: return (this->CF | this->ZF);               // BE, NA
    case  7: return !(this->CF | this->ZF);              // NBE, A
    case  8: return this->SF;                            // S
    case  9: return !this->SF;                           // NS
    case 10: return this->PF;                            // P, PE
    case 11: return !this->PF;                           // NP, PO
    case 12: return this->SF ^ this->OF;                 // L, NGE
    case 13: return !(this->SF ^ this->OF);              // NL, GE
    case 14: return (this->SF ^ this->OF) | this->ZF;    // LE, NG
    case 15: return !((this->SF ^ this->OF) | this->ZF); // NLE, G
    }
    return 0;
}

#endif
