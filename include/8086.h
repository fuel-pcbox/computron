#ifndef __8086_h__
#define __8086_h__

#define CPU_DEAD            0
#define CPU_ALIVE            1
#define CPU_HALTED            2

#define INTEL_8086          0
#define INTEL_80186         1
#define INTEL_80286         2

#define rmreg(b) (b>>3&7)    /* Extracts RegID from RM byte. */
#define MODRM_ISREG(b) (((b)&0xC0)==0xC0)

#define LSW(d) ((d)&0xFFFF)
#define MSW(d) (((d)&0xFFFF0000)>>16)

#define LSB(w) ((w)&0xFF)
#define MSB(w) (((w)&0xFF00)>>8)

#define FLAT(s,o) (((s)<<4)+(o))

#define SET_SEGMENT_PREFIX(cpu, segment) do { (cpu)->SegmentPrefix = (cpu)->segment; (cpu)->CurrentSegment = &(cpu)->SegmentPrefix; } while(0);
#define RESET_SEGMENT_PREFIX(cpu) do { (cpu)->CurrentSegment = &(cpu)->DS; } while(0);

typedef void (*vomit_opcode_handler) (struct __vomit_cpu_t *);

class VgaMemory;

typedef struct __vomit_cpu_t {
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
    WORD CS, DS, ES, SS, FS, GS, SegmentPrefix, *CurrentSegment;
    WORD IP;
    BYTE type;
    BYTE state;

    WORD base_CS;
    WORD base_IP;

    BYTE opcode;
    BYTE rmbyte;

    /* Memory size in KiB (will be reported by BIOS) */
    WORD memory_size;

    /* RAM */
    BYTE *memory;

    /* This points to the base of CS for fast opcode fetches. */
    BYTE *code_memory;

    /* Cycle counter. May wrap arbitrarily. */
    DWORD pit_counter;

    WORD *treg16[8];
    BYTE *treg8[8];
    WORD *tseg[8];

    vomit_opcode_handler opcode_handler[0x100];

#ifdef VOMIT_PREFETCH_QUEUE
    BYTE *pfq;
    BYTE pfq_current;
    BYTE pfq_size;
#endif

#ifndef __cplusplus
#error Vomit is a C++ program nowadays
#endif

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

    void exception(int ec) { jumpToInterruptHandler(ec); }

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

    void exec();

    BYTE *memoryPointer(WORD segment, WORD offset) { return &this->memory[FLAT(segment, offset)]; }

    WORD getFlags();
    void setFlags(WORD flags);

    bool evaluate(BYTE);

    void updateFlags(WORD value, BYTE bits);
    void updateFlags16(WORD value);
    void updateFlags8(BYTE value);
    void mathFlags8(DWORD result, BYTE dest, BYTE src);
    void mathFlags16(DWORD result, WORD dest, WORD src);
    void cmpFlags8(DWORD result, BYTE dest, BYTE src);
    void cmpFlags16(DWORD result, WORD dest, WORD src);

    BYTE readMemory8(DWORD flat_address) const;

    VgaMemory *vgaMemory;

    // TODO: make private
    BYTE *codeMemory() { return this->memory; }

private:
    bool CF, DF, TF, PF, AF, ZF, SF, IF, OF;
#ifdef VOMIT_DEBUG
    bool m_inDebugger;
    bool m_debugOneStep;
#endif
} vomit_cpu_t;

#define VCpu vomit_cpu_t

extern VCpu *g_cpu;

#ifdef VOMIT_PREFETCH_QUEUE
void vomit_cpu_pfq_flush(vomit_cpu_t *cpu);
BYTE vomit_cpu_pfq_getbyte(vomit_cpu_t *cpu);
WORD vomit_cpu_pfq_getword(vomit_cpu_t *cpu);
#else
#define vomit_cpu_pfq_flush(cpu)
#define vomit_cpu_pfq_getbyte(cpu) (cpu->code_memory[cpu->IP++])
WORD vomit_cpu_pfq_getword(vomit_cpu_t *cpu);
#endif

