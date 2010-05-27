#ifndef VCPU_H
#define VCPU_H

#include "types.h"

#ifdef VOMIT_DETECT_UNINITIALIZED_ACCESS
#include "debug.h"
#endif

// MACROS AND CONVENIENCE METHODS
#define IS_VGA_MEMORY(address) ((address) >= 0xA0000 && (address) < 0xB0000)

inline DWORD vomit_toFlatAddress(WORD segment, WORD offset)
{
    return (segment << 4) + offset;
}

inline void vomit_write16ToPointer(WORD* pointer, WORD value)
{
#ifdef VOMIT_BIG_ENDIAN
    *pointer = V_BYTESWAP(value);
#else
    *pointer = value;
#endif
}

inline DWORD vomit_read32FromPointer(DWORD* pointer)
{
#ifdef VOMIT_BIG_ENDIAN
    return V_BYTESWAP(*pointer);
#else
    return *pointer;
#endif
}

inline WORD vomit_read16FromPointer(WORD* pointer)
{
#ifdef VOMIT_BIG_ENDIAN
    return V_BYTESWAP(*pointer);
#else
    return *pointer;
#endif
}

inline WORD signext(BYTE b)
{
    WORD w = 0x0000 | b;
    if (w & 0x80)
        return w | 0xFF00;
    else
        return w & 0x00FF;
}

#define rmreg(b) (b>>3&7)    /* Extracts RegID from RM byte. */

#define LSW(d) ((d)&0xFFFF)
#define MSW(d) (((d)&0xFFFF0000)>>16)

#define LSB(w) ((w)&0xFF)
#define MSB(w) (((w)&0xFF00)>>8)

#define FLAT(s,o) (((s)<<4)+(o))

// VCPU MONSTROSITY

class VgaMemory;

class VCpu
{
public:

    enum {
        RegisterAL, RegisterCL, RegisterDL, RegisterBL,
        RegisterAH, RegisterCH, RegisterDH, RegisterBH
    };

    enum {
        RegisterAX, RegisterCX, RegisterDX, RegisterBX,
        RegisterSP, RegisterBP, RegisterSI, RegisterDI
    };

    enum {
        RegisterES, RegisterCS, RegisterSS, RegisterDS,
        RegisterFS, RegisterGS
    };

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
    WORD IP;

    WORD currentSegment() const { return *m_currentSegment; }
    bool hasSegmentPrefix() const { return m_currentSegment == &m_segmentPrefix; }

    void setSegmentPrefix(WORD segment)
    {
        m_segmentPrefix = segment;
        m_currentSegment = &m_segmentPrefix;
    }

    void resetSegmentPrefix() { m_currentSegment = &this->DS; }

    BYTE opcode;
    BYTE rmbyte;

    // Memory size in KiB (will be reported by BIOS)
    DWORD baseMemorySize() const { return m_baseMemorySize; }

    // RAM
    BYTE* memory;

    // ID-to-Register maps
    WORD* treg16[8];
    BYTE* treg8[8];
    WORD* tseg[8];

    typedef void (*OpcodeHandler) (VCpu*);
    OpcodeHandler opcode_handler[0x100];

    void init();
    void kill();

#ifdef VOMIT_DEBUG
    void attachDebugger();
    void detachDebugger();
    bool inDebugger() const;

    void debugger();
#else
    bool inDebugger() const { return false; }
#endif

    void jumpToInterruptHandler(int isr);
    void setInterruptHandler(BYTE isr, WORD segment, WORD offset);

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

    bool getIF() const { return this->IF; }
    bool getCF() const { return this->CF; }
    bool getDF() const { return this->DF; }
    bool getSF() const { return this->SF; }
    bool getAF() const { return this->AF; }
    bool getTF() const { return this->TF; }
    bool getOF() const { return this->OF; }
    bool getPF() const { return this->PF; }
    bool getZF() const { return this->ZF; }

    WORD getCS() const { return this->CS; }
    WORD getIP() const { return this->IP; }

    WORD getDS() const { return this->DS; }
    WORD getES() const { return this->ES; }
    WORD getSS() const { return this->SS; }

    // Base CS:IP is the start address of the currently executing instruction
    WORD getBaseCS() const { return m_baseCS; }
    WORD getBaseIP() const { return m_baseIP; }

