/*
 * Copyright (C) 2003-2013 Andreas Kling <kling@webkit.org>
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

#include "vomit.h"
#include "debug.h"
#include <QtCore/QMutex>
#include <QtCore/QQueue>

class Debugger;
class Machine;
class VGAMemory;

class VCpu
{
public:
    VCpu(Machine*);
    ~VCpu();

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

    enum SegmentIndex {
        RegisterES = 0,
        RegisterCS,
        RegisterSS,
        RegisterDS,
        RegisterFS,
        RegisterGS
    };

    struct SegmentSelector {
        unsigned DPL;
        unsigned count;
        bool present;
        bool big;
        bool granularity;
        bool _32bit;
        bool CE;
        bool BRW;
        bool acc;
        DWORD base;
        DWORD limit;
    };

    WORD currentSegment() const { return *m_currentSegment; }
    bool hasSegmentPrefix() const { return m_currentSegment == &m_segmentPrefix; }

    void setSegmentPrefix(WORD segment)
    {
        m_segmentPrefix = segment;
        m_currentSegment = &m_segmentPrefix;
    }

    void resetSegmentPrefix() { m_currentSegment = &this->DS; }

    // Extended memory size in KiB (will be reported by CMOS)
    DWORD extendedMemorySize() const { return m_extendedMemorySize; }
    void setExtendedMemorySize(DWORD size) { m_extendedMemorySize = size; }

    // Conventional memory size in KiB (will be reported by CMOS)
    DWORD baseMemorySize() const { return m_baseMemorySize; }
    void setBaseMemorySize(DWORD size) { m_baseMemorySize = size; }

    void kill();

    void setA20Enabled(bool value) { m_a20Enabled = value; }
    bool isA20Enabled() const { return m_a20Enabled; }

    void jumpToInterruptHandler(int isr);

    void GP(int code);

    void exception(int ec) { this->IP = getBaseIP(); jumpToInterruptHandler(ec); }

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
    unsigned int getCPL() const { return this->CPL; }
    bool getNT() const { return this->NT; }
    bool getVIP() const { return this->VIP; }
    bool getVIF() const { return this->VIF; }
    bool getVM() const { return this->VM; }
    bool getPE() const { return this->CR0 & 0x01; }
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

    void setCS(WORD cs) { this->CS = cs; }
    void setDS(WORD ds) { this->DS = ds; }
    void setES(WORD es) { this->ES = es; }
    void setSS(WORD ss) { this->SS = ss; }
    void setFS(WORD fs) { this->FS = fs; }
    void setGS(WORD gs) { this->GS = gs; }

    void setIP(WORD ip) { this->IP = ip; }
    void setEIP(DWORD eip) { this->EIP = eip; }

    DWORD getRegister32(RegisterIndex32 registerIndex) const { return *treg32[registerIndex]; }
    WORD getRegister16(RegisterIndex16 registerIndex) const { return *treg16[registerIndex]; }
    BYTE getRegister8(RegisterIndex8 registerIndex) const { return *treg8[registerIndex]; }

    void setRegister32(RegisterIndex32 registerIndex, DWORD value) { *treg32[registerIndex] = value; }
    void setRegister16(RegisterIndex16 registerIndex, WORD value) { *treg16[registerIndex] = value; }
    void setRegister8(RegisterIndex8 registerIndex, BYTE value) { *treg8[registerIndex] = value; }

    WORD getSegment(SegmentIndex segmentIndex) const { ASSERT_VALID_SEGMENT_INDEX(segmentIndex); return *m_segmentMap[segmentIndex]; }
    void setSegment(SegmentIndex segmentIndex, WORD value) const { ASSERT_VALID_SEGMENT_INDEX(segmentIndex); *m_segmentMap[segmentIndex] = value; }

    DWORD getControlRegister(int registerIndex) const { return *m_controlRegisterMap[registerIndex]; }
    void setControlRegister(int registerIndex, DWORD value) { *m_controlRegisterMap[registerIndex] = value; }

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
    WORD getBaseEIP() const { return m_baseEIP; }

    void jump32(WORD segment, DWORD offset);
    void jump16(WORD segment, WORD offset);
    void jumpRelative8(SIGNED_BYTE displacement);
    void jumpRelative16(SIGNED_WORD displacement);
    void jumpRelative32(SIGNED_DWORD displacement);
    void jumpAbsolute16(WORD offset);

    // Execute the next instruction at CS:IP (huge switch version)
    void decodeNext();

    // Execute the specified instruction
    void decode(BYTE op);

    // Execute the next instruction at CS:IP
    void exec();

    // CPU main loop - will fetch & decode until stopped
    void mainLoop();

    // CPU main loop when halted (HLT) - will do nothing until an IRQ is raised
    void haltedLoop();

    BYTE fetchOpcodeByte() { return m_codeMemory[this->IP++]; }
    inline WORD fetchOpcodeWord();
    inline DWORD fetchOpcodeDWord();

    void push32(DWORD value);
    DWORD pop32();

    void push(WORD value);
    WORD pop();

    Debugger* debugger() { return m_debugger; }

    /*!
        Writes an 8-bit value to an output port.
     */
    void out(WORD port, BYTE value);

    /*!
        Reads an 8-bit value from an input port.
     */
    BYTE in(WORD port);

    inline BYTE* memoryPointer(WORD segment, WORD offset) const;
    inline BYTE* memoryPointer(DWORD address) const;

    DWORD getEFlags() const;
    WORD getFlags() const;
    void setEFlags(DWORD flags);
    void setFlags(WORD flags);

    inline bool evaluate(BYTE) const;

    void updateFlags(WORD value, BYTE bits);
    void updateFlags32(DWORD value);
    void updateFlags16(WORD value);
    void updateFlags8(BYTE value);
    void mathFlags8(WORD result, BYTE dest, BYTE src);
    void mathFlags16(DWORD result, WORD dest, WORD src);
    void mathFlags32(QWORD result, DWORD dest, DWORD src);
    void cmpFlags8(DWORD result, BYTE dest, BYTE src);
    void cmpFlags16(DWORD result, WORD dest, WORD src);
    void cmpFlags32(QWORD result, DWORD dest, DWORD src);

    void adjustFlag32(DWORD result, WORD dest, WORD src);

    // These are faster than readMemory*() but will not access VGA memory, etc.
    inline BYTE readUnmappedMemory8(DWORD address) const;
    inline WORD readUnmappedMemory16(DWORD address) const;
    inline void writeUnmappedMemory8(DWORD address, BYTE data);
    inline void writeUnmappedMemory16(DWORD address, WORD data);

    BYTE readMemory8(DWORD address) const;
    BYTE readMemory8(WORD segment, WORD offset) const;
    WORD readMemory16(DWORD address) const;
    WORD readMemory16(WORD segment, WORD offset) const;
    DWORD readMemory32(DWORD address) const;
    DWORD readMemory32(WORD segment, WORD offset) const;
    void writeMemory8(DWORD address, BYTE data);
    void writeMemory8(WORD segment, WORD offset, BYTE data);
    void writeMemory16(DWORD address, WORD data);
    void writeMemory16(WORD segment, WORD offset, WORD data);
    void writeMemory32(DWORD address, DWORD data);
    void writeMemory32(WORD segment, WORD offset, DWORD data);

    BYTE readModRM8(BYTE rmbyte);
    WORD readModRM16(BYTE rmbyte);
    DWORD readModRM32(BYTE rmbyte);
    void writeModRM8(BYTE rmbyte, BYTE value);
    void writeModRM16(BYTE rmbyte, WORD value);
    void writeModRM32(BYTE rmbyte, DWORD value);

    /*!
        Writes an 8-bit value back to the most recently resolved ModR/M location.
     */
    void updateModRM8(BYTE value);

    /*!
        Writes a 16-bit value back to the most recently resolved ModR/M location.
     */
    void updateModRM16(WORD value);

    /*!
        Writes a 32-bit value back to the most recently resolved ModR/M location.
     */
    void updateModRM32(DWORD value);

    void* resolveModRM8(BYTE rmbyte);
    void* resolveModRM16(BYTE rmbyte);
    void* resolveModRM32(BYTE rmbyte);

    DWORD evaluateSIB(BYTE sib);

    enum Mode { RealMode, ProtectedMode };
    Mode mode() const { return m_mode; }
    void setMode(Mode m) { m_mode = m; }

    enum State { Dead, Alive, Halted };
    State state() const { return m_state; }
    void setState(State s) { m_state = s; }

    // Dumps registers, flags & stack
    void dumpAll() const;

    // Dumps all ISR handler pointers (0000:0000 - 0000:03FF)
    void dumpIVT() const;

    void dumpMemory(WORD segment, DWORD offset, int rows) const;

    int dumpDisassembled(WORD segment, DWORD offset) const;