void vomit_cpu_jump(vomit_cpu_t *cpu, word segment, word offset);
void vomit_cpu_main(vomit_cpu_t *cpu);
void vomit_cpu_jump_relative8(vomit_cpu_t *cpu, SIGNED_BYTE displacement);
void vomit_cpu_jump_relative16(vomit_cpu_t *cpu, SIGNED_WORD displacement);
void vomit_cpu_jump_absolute16(vomit_cpu_t *cpu, WORD offset);
void vomit_cpu_set_interrupt(vomit_cpu_t *cpu, BYTE isr_index, WORD segment, WORD offset);

/*!
    Writes an 8-bit value to an output port.
 */
void vomit_cpu_out(vomit_cpu_t *cpu, WORD port, BYTE value);

/*!
    Reads an 8-bit value from an input port.
 */
BYTE vomit_cpu_in(vomit_cpu_t *cpu, WORD port);

WORD vomit_cpu_static_flags(vomit_cpu_t *cpu);

void vomit_cpu_setAF(vomit_cpu_t *cpu, DWORD result, WORD dest, WORD src);

WORD cpu_add8( byte, byte );
WORD cpu_sub8( byte, byte );
WORD cpu_mul8( byte, byte );
WORD cpu_div8( byte, byte );
sigword cpu_imul8( sigbyte, sigbyte );

DWORD cpu_add16(vomit_cpu_t *cpu, WORD, WORD);
DWORD cpu_sub16(vomit_cpu_t *cpu, WORD, WORD);
DWORD cpu_mul16(vomit_cpu_t *cpu, WORD, WORD);
DWORD cpu_div16(vomit_cpu_t *cpu, WORD, WORD);
SIGNED_DWORD cpu_imul16(vomit_cpu_t *cpu, SIGNED_WORD, SIGNED_WORD);

BYTE cpu_or8(vomit_cpu_t *cpu, BYTE, BYTE);
BYTE cpu_and8(vomit_cpu_t *cpu, BYTE, BYTE);
BYTE cpu_xor8(vomit_cpu_t *cpu, BYTE, BYTE);
WORD cpu_or16(vomit_cpu_t *cpu, WORD, WORD);
WORD cpu_and16(vomit_cpu_t *cpu, WORD, WORD);
WORD cpu_xor16(vomit_cpu_t *cpu, WORD, WORD);

DWORD cpu_shl(vomit_cpu_t *cpu, word, byte, byte);
DWORD cpu_shr(vomit_cpu_t *cpu, word, byte, byte);
DWORD cpu_sar(vomit_cpu_t *cpu, word, byte, byte);
DWORD cpu_rcl(vomit_cpu_t *cpu, word, byte, byte);
DWORD cpu_rcr(vomit_cpu_t *cpu, word, byte, byte);
DWORD cpu_rol(vomit_cpu_t *cpu, word, byte, byte);
DWORD cpu_ror(vomit_cpu_t *cpu, word, byte, byte);

BYTE vomit_cpu_modrm_read8(vomit_cpu_t *cpu, BYTE rm);
WORD vomit_cpu_modrm_read16(vomit_cpu_t *cpu, BYTE rm);
DWORD vomit_cpu_modrm_read32(vomit_cpu_t *cpu, BYTE rm);
void vomit_cpu_modrm_write8(vomit_cpu_t *cpu, BYTE rm, BYTE value);
void vomit_cpu_modrm_write16(vomit_cpu_t *cpu, BYTE rm, WORD value);

/*!
    Writes an 8-bit value back to the most recently resolved ModR/M location.
 */
void vomit_cpu_modrm_update8(vomit_cpu_t *cpu, BYTE value);

/*!
    Writes a 16-bit value back to the most recently resolved ModR/M location.
 */
void vomit_cpu_modrm_update16(vomit_cpu_t *cpu, WORD value);

void *vomit_cpu_modrm_resolve8(vomit_cpu_t *cpu, BYTE rm);
void *vomit_cpu_modrm_resolve16(vomit_cpu_t *cpu, BYTE rm);