    void jump(WORD segment, WORD offset);
    void jumpRelative8(SIGNED_BYTE displacement);
    void jumpRelative16(SIGNED_WORD displacement);
    void jumpAbsolute16(WORD offset);

    // Execute the next instruction at CS:IP
    void exec();

    // CPU main loop - will fetch & decode until stopped
    void mainLoop();

    // CPU main loop when halted (HLT) - will do nothing until an IRQ is raised
    void haltedLoop();

#ifdef VOMIT_PREFETCH_QUEUE
    void flushFetchQueue();
    BYTE fetchOpcodeByte();
    WORD fetchOpcodeWord();
#else
    void flushFetchQueue() {}
    BYTE fetchOpcodeByte() { return m_codeMemory[this->IP++]; }
    inline WORD fetchOpcodeWord();
#endif

    void push(WORD value);
    WORD pop();

    /*!
        Writes an 8-bit value to an output port.
     */
    void out(WORD port, BYTE value);

    /*!
        Reads an 8-bit value from an input port.
     */
    BYTE in(WORD port);

    inline BYTE* memoryPointer(WORD segment, WORD offset) const;

    WORD getFlags();
    void setFlags(WORD flags);

    inline bool evaluate(BYTE) const;

    void updateFlags(WORD value, BYTE bits);
    void updateFlags16(WORD value);
    void updateFlags8(BYTE value);
    void mathFlags8(DWORD result, BYTE dest, BYTE src);
    void mathFlags16(DWORD result, WORD dest, WORD src);
    void cmpFlags8(DWORD result, BYTE dest, BYTE src);
    void cmpFlags16(DWORD result, WORD dest, WORD src);

    // These are faster than readMemory*() but will not access VGA memory, etc.
    inline BYTE readUnmappedMemory8(DWORD address) const;
    inline WORD readUnmappedMemory16(DWORD address) const;
    inline void writeUnmappedMemory8(DWORD address, BYTE data);
    inline void writeUnmappedMemory16(DWORD address, WORD data);

    inline BYTE readMemory8(DWORD address) const;
    inline BYTE readMemory8(WORD segment, WORD offset) const;
    inline WORD readMemory16(DWORD address) const;
    inline WORD readMemory16(WORD segment, WORD offset) const;
    inline DWORD readMemory32(DWORD address) const;
    inline DWORD readMemory32(WORD segment, WORD offset) const;
    inline void writeMemory8(DWORD address, BYTE data);
    inline void writeMemory8(WORD segment, WORD offset, BYTE data);
    inline void writeMemory16(DWORD address, WORD data);
    inline void writeMemory16(WORD segment, WORD offset, WORD data);

    VgaMemory* vgaMemory;

    enum State { Dead, Alive, Halted };
    State state() const { return m_state; }
    void setState(State s) { m_state = s; }

    // TODO: make private
    inline BYTE* codeMemory() const;

    /* TODO: actual PIT implementation.. */
    inline bool tick();

    void registerDefaultOpcodeHandlers();

    // Dumps some basic information about this CPU
    void dump();

    // Dumps registers, flags & stack
    void dumpAll();

#ifdef VOMIT_DETECT_UNINITIALIZED_ACCESS
    void markDirty(DWORD address) { m_dirtMap[address] = true; }
#endif

private:
    void setOpcodeHandler(BYTE rangeStart, BYTE rangeEnd, OpcodeHandler handler);

    DWORD m_instructionsPerTick;

    // This points to the base of CS for fast opcode fetches.
    BYTE* m_codeMemory;

    bool CF, DF, TF, PF, AF, ZF, SF, IF, OF;

    State m_state;

#ifdef VOMIT_DEBUG
    bool m_inDebugger;
    bool m_debugOneStep;
#endif

    // Cycle counter. May wrap arbitrarily.
    DWORD m_pitCountdown;

    // Actual CS:IP (when we started fetching the instruction)
    WORD m_baseCS;
    WORD m_baseIP;

#ifdef VOMIT_PREFETCH_QUEUE
    BYTE* m_prefetchQueue;
    BYTE m_prefetchQueueIndex;
    BYTE m_prefetchQueueSize;
#endif