#ifdef VOMIT_TRACE
    // Dumps registers (used by --trace)
    void dumpTrace() const;
#endif

    bool a16() const { return !m_addressSize32; }
    bool a32() const { return m_addressSize32; }
    bool o16() const { return !m_operationSize32; }
    bool o32() const { return m_operationSize32; }

    void nextSI(int size) { this->regs.W.SI += (getDF() ? -size : size); }
    void nextDI(int size) { this->regs.W.DI += (getDF() ? -size : size); }
    void nextESI(int size) { this->regs.D.ESI += (getDF() ? -size : size); }
    void nextEDI(int size) { this->regs.D.EDI += (getDF() ? -size : size); }

    enum Command { EnterMainLoop, ExitMainLoop };
    void queueCommand(Command);

protected:
    void _UNSUPP();
    void _ESCAPE();

    void _NOP();
    void _HLT();
    void _INT_imm8();
    void _INT3();
    void _INTO();
    void _IRET();

    void _AAA();
    void _AAM();
    void _AAD();
    void _AAS();

    void _DAA();
    void _DAS();

    void _STC();
    void _STD();
    void _STI();
    void _CLC();
    void _CLD();
    void _CLI();
    void _CMC();

    void _CBW();
    void _CWD();
    void _CWDE();
    void _CDQ();

    void _XLAT();

    void _CS();
    void _DS();
    void _ES();
    void _SS();

    void _SALC();

    void _JMP_imm32();
    void _JMP_imm16();
    void _JMP_imm16_imm16();
    void _JMP_short_imm8();
    void _Jcc_imm8();
    void _JCXZ_imm8();
    void _JECXZ_imm8();

    void _JO_imm8();
    void _JNO_imm8();
    void _JC_imm8();
    void _JNC_imm8();
    void _JZ_imm8();
    void _JNZ_imm8();
    void _JNA_imm8();
    void _JA_imm8();
    void _JS_imm8();
    void _JNS_imm8();
    void _JP_imm8();
    void _JNP_imm8();
    void _JL_imm8();
    void _JNL_imm8();
    void _JNG_imm8();
    void _JG_imm8();

    void _JO_NEAR_imm();
    void _JNO_NEAR_imm();
    void _JC_NEAR_imm();
    void _JNC_NEAR_imm();
    void _JZ_NEAR_imm();
    void _JNZ_NEAR_imm();
    void _JNA_NEAR_imm();
    void _JA_NEAR_imm();
    void _JS_NEAR_imm();
    void _JNS_NEAR_imm();
    void _JP_NEAR_imm();
    void _JNP_NEAR_imm();
    void _JL_NEAR_imm();
    void _JNL_NEAR_imm();
    void _JNG_NEAR_imm();
    void _JG_NEAR_imm();

    void _CALL_imm16();
    void _CALL_imm32();
    void _RET();
    void _RET_imm16();
    void _RETF();
    void _RETF_imm16();

    void _LOOP_imm8();
    void _LOOPE_imm8();
    void _LOOPNE_imm8();

    void _REP();
    void _REPNE();

    void _XCHG_AX_reg16();
    void _XCHG_EAX_reg32();
    void _XCHG_reg8_RM8();
    void _XCHG_reg16_RM16();
    void _XCHG_reg32_RM32();

    void _CMPSB();
    void _CMPSW();
    void _CMPSD();
    void _LODSB();
    void _LODSW();
    void _LODSD();
    void _SCASB();
    void _SCASW();
    void _SCASD();
    void _STOSB();
    void _STOSW();
    void _STOSD();
    void _MOVSB();
    void _MOVSW();
    void _MOVSD();

    void _LEA_reg16_mem16();
    void _LEA_reg32_mem32();

    void _LDS_reg16_mem16();
    void _LDS_reg32_mem32();
    void _LES_reg16_mem16();
    void _LES_reg32_mem32();

    void _MOV_AL_imm8();
    void _MOV_BL_imm8();
    void _MOV_CL_imm8();
    void _MOV_DL_imm8();
    void _MOV_AH_imm8();
    void _MOV_BH_imm8();
    void _MOV_CH_imm8();
    void _MOV_DH_imm8();

    void _MOV_AX_imm16();
    void _MOV_BX_imm16();
    void _MOV_CX_imm16();
    void _MOV_DX_imm16();
    void _MOV_BP_imm16();
    void _MOV_SP_imm16();
    void _MOV_SI_imm16();
    void _MOV_DI_imm16();

    void _MOV_seg_RM16();
    void _MOV_RM16_seg();
    void _MOV_RM32_seg();
    void _MOV_AL_moff8();
    void _MOV_AX_moff16();
    void _MOV_EAX_moff32();
    void _MOV_moff8_AL();
    void _MOV_moff16_AX();
    void _MOV_reg8_RM8();
    void _MOV_reg16_RM16();
    void _MOV_RM8_reg8();
    void _MOV_RM16_reg16();
    void _MOV_RM8_imm8();
    void _MOV_RM16_imm16();
    void _MOV_RM32_imm32();

    void _XOR_RM8_reg8();
    void _XOR_RM16_reg16();
    void _XOR_reg8_RM8();
    void _XOR_reg16_RM16();
    void _XOR_reg32_RM32();
    void _XOR_RM8_imm8();
    void _XOR_RM16_imm16();
    void _XOR_RM16_imm8();
    void _XOR_AL_imm8();
    void _XOR_AX_imm16();
    void _XOR_EAX_imm32();

    void _OR_RM8_reg8();
    void _OR_RM16_reg16();
    void _OR_RM32_reg32();
    void _OR_reg8_RM8();
    void _OR_reg16_RM16();
    void _OR_reg32_RM32();
    void _OR_RM8_imm8();
    void _OR_RM16_imm16();
    void _OR_RM16_imm8();
    void _OR_EAX_imm32();
    void _OR_AX_imm16();
    void _OR_AL_imm8();

    void _AND_RM8_reg8();
    void _AND_RM16_reg16();
    void _AND_reg8_RM8();
    void _AND_reg16_RM16();
    void _AND_RM8_imm8();
    void _AND_RM16_imm16();
    void _AND_RM16_imm8();
    void _AND_AL_imm8();
    void _AND_AX_imm16();
    void _AND_EAX_imm32();

    void _TEST_RM8_reg8();
    void _TEST_RM16_reg16();
    void _TEST_RM32_reg32();
    void _TEST_AL_imm8();
    void _TEST_AX_imm16();
    void _TEST_EAX_imm32();

    void _PUSH_SP_8086_80186();
    void _PUSH_CS();
    void _PUSH_DS();
    void _PUSH_ES();
    void _PUSH_SS();
    void _PUSHF();

    void _POP_DS();
    void _POP_ES();
    void _POP_SS();
    void _POPF();

    void _LAHF();
    void _SAHF();

    void _OUT_imm8_AL();
    void _OUT_imm8_AX();
    void _OUT_imm8_EAX();
    void _OUT_DX_AL();
    void _OUT_DX_AX();
    void _OUT_DX_EAX();
    void _OUTSB();
    void _OUTSW();
    void _OUTSD();

    void _IN_AL_imm8();
    void _IN_AX_imm8();
    void _IN_EAX_imm8();
    void _IN_AL_DX();
    void _IN_AX_DX();
    void _IN_EAX_DX();

    void _ADD_RM8_reg8();
    void _ADD_RM16_reg16();
    void _ADD_reg8_RM8();
    void _ADD_reg16_RM16();
    void _ADD_AL_imm8();
    void _ADD_AX_imm16();
    void _ADD_EAX_imm32();
    void _ADD_RM8_imm8();
    void _ADD_RM16_imm16();
    void _ADD_RM16_imm8();

    void _SUB_RM8_reg8();
    void _SUB_RM16_reg16();
    void _SUB_reg8_RM8();
    void _SUB_reg16_RM16();
    void _SUB_AL_imm8();
    void _SUB_AX_imm16();
    void _SUB_EAX_imm32();
    void _SUB_RM8_imm8();
    void _SUB_RM16_imm16();
    void _SUB_RM16_imm8();

    void _ADC_RM8_reg8();
    void _ADC_RM16_reg16();
    void _ADC_reg8_RM8();
    void _ADC_reg16_RM16();
    void _ADC_AL_imm8();
    void _ADC_AX_imm16();
    void _ADC_EAX_imm32();
    void _ADC_RM8_imm8();
    void _ADC_RM16_imm16();
    void _ADC_RM16_imm8();

    void _SBB_RM8_reg8();
    void _SBB_RM16_reg16();
    void _SBB_RM32_reg32();
    void _SBB_reg8_RM8();
    void _SBB_reg16_RM16();
    void _SBB_AL_imm8();
    void _SBB_AX_imm16();
    void _SBB_EAX_imm32();
    void _SBB_RM8_imm8();
    void _SBB_RM16_imm16();
    void _SBB_RM16_imm8();

    void _CMP_RM8_reg8();
    void _CMP_RM16_reg16();
    void _CMP_RM32_reg32();
    void _CMP_reg8_RM8();
    void _CMP_reg16_RM16();
    void _CMP_reg32_RM32();
    void _CMP_AL_imm8();
    void _CMP_AX_imm16();
    void _CMP_EAX_imm32();
    void _CMP_RM8_imm8();
    void _CMP_RM16_imm16();
    void _CMP_RM16_imm8();

    void _MUL_RM8();
    void _MUL_RM16();
    void _MUL_RM32();
    void _DIV_RM8();
    void _DIV_RM16();
    void _DIV_RM32();
    void _IMUL_RM8();
    void _IMUL_RM16();
    void _IMUL_RM32();
    void _IDIV_RM8();
    void _IDIV_RM16();
    void _IDIV_RM32();

    void _TEST_RM8_imm8();
    void _TEST_RM16_imm16();
    void _NOT_RM8();
    void _NOT_RM16();
    void _NEG_RM8();
    void _NEG_RM16();

    void _INC_RM8();
    void _INC_RM16();
    void _INC_reg16();
    void _INC_reg32();
    void _DEC_RM8();
    void _DEC_RM16();
    void _DEC_reg16();
    void _DEC_reg32();

    void _CALL_RM16();
    void _CALL_FAR_mem16();
    void _CALL_imm16_imm16();
    void _CALL_imm16_imm32();

    void _JMP_RM16();
    void _JMP_FAR_mem16();

    void _PUSH_RM16();
    void _POP_RM16();
    void _POP_RM32();

    void _wrap_0x80();
    void _wrap_0x81_16();
    void _wrap_0x81_32();
    void _wrap_0x83_16();
    void _wrap_0x83_32();
    void _wrap_0x8F_16();
    void _wrap_0x8F_32();
    void _wrap_0xC0();
    void _wrap_0xC1_16();
    void _wrap_0xC1_32();
    void _wrap_0xD0();
    void _wrap_0xD1_16();
    void _wrap_0xD1_32();
    void _wrap_0xD2();
    void _wrap_0xD3_16();
    void _wrap_0xD3_32();
    void _wrap_0xF6();
    void _wrap_0xF7_16();
    void _wrap_0xF7_32();
    void _wrap_0xFE();
    void _wrap_0xFF_16();
    void _wrap_0xFF_32();

    // 80186+ INSTRUCTIONS

    void _wrap_0x0F();

    void _BOUND();
    void _ENTER();
    void _LEAVE();

    void _PUSHA();
    void _POPA();
    void _PUSH_imm8();
    void _PUSH_imm16();

    void _IMUL_reg16_RM16_imm8();

    // 80386+ INSTRUCTIONS

    void _LMSW();
    void _SMSW();

    void _SGDT();
    void _LGDT();
    void _SIDT();
    void _LIDT();

    void _PUSHAD();
    void _POPAD();
    void _PUSHFD();
    void _POPFD();
    void _PUSH_imm32();

    void _PUSH_reg16();
    void _PUSH_reg32();
    void _POP_reg16();
    void _POP_reg32();

    void _TEST_RM32_imm32();
    void _XOR_RM32_reg32();
    void _ADD_RM32_reg32();
    void _ADC_RM32_reg32();
    void _SUB_RM32_reg32();

    void _MOVZX_reg16_RM8();
    void _MOVZX_reg32_RM8();
    void _MOVZX_reg32_RM16();

    void _LFS_reg16_mem16();
    void _LFS_reg32_mem32();
    void _LGS_reg16_mem16();
    void _LGS_reg32_mem32();

    void _PUSH_FS();
    void _PUSH_GS();
    void _POP_FS();
    void _POP_GS();

    void _FS();
    void _GS();

    void _MOV_RM32_reg32();
    void _MOV_reg32_RM32();
    void _MOV_reg32_CR();
    void _MOV_CR_reg32();
    void _MOV_moff32_EAX();
    void _MOV_EAX_imm32();
    void _MOV_EBX_imm32();
    void _MOV_ECX_imm32();
    void _MOV_EDX_imm32();
    void _MOV_EBP_imm32();
    void _MOV_ESP_imm32();
    void _MOV_ESI_imm32();
    void _MOV_EDI_imm32();

    void _MOV_seg_RM32();

    void _JMP_imm16_imm32();

    void _ADD_RM32_imm32();
    void _OR_RM32_imm32();
    void _ADC_RM32_imm32();
    void _SBB_RM32_imm32();
    void _AND_RM32_imm32();
    void _SUB_RM32_imm32();
    void _XOR_RM32_imm32();
    void _CMP_RM32_imm32();

    void _ADD_RM32_imm8();
    void _OR_RM32_imm8();
    void _ADC_RM32_imm8();
    void _SBB_RM32_imm8();
    void _AND_RM32_imm8();
    void _SUB_RM32_imm8();
    void _XOR_RM32_imm8();
    void _CMP_RM32_imm8();

    void _ADD_reg32_RM32();
    void _ADC_reg32_RM32();
    void _SBB_reg32_RM32();
    void _AND_reg32_RM32();
    void _SUB_reg32_RM32();

    void _AND_RM32_reg32();

    void _UD0();
    void _AddressSizeOverride();
    void _OperationSizeOverride();

    // REP* helper.
    void handleRepeatOpcode(BYTE opcode, bool shouldEqual);