void vomit_cpu_push(vomit_cpu_t *cpu, WORD value);
WORD vomit_cpu_pop(vomit_cpu_t *cpu);

BYTE vomit_cpu_memory_read8(vomit_cpu_t *cpu, WORD segment, WORD offset);
WORD vomit_cpu_memory_read16(vomit_cpu_t *cpu, WORD segment, WORD offset);
void vomit_cpu_memory_write8(vomit_cpu_t *cpu, WORD segment, WORD offset, BYTE value);
void vomit_cpu_memory_write16(vomit_cpu_t *cpu, WORD segment, WORD offset, WORD value);

inline WORD signext (byte b)
{
    WORD w = 0x0000 | b;
    if ((w&0x80))
        return (w | 0xff00);
    else
        return (w & 0x00ff);
}

#define REG_AL  0
#define REG_CL  1
#define REG_DL  2
#define REG_BL  3
#define REG_AH  4
#define REG_CH  5
#define REG_DH  6
#define REG_BH  7

#define REG_AX  0
#define REG_CX  1
#define REG_DX  2
#define REG_BX  3
#define REG_SP  4
#define REG_BP  5
#define REG_SI  6
#define REG_DI  7

#define REG_ES  0
#define REG_CS  1
#define REG_SS  2
#define REG_DS  3
#define REG_FS  4
#define REG_GS  5

void _UNSUPP(vomit_cpu_t *cpu);
void _ESCAPE(vomit_cpu_t *cpu);

void _NOP(vomit_cpu_t *cpu);
void _HLT(vomit_cpu_t *cpu);
void _INT_imm8(vomit_cpu_t *cpu);
void _INT3(vomit_cpu_t *cpu);
void _INTO(vomit_cpu_t *cpu);
void _IRET(vomit_cpu_t *cpu);

void _AAA(vomit_cpu_t *cpu);
void _AAM(vomit_cpu_t *cpu);
void _AAD(vomit_cpu_t *cpu);
void _AAS(vomit_cpu_t *cpu);

void _DAA(vomit_cpu_t *cpu);
void _DAS(vomit_cpu_t *cpu);

void _STC(vomit_cpu_t *cpu);
void _STD(vomit_cpu_t *cpu);
void _STI(vomit_cpu_t *cpu);
void _CLC(vomit_cpu_t *cpu);
void _CLD(vomit_cpu_t *cpu);
void _CLI(vomit_cpu_t *cpu);
void _CMC(vomit_cpu_t *cpu);

void _CBW(vomit_cpu_t *cpu);
void _CWD(vomit_cpu_t *cpu);

void _XLAT(vomit_cpu_t *cpu);

void _CS(vomit_cpu_t *cpu);
void _DS(vomit_cpu_t *cpu);
void _ES(vomit_cpu_t *cpu);
void _SS(vomit_cpu_t *cpu);

void _SALC(vomit_cpu_t *cpu);

void _JMP_imm16(vomit_cpu_t *cpu);
void _JMP_imm16_imm16(vomit_cpu_t *cpu);
void _JMP_short_imm8(vomit_cpu_t *cpu);
void _Jcc_imm8(vomit_cpu_t *cpu);
void _JCXZ_imm8(vomit_cpu_t *cpu);

void _JO_imm8(vomit_cpu_t *cpu);
void _JNO_imm8(vomit_cpu_t *cpu);
void _JC_imm8(vomit_cpu_t *cpu);
void _JNC_imm8(vomit_cpu_t *cpu);
void _JZ_imm8(vomit_cpu_t *cpu);
void _JNZ_imm8(vomit_cpu_t *cpu);
void _JNA_imm8(vomit_cpu_t *cpu);
void _JA_imm8(vomit_cpu_t *cpu);
void _JS_imm8(vomit_cpu_t *cpu);
void _JNS_imm8(vomit_cpu_t *cpu);
void _JP_imm8(vomit_cpu_t *cpu);
void _JNP_imm8(vomit_cpu_t *cpu);
void _JL_imm8(vomit_cpu_t *cpu);
void _JNL_imm8(vomit_cpu_t *cpu);
void _JNG_imm8(vomit_cpu_t *cpu);
void _JG_imm8(vomit_cpu_t *cpu);