    WORD* m_currentSegment;
    WORD m_segmentPrefix;

#ifdef VOMIT_DETECT_UNINITIALIZED_ACCESS
    bool* m_dirtMap;
#endif

    // FIXME: Don't befriend this... thing.
    friend void unspeakable_abomination();
    DWORD m_baseMemorySize;
};

extern VCpu* g_cpu;

void vomit_cpu_setAF(VCpu*, DWORD result, WORD dest, WORD src);

WORD cpu_add8( byte, byte );
WORD cpu_sub8( byte, byte );
WORD cpu_mul8( byte, byte );
WORD cpu_div8( byte, byte );
sigword cpu_imul8( sigbyte, sigbyte );

DWORD cpu_add16(VCpu*, WORD, WORD);
DWORD cpu_sub16(VCpu*, WORD, WORD);
DWORD cpu_mul16(VCpu*, WORD, WORD);
DWORD cpu_div16(VCpu*, WORD, WORD);
SIGNED_DWORD cpu_imul16(VCpu*, SIGNED_WORD, SIGNED_WORD);

BYTE cpu_or8(VCpu*, BYTE, BYTE);
BYTE cpu_and8(VCpu*, BYTE, BYTE);
BYTE cpu_xor8(VCpu*, BYTE, BYTE);
WORD cpu_or16(VCpu*, WORD, WORD);
WORD cpu_and16(VCpu*, WORD, WORD);
WORD cpu_xor16(VCpu*, WORD, WORD);

DWORD cpu_shl(VCpu*, word, byte, byte);
DWORD cpu_shr(VCpu*, word, byte, byte);
DWORD cpu_sar(VCpu*, word, byte, byte);
DWORD cpu_rcl(VCpu*, word, byte, byte);
DWORD cpu_rcr(VCpu*, word, byte, byte);
DWORD cpu_rol(VCpu*, word, byte, byte);
DWORD cpu_ror(VCpu*, word, byte, byte);

BYTE vomit_cpu_modrm_read8(VCpu*, BYTE rm);
WORD vomit_cpu_modrm_read16(VCpu*, BYTE rm);
DWORD vomit_cpu_modrm_read32(VCpu*, BYTE rm);
void vomit_cpu_modrm_write8(VCpu*, BYTE rm, BYTE value);
void vomit_cpu_modrm_write16(VCpu*, BYTE rm, WORD value);

/*!
    Writes an 8-bit value back to the most recently resolved ModR/M location.
 */
void vomit_cpu_modrm_update8(VCpu*, BYTE value);

/*!
    Writes a 16-bit value back to the most recently resolved ModR/M location.
 */
void vomit_cpu_modrm_update16(VCpu*, WORD value);

void* vomit_cpu_modrm_resolve8(VCpu*, BYTE rm);
void* vomit_cpu_modrm_resolve16(VCpu*, BYTE rm);

void _UNSUPP(VCpu*);
void _ESCAPE(VCpu*);

void _NOP(VCpu*);
void _HLT(VCpu*);
void _INT_imm8(VCpu*);
void _INT3(VCpu*);
void _INTO(VCpu*);
void _IRET(VCpu*);

void _AAA(VCpu*);
void _AAM(VCpu*);
void _AAD(VCpu*);
void _AAS(VCpu*);

void _DAA(VCpu*);
void _DAS(VCpu*);

void _STC(VCpu*);
void _STD(VCpu*);
void _STI(VCpu*);
void _CLC(VCpu*);
void _CLD(VCpu*);
void _CLI(VCpu*);
void _CMC(VCpu*);

void _CBW(VCpu*);
void _CWD(VCpu*);

void _XLAT(VCpu*);

void _CS(VCpu*);
void _DS(VCpu*);
void _ES(VCpu*);
void _SS(VCpu*);

void _SALC(VCpu*);

void _JMP_imm16(VCpu*);
void _JMP_imm16_imm16(VCpu*);
void _JMP_short_imm8(VCpu*);
void _Jcc_imm8(VCpu*);
void _JCXZ_imm8(VCpu*);