private:
    template<typename T>
    T rightShift(T, int steps);

    template<typename T>
    T leftShift(T, int steps);

    enum ValueSize {
        ByteSize,
        WordSize,
        DWordSize
    };

    void* resolveModRM_internal(BYTE rmbyte, ValueSize size);
    void* resolveModRM8_internal(BYTE rmbyte);
    void* resolveModRM16_internal(BYTE rmbyte);
    void* resolveModRM32_internal(BYTE rmbyte);

    void saveBaseAddress()
    {
        m_baseCS = getCS();
        m_baseEIP = getEIP();
    }

    inline BYTE* codeMemory() const;

    void flushCommandQueue();

    void dumpSelector(const char* segmentRegisterName, SegmentIndex) const;
    void syncSegmentRegister(SegmentIndex);
    SegmentSelector m_selector[6];

    // This points to the base of CS for fast opcode fetches.
    BYTE* m_codeMemory;

    union {
        struct {
            DWORD EAX, EBX, ECX, EDX;
            DWORD EBP, ESP, ESI, EDI;
            DWORD EIP;
        } D;
#ifdef VOMIT_BIG_ENDIAN
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

    unsigned int CPL;
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
        WORD limit;
    } GDTR;

    struct {
        DWORD base;
        WORD limit;
    } IDTR;

    struct {
        WORD segment;
        DWORD base;
        WORD limit;
        // LDT's index in GDT
        int index;
    } LDTR;

    DWORD CR0, CR1, CR2, CR3, CR4, CR5, CR6, CR7;
    DWORD DR0, DR1, DR2, DR3, DR4, DR5, DR6, DR7;

    union {
        struct {
#ifdef VOMIT_BIG_ENDIAN
            WORD __EIP_high_word, IP;
#else
            WORD IP, __EIP_high_word;
#endif
        };
        DWORD EIP;
    };

    struct {
        WORD segment;
        DWORD base;
        WORD limit;
    } TR;

    BYTE opcode;
    BYTE rmbyte;
    BYTE subrmbyte;

    State m_state;

    Mode m_mode;