void _CALL_imm16(vomit_cpu_t *cpu);
void _RET(vomit_cpu_t *cpu);
void _RET_imm16(vomit_cpu_t *cpu);
void _RETF(vomit_cpu_t *cpu);
void _RETF_imm16(vomit_cpu_t *cpu);

void _LOOP_imm8(vomit_cpu_t *cpu);
void _LOOPE_imm8(vomit_cpu_t *cpu);
void _LOOPNE_imm8(vomit_cpu_t *cpu);

void _REP(vomit_cpu_t *cpu);
void _REPNE(vomit_cpu_t *cpu);

void _XCHG_AX_reg16(vomit_cpu_t *cpu);
void _XCHG_reg8_RM8(vomit_cpu_t *cpu);
void _XCHG_reg16_RM16(vomit_cpu_t *cpu);

void _CMPSB(vomit_cpu_t *cpu);
void _CMPSW(vomit_cpu_t *cpu);
void _LODSB(vomit_cpu_t *cpu);
void _LODSW(vomit_cpu_t *cpu);
void _SCASB(vomit_cpu_t *cpu);
void _SCASW(vomit_cpu_t *cpu);
void _STOSB(vomit_cpu_t *cpu);
void _STOSW(vomit_cpu_t *cpu);
void _MOVSB(vomit_cpu_t *cpu);
void _MOVSW(vomit_cpu_t *cpu);

void _LEA_reg16_mem16(vomit_cpu_t *cpu);

void _LDS_reg16_mem16(vomit_cpu_t *cpu);
void _LES_reg16_mem16(vomit_cpu_t *cpu);

void _MOV_reg8_imm8(vomit_cpu_t *cpu);
void _MOV_reg16_imm16(vomit_cpu_t *cpu);
void _MOV_seg_RM16(vomit_cpu_t *cpu);
void _MOV_RM16_seg(vomit_cpu_t *cpu);
void _MOV_AL_moff8(vomit_cpu_t *cpu);
void _MOV_AX_moff16(vomit_cpu_t *cpu);
void _MOV_moff8_AL(vomit_cpu_t *cpu);
void _MOV_moff16_AX(vomit_cpu_t *cpu);
void _MOV_reg8_RM8(vomit_cpu_t *cpu);
void _MOV_reg16_RM16(vomit_cpu_t *cpu);
void _MOV_RM8_reg8(vomit_cpu_t *cpu);
void _MOV_RM16_reg16(vomit_cpu_t *cpu);
void _MOV_RM8_imm8(vomit_cpu_t *cpu);
void _MOV_RM16_imm16(vomit_cpu_t *cpu);

void _XOR_RM8_reg8(vomit_cpu_t *cpu);
void _XOR_RM16_reg16(vomit_cpu_t *cpu);
void _XOR_reg8_RM8(vomit_cpu_t *cpu);
void _XOR_reg16_RM16(vomit_cpu_t *cpu);
void _XOR_RM8_imm8(vomit_cpu_t *cpu);
void _XOR_RM16_imm16(vomit_cpu_t *cpu);
void _XOR_RM16_imm8(vomit_cpu_t *cpu);
void _XOR_AX_imm16(vomit_cpu_t *cpu);
void _XOR_AL_imm8(vomit_cpu_t *cpu);

void _OR_RM8_reg8(vomit_cpu_t *cpu);
void _OR_RM16_reg16(vomit_cpu_t *cpu);
void _OR_reg8_RM8(vomit_cpu_t *cpu);
void _OR_reg16_RM16(vomit_cpu_t *cpu);
void _OR_RM8_imm8(vomit_cpu_t *cpu);
void _OR_RM16_imm16(vomit_cpu_t *cpu);
void _OR_RM16_imm8(vomit_cpu_t *cpu);
void _OR_AX_imm16(vomit_cpu_t *cpu);
void _OR_AL_imm8(vomit_cpu_t *cpu);