void _JO_imm8(VCpu*);
void _JNO_imm8(VCpu*);
void _JC_imm8(VCpu*);
void _JNC_imm8(VCpu*);
void _JZ_imm8(VCpu*);
void _JNZ_imm8(VCpu*);
void _JNA_imm8(VCpu*);
void _JA_imm8(VCpu*);
void _JS_imm8(VCpu*);
void _JNS_imm8(VCpu*);
void _JP_imm8(VCpu*);
void _JNP_imm8(VCpu*);
void _JL_imm8(VCpu*);
void _JNL_imm8(VCpu*);
void _JNG_imm8(VCpu*);
void _JG_imm8(VCpu*);

void _CALL_imm16(VCpu*);
void _RET(VCpu*);
void _RET_imm16(VCpu*);
void _RETF(VCpu*);
void _RETF_imm16(VCpu*);

void _LOOP_imm8(VCpu*);
void _LOOPE_imm8(VCpu*);
void _LOOPNE_imm8(VCpu*);

void _REP(VCpu*);
void _REPNE(VCpu*);

void _XCHG_AX_reg16(VCpu*);
void _XCHG_reg8_RM8(VCpu*);
void _XCHG_reg16_RM16(VCpu*);

void _CMPSB(VCpu*);
void _CMPSW(VCpu*);
void _LODSB(VCpu*);
void _LODSW(VCpu*);
void _SCASB(VCpu*);
void _SCASW(VCpu*);
void _STOSB(VCpu*);
void _STOSW(VCpu*);
void _MOVSB(VCpu*);
void _MOVSW(VCpu*);

void _LEA_reg16_mem16(VCpu*);

void _LDS_reg16_mem16(VCpu*);
void _LES_reg16_mem16(VCpu*);

void _MOV_AL_imm8(VCpu*);
void _MOV_BL_imm8(VCpu*);
void _MOV_CL_imm8(VCpu*);
void _MOV_DL_imm8(VCpu*);
void _MOV_AH_imm8(VCpu*);
void _MOV_BH_imm8(VCpu*);
void _MOV_CH_imm8(VCpu*);
void _MOV_DH_imm8(VCpu*);

void _MOV_AX_imm16(VCpu*);
void _MOV_BX_imm16(VCpu*);
void _MOV_CX_imm16(VCpu*);
void _MOV_DX_imm16(VCpu*);
void _MOV_BP_imm16(VCpu*);
void _MOV_SP_imm16(VCpu*);
void _MOV_SI_imm16(VCpu*);
void _MOV_DI_imm16(VCpu*);

void _MOV_seg_RM16(VCpu*);
void _MOV_RM16_seg(VCpu*);
void _MOV_AL_moff8(VCpu*);
void _MOV_AX_moff16(VCpu*);
void _MOV_moff8_AL(VCpu*);
void _MOV_moff16_AX(VCpu*);
void _MOV_reg8_RM8(VCpu*);
void _MOV_reg16_RM16(VCpu*);
void _MOV_RM8_reg8(VCpu*);
void _MOV_RM16_reg16(VCpu*);
void _MOV_RM8_imm8(VCpu*);
void _MOV_RM16_imm16(VCpu*);

void _XOR_RM8_reg8(VCpu*);
void _XOR_RM16_reg16(VCpu*);
void _XOR_reg8_RM8(VCpu*);
void _XOR_reg16_RM16(VCpu*);
void _XOR_RM8_imm8(VCpu*);
void _XOR_RM16_imm16(VCpu*);
void _XOR_RM16_imm8(VCpu*);
void _XOR_AX_imm16(VCpu*);
void _XOR_AL_imm8(VCpu*);

void _OR_RM8_reg8(VCpu*);
void _OR_RM16_reg16(VCpu*);
void _OR_reg8_RM8(VCpu*);
void _OR_reg16_RM16(VCpu*);
void _OR_RM8_imm8(VCpu*);
void _OR_RM16_imm16(VCpu*);
void _OR_RM16_imm8(VCpu*);
void _OR_AX_imm16(VCpu*);
void _OR_AL_imm8(VCpu*);