#ifdef VOMIT_DEBUG
    bool m_inDebugger;
    bool m_debugOneStep;
#endif

    // Actual CS:EIP (when we started fetching the instruction)
    WORD m_baseCS;
    DWORD m_baseEIP;

    WORD* m_currentSegment;
    WORD m_segmentPrefix;

    mutable void* m_lastModRMPointer;
    mutable WORD m_lastModRMSegment;
    mutable DWORD m_lastModRMOffset;

    DWORD m_baseMemorySize;
    DWORD m_extendedMemorySize;

    QList<DWORD> m_breakPoints;

    bool m_a20Enabled;

    Debugger* m_debugger;

    BYTE* m_memory;

    WORD* m_segmentMap[8];
    DWORD* m_controlRegisterMap[8];

    // ID-to-Register maps
    DWORD* treg32[8];
    WORD* treg16[8];
    BYTE* treg8[8];

    Machine* machine() const { return m_machine; }
    Machine* m_machine;

    bool m_addressSize32;
    bool m_operationSize32;

    bool m_shouldBreakOutOfMainLoop;

    QQueue<Command> m_commandQueue;
    QMutex m_commandMutex;
};

extern VCpu* g_cpu;

WORD cpu_add8(BYTE, BYTE);
WORD cpu_sub8(BYTE, BYTE);
WORD cpu_mul8(BYTE, BYTE);
WORD cpu_div8(BYTE, BYTE);
SIGNED_WORD cpu_imul8(SIGNED_BYTE, SIGNED_BYTE);