void _AND_RM8_reg8(vomit_cpu_t *cpu);
void _AND_RM16_reg16(vomit_cpu_t *cpu);
void _AND_reg8_RM8(vomit_cpu_t *cpu);
void _AND_reg16_RM16(vomit_cpu_t *cpu);
void _AND_RM8_imm8(vomit_cpu_t *cpu);
void _AND_RM16_imm16(vomit_cpu_t *cpu);
void _AND_RM16_imm8(vomit_cpu_t *cpu);
void _AND_AX_imm16(vomit_cpu_t *cpu);
void _AND_AL_imm8(vomit_cpu_t *cpu);

void _TEST_RM8_reg8(vomit_cpu_t *cpu);
void _TEST_RM16_reg16(vomit_cpu_t *cpu);
void _TEST_AX_imm16(vomit_cpu_t *cpu);
void _TEST_AL_imm8(vomit_cpu_t *cpu);

void _PUSH_reg16(vomit_cpu_t *cpu);
void _PUSH_seg(vomit_cpu_t *cpu);
void _PUSHF(vomit_cpu_t *cpu);

void _POP_reg16(vomit_cpu_t *cpu);
void _POP_seg(vomit_cpu_t *cpu);
void _POP_CS(vomit_cpu_t *cpu);
void _POPF(vomit_cpu_t *cpu);

void _LAHF(vomit_cpu_t *cpu);
void _SAHF(vomit_cpu_t *cpu);

void _OUT_imm8_AL(vomit_cpu_t *cpu);
void _OUT_imm8_AX(vomit_cpu_t *cpu);
void _OUT_DX_AL(vomit_cpu_t *cpu);
void _OUT_DX_AX(vomit_cpu_t *cpu);
void _OUTSB(vomit_cpu_t *cpu);
void _OUTSW(vomit_cpu_t *cpu);

void _IN_AL_imm8(vomit_cpu_t *cpu);
void _IN_AX_imm8(vomit_cpu_t *cpu);
void _IN_AL_DX(vomit_cpu_t *cpu);
void _IN_AX_DX(vomit_cpu_t *cpu);

void _ADD_RM8_reg8(vomit_cpu_t *cpu);
void _ADD_RM16_reg16(vomit_cpu_t *cpu);
void _ADD_reg8_RM8(vomit_cpu_t *cpu);
void _ADD_reg16_RM16(vomit_cpu_t *cpu);
void _ADD_AL_imm8(vomit_cpu_t *cpu);
void _ADD_AX_imm16(vomit_cpu_t *cpu);
void _ADD_RM8_imm8(vomit_cpu_t *cpu);
void _ADD_RM16_imm16(vomit_cpu_t *cpu);
void _ADD_RM16_imm8(vomit_cpu_t *cpu);

void _SUB_RM8_reg8(vomit_cpu_t *cpu);
void _SUB_RM16_reg16(vomit_cpu_t *cpu);
void _SUB_reg8_RM8(vomit_cpu_t *cpu);
void _SUB_reg16_RM16(vomit_cpu_t *cpu);
void _SUB_AL_imm8(vomit_cpu_t *cpu);
void _SUB_AX_imm16(vomit_cpu_t *cpu);
void _SUB_RM8_imm8(vomit_cpu_t *cpu);
void _SUB_RM16_imm16(vomit_cpu_t *cpu);
void _SUB_RM16_imm8(vomit_cpu_t *cpu);

void _ADC_RM8_reg8(vomit_cpu_t *cpu);
void _ADC_RM16_reg16(vomit_cpu_t *cpu);
void _ADC_reg8_RM8(vomit_cpu_t *cpu);
void _ADC_reg16_RM16(vomit_cpu_t *cpu);
void _ADC_AL_imm8(vomit_cpu_t *cpu);
void _ADC_AX_imm16(vomit_cpu_t *cpu);
void _ADC_RM8_imm8(vomit_cpu_t *cpu);
void _ADC_RM16_imm16(vomit_cpu_t *cpu);
void _ADC_RM16_imm8(vomit_cpu_t *cpu);