void _AND_RM8_reg8(VCpu*);
void _AND_RM16_reg16(VCpu*);
void _AND_reg8_RM8(VCpu*);
void _AND_reg16_RM16(VCpu*);
void _AND_RM8_imm8(VCpu*);
void _AND_RM16_imm16(VCpu*);
void _AND_RM16_imm8(VCpu*);
void _AND_AX_imm16(VCpu*);
void _AND_AL_imm8(VCpu*);

void _TEST_RM8_reg8(VCpu*);
void _TEST_RM16_reg16(VCpu*);
void _TEST_AX_imm16(VCpu*);
void _TEST_AL_imm8(VCpu*);

void _PUSH_SP_8086_80186(VCpu*);
void _PUSH_AX(VCpu*);
void _PUSH_BX(VCpu*);
void _PUSH_CX(VCpu*);
void _PUSH_DX(VCpu*);
void _PUSH_BP(VCpu*);
void _PUSH_SP(VCpu*);
void _PUSH_SI(VCpu*);
void _PUSH_DI(VCpu*);
void _POP_AX(VCpu*);
void _POP_BX(VCpu*);
void _POP_CX(VCpu*);
void _POP_DX(VCpu*);
void _POP_BP(VCpu*);
void _POP_SP(VCpu*);
void _POP_SI(VCpu*);
void _POP_DI(VCpu*);
void _PUSH_CS(VCpu*);
void _PUSH_DS(VCpu*);
void _PUSH_ES(VCpu*);
void _PUSH_SS(VCpu*);
void _PUSHF(VCpu*);

void _POP_CS(VCpu*);
void _POP_DS(VCpu*);
void _POP_ES(VCpu*);
void _POP_SS(VCpu*);
void _POPF(VCpu*);

void _LAHF(VCpu*);
void _SAHF(VCpu*);

void _OUT_imm8_AL(VCpu*);
void _OUT_imm8_AX(VCpu*);
void _OUT_DX_AL(VCpu*);
void _OUT_DX_AX(VCpu*);
void _OUTSB(VCpu*);
void _OUTSW(VCpu*);

void _IN_AL_imm8(VCpu*);
void _IN_AX_imm8(VCpu*);
void _IN_AL_DX(VCpu*);
void _IN_AX_DX(VCpu*);

void _ADD_RM8_reg8(VCpu*);
void _ADD_RM16_reg16(VCpu*);
void _ADD_reg8_RM8(VCpu*);
void _ADD_reg16_RM16(VCpu*);
void _ADD_AL_imm8(VCpu*);
void _ADD_AX_imm16(VCpu*);
void _ADD_RM8_imm8(VCpu*);
void _ADD_RM16_imm16(VCpu*);
void _ADD_RM16_imm8(VCpu*);

void _SUB_RM8_reg8(VCpu*);
void _SUB_RM16_reg16(VCpu*);
void _SUB_reg8_RM8(VCpu*);
void _SUB_reg16_RM16(VCpu*);
void _SUB_AL_imm8(VCpu*);
void _SUB_AX_imm16(VCpu*);
void _SUB_RM8_imm8(VCpu*);
void _SUB_RM16_imm16(VCpu*);
void _SUB_RM16_imm8(VCpu*);

void _ADC_RM8_reg8(VCpu*);
void _ADC_RM16_reg16(VCpu*);
void _ADC_reg8_RM8(VCpu*);
void _ADC_reg16_RM16(VCpu*);
void _ADC_AL_imm8(VCpu*);
void _ADC_AX_imm16(VCpu*);
void _ADC_RM8_imm8(VCpu*);
void _ADC_RM16_imm16(VCpu*);
void _ADC_RM16_imm8(VCpu*);

void _SBB_RM8_reg8(VCpu*);
void _SBB_RM16_reg16(VCpu*);
void _SBB_reg8_RM8(VCpu*);
void _SBB_reg16_RM16(VCpu*);
void _SBB_AL_imm8(VCpu*);
void _SBB_AX_imm16(VCpu*);
void _SBB_RM8_imm8(VCpu*);
void _SBB_RM16_imm16(VCpu*);
void _SBB_RM16_imm8(VCpu*);

void _CMP_RM8_reg8(VCpu*);
void _CMP_RM16_reg16(VCpu*);
void _CMP_reg8_RM8(VCpu*);
void _CMP_reg16_RM16(VCpu*);
void _CMP_AL_imm8(VCpu*);
void _CMP_AX_imm16(VCpu*);
void _CMP_RM8_imm8(VCpu*);
void _CMP_RM16_imm16(VCpu*);
void _CMP_RM16_imm8(VCpu*);