DWORD cpu_add16(VCpu*, WORD, WORD);
QWORD cpu_add32(VCpu*, DWORD, DWORD);
DWORD cpu_sub16(VCpu*, WORD, WORD);
QWORD cpu_sub32(VCpu*, DWORD, DWORD);
DWORD cpu_mul16(VCpu*, WORD, WORD);
DWORD cpu_div16(VCpu*, WORD, WORD);
SIGNED_DWORD cpu_imul16(VCpu*, SIGNED_WORD, SIGNED_WORD);

BYTE cpu_or8(VCpu*, BYTE, BYTE);
BYTE cpu_and8(VCpu*, BYTE, BYTE);
BYTE cpu_xor8(VCpu*, BYTE, BYTE);
WORD cpu_or16(VCpu*, WORD, WORD);
WORD cpu_and16(VCpu*, WORD, WORD);
WORD cpu_xor16(VCpu*, WORD, WORD);
DWORD cpu_or32(VCpu*, DWORD, DWORD);
DWORD cpu_xor32(VCpu*, DWORD, DWORD);
DWORD cpu_and32(VCpu*, DWORD, DWORD);

DWORD cpu_sar(VCpu*, WORD, BYTE, BYTE);
DWORD cpu_rcl(VCpu*, WORD, BYTE, BYTE);
DWORD cpu_rcr(VCpu*, WORD, BYTE, BYTE);
DWORD cpu_rol(VCpu*, WORD, BYTE, BYTE);
DWORD cpu_ror(VCpu*, WORD, BYTE, BYTE);