void _SBB_RM8_reg8(vomit_cpu_t *cpu);
void _SBB_RM16_reg16(vomit_cpu_t *cpu);
void _SBB_reg8_RM8(vomit_cpu_t *cpu);
void _SBB_reg16_RM16(vomit_cpu_t *cpu);
void _SBB_AL_imm8(vomit_cpu_t *cpu);
void _SBB_AX_imm16(vomit_cpu_t *cpu);
void _SBB_RM8_imm8(vomit_cpu_t *cpu);
void _SBB_RM16_imm16(vomit_cpu_t *cpu);
void _SBB_RM16_imm8(vomit_cpu_t *cpu);

void _CMP_RM8_reg8(vomit_cpu_t *cpu);
void _CMP_RM16_reg16(vomit_cpu_t *cpu);
void _CMP_reg8_RM8(vomit_cpu_t *cpu);
void _CMP_reg16_RM16(vomit_cpu_t *cpu);
void _CMP_AL_imm8(vomit_cpu_t *cpu);
void _CMP_AX_imm16(vomit_cpu_t *cpu);
void _CMP_RM8_imm8(vomit_cpu_t *cpu);
void _CMP_RM16_imm16(vomit_cpu_t *cpu);
void _CMP_RM16_imm8(vomit_cpu_t *cpu);

void _MUL_RM8(vomit_cpu_t *cpu);
void _MUL_RM16(vomit_cpu_t *cpu);
void _DIV_RM8(vomit_cpu_t *cpu);
void _DIV_RM16(vomit_cpu_t *cpu);
void _IMUL_RM8(vomit_cpu_t *cpu);
void _IMUL_RM16(vomit_cpu_t *cpu);
void _IDIV_RM8(vomit_cpu_t *cpu);
void _IDIV_RM16(vomit_cpu_t *cpu);

void _TEST_RM8_imm8(vomit_cpu_t *cpu);
void _TEST_RM16_imm16(vomit_cpu_t *cpu);
void _NOT_RM8(vomit_cpu_t *cpu);
void _NOT_RM16(vomit_cpu_t *cpu);
void _NEG_RM8(vomit_cpu_t *cpu);
void _NEG_RM16(vomit_cpu_t *cpu);

void _INC_RM16(vomit_cpu_t *cpu);
void _INC_reg16(vomit_cpu_t *cpu);
void _DEC_RM16(vomit_cpu_t *cpu);
void _DEC_reg16(vomit_cpu_t *cpu);

void _CALL_RM16(vomit_cpu_t *cpu);
void _CALL_FAR_mem16(vomit_cpu_t *cpu);
void _CALL_imm16_imm16(vomit_cpu_t *cpu);

void _JMP_RM16(vomit_cpu_t *cpu);
void _JMP_FAR_mem16(vomit_cpu_t *cpu);

void _PUSH_RM16(vomit_cpu_t *cpu);
void _POP_RM16(vomit_cpu_t *cpu);

void _wrap_0x80(vomit_cpu_t *cpu);
void _wrap_0x81(vomit_cpu_t *cpu);
void _wrap_0x83(vomit_cpu_t *cpu);
void _wrap_0x8F(vomit_cpu_t *cpu);
void _wrap_0xC0(vomit_cpu_t *cpu);
void _wrap_0xC1(vomit_cpu_t *cpu);
void _wrap_0xD0(vomit_cpu_t *cpu);
void _wrap_0xD1(vomit_cpu_t *cpu);
void _wrap_0xD2(vomit_cpu_t *cpu);
void _wrap_0xD3(vomit_cpu_t *cpu);
void _wrap_0xF6(vomit_cpu_t *cpu);
void _wrap_0xF7(vomit_cpu_t *cpu);
void _wrap_0xFE(vomit_cpu_t *cpu);
void _wrap_0xFF(vomit_cpu_t *cpu);

#endif /* __8086_h__ */