void _MUL_RM8(VCpu*);
void _MUL_RM16(VCpu*);
void _DIV_RM8(VCpu*);
void _DIV_RM16(VCpu*);
void _IMUL_RM8(VCpu*);
void _IMUL_RM16(VCpu*);
void _IDIV_RM8(VCpu*);
void _IDIV_RM16(VCpu*);

void _TEST_RM8_imm8(VCpu*);
void _TEST_RM16_imm16(VCpu*);
void _NOT_RM8(VCpu*);
void _NOT_RM16(VCpu*);
void _NEG_RM8(VCpu*);
void _NEG_RM16(VCpu*);

void _INC_RM16(VCpu*);
void _INC_reg16(VCpu*);
void _DEC_RM16(VCpu*);
void _DEC_reg16(VCpu*);

void _CALL_RM16(VCpu*);
void _CALL_FAR_mem16(VCpu*);
void _CALL_imm16_imm16(VCpu*);

void _JMP_RM16(VCpu*);
void _JMP_FAR_mem16(VCpu*);

void _PUSH_RM16(VCpu*);
void _POP_RM16(VCpu*);

void _wrap_0x80(VCpu*);
void _wrap_0x81(VCpu*);
void _wrap_0x83(VCpu*);
void _wrap_0x8F(VCpu*);
void _wrap_0xC0(VCpu*);
void _wrap_0xC1(VCpu*);
void _wrap_0xD0(VCpu*);
void _wrap_0xD1(VCpu*);
void _wrap_0xD2(VCpu*);
void _wrap_0xD3(VCpu*);
void _wrap_0xF6(VCpu*);
void _wrap_0xF7(VCpu*);
void _wrap_0xFE(VCpu*);
void _wrap_0xFF(VCpu*);

// 80186+ INSTRUCTIONS

void _wrap_0x0F(VCpu*);

void _BOUND(VCpu*);
void _ENTER(VCpu*);
void _LEAVE(VCpu*);

void _PUSHA(VCpu*);
void _POPA(VCpu*);
void _PUSH_imm8(VCpu*);
void _PUSH_imm16(VCpu*);

void _IMUL_reg16_RM16_imm8(VCpu*);

// INLINE IMPLEMENTATIONS

#include "vga_memory.h"

BYTE VCpu::readUnmappedMemory8(DWORD address) const
{
    return this->memory[address];
}

WORD VCpu::readUnmappedMemory16(DWORD address) const
{
    return vomit_read16FromPointer(reinterpret_cast<WORD*>(this->memory + address));
}

void VCpu::writeUnmappedMemory8(DWORD address, BYTE value)
{
#ifdef VOMIT_DETECT_UNINITIALIZED_ACCESS
    m_dirtMap[address] = true;
#endif
    this->memory[address] = value;
}

void VCpu::writeUnmappedMemory16(DWORD address, WORD value)
{
#ifdef VOMIT_DETECT_UNINITIALIZED_ACCESS
    m_dirtMap[address] = true;
    m_dirtMap[address + 1] = true;
#endif
    vomit_write16ToPointer(reinterpret_cast<WORD*>(this->memory + address), value);
}

BYTE VCpu::readMemory8(DWORD address) const
{
    if (IS_VGA_MEMORY(address))
        return this->vgaMemory->read8(address);
#ifdef VOMIT_DETECT_UNINITIALIZED_ACCESS
    if (!m_dirtMap[address])
        vlog(VM_MEMORYMSG, "%04X:%04X: Uninitialized read from %08X", getBaseCS(), getBaseIP(), address);
#endif
    return this->memory[address];
}

BYTE VCpu::readMemory8(WORD segment, WORD offset) const
{
    return readMemory8(vomit_toFlatAddress(segment, offset));
}