// INLINE IMPLEMENTATIONS

BYTE VCpu::readUnmappedMemory8(DWORD address) const
{
    return m_memory[address];
}

WORD VCpu::readUnmappedMemory16(DWORD address) const
{
    return vomit_read16FromPointer(reinterpret_cast<WORD*>(m_memory + address));
}

void VCpu::writeUnmappedMemory8(DWORD address, BYTE value)
{
    m_memory[address] = value;
}

void VCpu::writeUnmappedMemory16(DWORD address, WORD value)
{
    vomit_write16ToPointer(reinterpret_cast<WORD*>(m_memory + address), value);
}

BYTE* VCpu::codeMemory() const
{
    return m_codeMemory;
}

BYTE* VCpu::memoryPointer(DWORD address) const
{
    if (!isA20Enabled()) {
#ifdef VOMIT_DEBUG
        if (address > 0xFFFFF)
            vlog(LogCPU, "%04X:%08X Get pointer to %08X with A20 disabled, wrapping to %08X", getBaseCS(), getBaseEIP(), address, address & 0xFFFFF);
#endif
        address &= 0xFFFFF;
    }
    return &m_memory[address];
}

BYTE* VCpu::memoryPointer(WORD segment, WORD offset) const
{
    return memoryPointer(vomit_toFlatAddress(segment, offset));
}

WORD VCpu::fetchOpcodeWord()
{
    WORD w = *reinterpret_cast<WORD*>(&m_codeMemory[getIP()]);
    this->IP += 2;
#ifdef VOMIT_BIG_ENDIAN
    return V_BYTESWAP(w);
#else
    return w;
#endif
}

DWORD VCpu::fetchOpcodeDWord()
{
    DWORD d = *reinterpret_cast<DWORD*>(&m_codeMemory[getIP()]);
    this->IP += 4;
#ifdef VOMIT_BIG_ENDIAN
#error IMPLEMENT ME
#else
    return d;
#endif
}

#include "debug.h"

bool VCpu::evaluate(BYTE conditionCode) const
{
    VM_ASSERT(conditionCode <= 0xF);

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