WORD VCpu::readMemory16(DWORD address) const
{
    if (IS_VGA_MEMORY(address))
        return this->vgaMemory->read16(address);
#ifdef VOMIT_DETECT_UNINITIALIZED_ACCESS
    if (!m_dirtMap[address] || !m_dirtMap[address + 1])
        vlog(VM_MEMORYMSG, "%04X:%04X: Uninitialized read from %08X", getBaseCS(), getBaseIP(), address);
#endif
    return vomit_read16FromPointer(reinterpret_cast<WORD*>(this->memory + address));
}

WORD VCpu::readMemory16(WORD segment, WORD offset) const
{
#if VOMIT_CPU_LEVEL == 0
    // FIXME: Broken for VGA read at 0xFFFF although that's beyond unlikely to occur.
    if (offset == 0xFFFF)
        return this->memory[vomit_toFlatAddress(segment, offset)] | (this->memory[vomit_toFlatAddress(segment, 0)] << 8);
#endif
    return readMemory16(vomit_toFlatAddress(segment, offset));
}

DWORD VCpu::readMemory32(DWORD address) const
{
    if (IS_VGA_MEMORY(address))
        return this->vgaMemory->read16(address) | (this->vgaMemory->read16(address + 2) << 16);
#ifdef VOMIT_DETECT_UNINITIALIZED_ACCESS
    if (!m_dirtMap[address] || !m_dirtMap[address + 1] || !m_dirtMap[address + 2] || !m_dirtMap[address + 3])
        vlog(VM_MEMORYMSG, "%04X:%04X: Uninitialized read from %08X", getBaseCS(), getBaseIP(), address);
#endif
    return vomit_read32FromPointer(reinterpret_cast<DWORD*>(this->memory + address));
}

DWORD VCpu::readMemory32(WORD segment, WORD offset) const
{
#if VOMIT_CPU_LEVEL == 0
    // FIXME: Broken for VGA read at 0xFFFF although that's beyond unlikely to occur.
    if (offset == 0xFFFF)
        return this->memory[vomit_toFlatAddress(segment, offset)] |
               (this->memory[vomit_toFlatAddress(segment, 0)] << 8) |
               (this->memory[vomit_toFlatAddress(segment, 1)] << 16) |
               (this->memory[vomit_toFlatAddress(segment, 2)] << 24);
#endif
    return readMemory32(vomit_toFlatAddress(segment, offset));
}

void VCpu::writeMemory8(DWORD address, BYTE value)
{
    if (IS_VGA_MEMORY(address)) {
        this->vgaMemory->write8(address, value);
        return;
    }

#ifdef VOMIT_DETECT_UNINITIALIZED_ACCESS
    m_dirtMap[address] = true;
#endif

    this->memory[address] = value;
}

void VCpu::writeMemory8(WORD segment, WORD offset, BYTE value)
{
    writeMemory8(vomit_toFlatAddress(segment, offset), value);
}

void VCpu::writeMemory16(DWORD address, WORD value)
{
    if (IS_VGA_MEMORY(address)) {
        this->vgaMemory->write16(address, value);
        return;
    }

#ifdef VOMIT_DETECT_UNINITIALIZED_ACCESS
    m_dirtMap[address] = true;
    m_dirtMap[address + 1] = true;
#endif

    WORD* ptr = reinterpret_cast<WORD*>(this->memory + address);
    vomit_write16ToPointer(ptr, value);
}

void VCpu::writeMemory16(WORD segment, WORD offset, WORD value)
{
#if VOMIT_CPU_LEVEL == 0
    // FIXME: Broken for VGA write at 0xFFFF although that's beyond unlikely to occur.
    if (offset == 0xFFFF) {
        this->memory[vomit_toFlatAddress(segment, offset)] = value & 0xFF;
        this->memory[vomit_toFlatAddress(segment, 0)] = value >> 8;
        return;
    }
#endif

    writeMemory16(FLAT(segment, offset), value);
}

BYTE* VCpu::codeMemory() const
{
    return m_codeMemory;
}

BYTE* VCpu::memoryPointer(WORD segment, WORD offset) const
{
    return this->memory + vomit_toFlatAddress(segment, offset);
}

#ifndef VOMIT_PREFETCH_QUEUE
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
#endif

bool VCpu::tick()
{
    if (--m_pitCountdown == 0) {
        m_pitCountdown = m_instructionsPerTick;
        return true;
    }
    return false;
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
