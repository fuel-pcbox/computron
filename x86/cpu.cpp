// x86/cpu.cpp
// Initialization, main loop, misc instructions

#include "vomit.h"
#include "debug.h"
#include "vga_memory.h"
#include "pic.h"
#include <QtCore/QStringList>

VCpu* g_cpu = 0;
bool g_vomit_exit_main_loop = 0;

#define CALL_HANDLER(handler16, handler32) if (o16()) { handler16(this); } else { handler32(this); }

void _UD0(VCpu* cpu)
{
    vlog(VM_ALERT, "Undefined opcode 0F FF (UD0)");
    cpu->exception(6);
}

void _OperationSizeOverride(VCpu*cpu)
{
    cpu->m_operationSize32 = !cpu->m_operationSize32;

#ifdef VOMIT_DEBUG_OVERRIDE_OPCODES
    vlog(VM_LOGMSG, "%04X:%08X Operation size override detected! Opcode: db 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X ", cpu->getBaseCS(), cpu->getBaseEIP(),
         cpu->readMemory8(cpu->getCS(), cpu->getEIP() - 1),
         cpu->readMemory8(cpu->getCS(), cpu->getEIP()),
         cpu->readMemory8(cpu->getCS(), cpu->getEIP() + 1),
         cpu->readMemory8(cpu->getCS(), cpu->getEIP() + 2),
         cpu->readMemory8(cpu->getCS(), cpu->getEIP() + 3),
         cpu->readMemory8(cpu->getCS(), cpu->getEIP() + 4),
         cpu->readMemory8(cpu->getCS(), cpu->getEIP() + 5)
    );
    cpu->dumpAll();
#endif

    cpu->decodeNext();

    cpu->m_operationSize32 = !cpu->m_operationSize32;
}

void _AddressSizeOverride(VCpu* cpu)
{
    cpu->m_addressSize32 = !cpu->m_addressSize32;

#ifdef VOMIT_DEBUG_OVERRIDE_OPCODES
    vlog(VM_LOGMSG, "%04X:%08X Address size override detected! Opcode: db 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X ", cpu->getBaseCS(), cpu->getBaseEIP(),
         cpu->readMemory8(cpu->getCS(), cpu->getEIP() - 1),
         cpu->readMemory8(cpu->getCS(), cpu->getEIP()),
         cpu->readMemory8(cpu->getCS(), cpu->getEIP() + 1),
         cpu->readMemory8(cpu->getCS(), cpu->getEIP() + 2),
         cpu->readMemory8(cpu->getCS(), cpu->getEIP() + 3),
         cpu->readMemory8(cpu->getCS(), cpu->getEIP() + 4),
         cpu->readMemory8(cpu->getCS(), cpu->getEIP() + 5)
    );
    cpu->dumpAll();
#endif

    cpu->decodeNext();

    cpu->m_addressSize32 = !cpu->m_addressSize32;
}

void VCpu::decodeNext()
{
    this->opcode = fetchOpcodeByte();
    decode(this->opcode);
}

void VCpu::decode(BYTE op)
{
    this->opcode = op;
    switch (this->opcode) {
    case 0x00: _ADD_RM8_reg8(this); break;
    case 0x01: CALL_HANDLER(_ADD_RM16_reg16, _ADD_RM32_reg32); break;
    case 0x02: _ADD_reg8_RM8(this); break;
    case 0x03: CALL_HANDLER(_ADD_reg16_RM16, _ADD_reg32_RM32); break;
    case 0x04: _ADD_AL_imm8(this); break;
    case 0x05: CALL_HANDLER(_ADD_AX_imm16, _ADD_EAX_imm32); break;
    case 0x06: _PUSH_ES(this); break;
    case 0x07: _POP_ES(this); break;
    case 0x08: _OR_RM8_reg8(this); break;
    case 0x09: CALL_HANDLER(_OR_RM16_reg16, _OR_RM32_reg32); break;
    case 0x0A: _OR_reg8_RM8(this); break;
    case 0x0B: CALL_HANDLER(_OR_reg16_RM16, _OR_reg32_RM32); break;
    case 0x0C: _OR_AL_imm8(this); break;
    case 0x0D: CALL_HANDLER(_OR_AX_imm16, _OR_EAX_imm32); break;
    case 0x0E: _PUSH_CS(this); break;
    case 0x0F:
        this->rmbyte = fetchOpcodeByte();
        switch (this->rmbyte) {
        case 0x01:
            this->subrmbyte = fetchOpcodeByte();
            switch (vomit_modRMRegisterPart(this->subrmbyte)) {
            case 0: _SGDT(this); break;
            case 1: _SIDT(this); break;
            case 2: _LGDT(this); break;
            case 3: _LIDT(this); break;
            case 6: _LMSW(this); break;
            default: goto fffuuu;
            }
            break;
        case 0x20: _MOV_reg32_CR(this); break;
        case 0x22: _MOV_CR_reg32(this); break;
        case 0x80: _JO_NEAR_imm(this); break;
        case 0x81: _JNO_NEAR_imm(this); break;
        case 0x82: _JC_NEAR_imm(this); break;
        case 0x83: _JNC_NEAR_imm(this); break;
        case 0x84: _JZ_NEAR_imm(this); break;
        case 0x85: _JNZ_NEAR_imm(this); break;
        case 0x86: _JNA_NEAR_imm(this); break;
        case 0x87: _JA_NEAR_imm(this); break;
        case 0x88: _JS_NEAR_imm(this); break;
        case 0x89: _JNS_NEAR_imm(this); break;
        case 0x8A: _JP_NEAR_imm(this); break;
        case 0x8B: _JNP_NEAR_imm(this); break;
        case 0x8C: _JL_NEAR_imm(this); break;
        case 0x8D: _JNL_NEAR_imm(this); break;
        case 0x8E: _JNG_NEAR_imm(this); break;
        case 0x8F: _JG_NEAR_imm(this); break;
        case 0xA0: _PUSH_FS(this); break;
        case 0xA1: _POP_FS(this); break;
        case 0xA8: _PUSH_GS(this); break;
        case 0xA9: _POP_GS(this); break;
        case 0xB4: CALL_HANDLER(_LFS_reg16_mem16, _LFS_reg32_mem32); break;
        case 0xB5: CALL_HANDLER(_LFS_reg16_mem16, _LFS_reg32_mem32); break;
        case 0xB6: CALL_HANDLER(_MOVZX_reg16_RM8, _MOVZX_reg32_RM8); break;
        case 0xB7: CALL_HANDLER(_UNSUPP, _MOVZX_reg32_RM16); break;
        case 0xFF: _UD0(this); break;
        default: goto fffuuu;
        }
        break;
    case 0x10: _ADC_RM8_reg8(this); break;
    case 0x11: CALL_HANDLER(_ADC_RM16_reg16, _ADC_RM32_reg32); break;
    case 0x12: _ADC_reg8_RM8(this); break;
    case 0x13: CALL_HANDLER(_ADC_reg16_RM16, _ADC_reg32_RM32); break;
    case 0x14: _ADC_AL_imm8(this); break;
    case 0x15: CALL_HANDLER(_ADC_AX_imm16, _ADC_EAX_imm32); break;
    case 0x16: _PUSH_SS(this); break;
    case 0x17: _POP_SS(this); break;
    case 0x18: _SBB_RM8_reg8(this); break;
    case 0x19: CALL_HANDLER(_SBB_RM16_reg16, _SBB_RM32_reg32); break;
    case 0x1A: _SBB_reg8_RM8(this); break;
    case 0x1B: CALL_HANDLER(_SBB_reg16_RM16, _SBB_reg32_RM32); break;
    case 0x1C: _SBB_AL_imm8(this); break;
    case 0x1D: CALL_HANDLER(_SBB_AX_imm16, _SBB_EAX_imm32); break;
    case 0x1E: _PUSH_DS(this); break;
    case 0x1F: _POP_DS(this); break;
    case 0x20: _AND_RM8_reg8(this); break;
    case 0x21: CALL_HANDLER(_AND_RM16_reg16, _AND_RM32_reg32); break;
    case 0x22: _AND_reg8_RM8(this); break;
    case 0x23: CALL_HANDLER(_AND_reg16_RM16, _AND_reg32_RM32); break;
    case 0x24: _AND_AL_imm8(this); break;
    case 0x25: CALL_HANDLER(_AND_AX_imm16, _AND_EAX_imm32); break;
    case 0x26: _ES(this); break;
    case 0x27: _DAA(this); break;
    case 0x28: _SUB_RM8_reg8(this); break;
    case 0x29: CALL_HANDLER(_SUB_RM16_reg16, _SUB_RM32_reg32); break;
    case 0x2A: _SUB_reg8_RM8(this); break;
    case 0x2B: CALL_HANDLER(_SUB_reg16_RM16, _SUB_reg32_RM32); break;
    case 0x2C: _SUB_AL_imm8(this); break;
    case 0x2D: CALL_HANDLER(_SUB_AX_imm16, _SUB_EAX_imm32); break;
    case 0x2E: _CS(this); break;
    case 0x2F: _DAS(this); break;
    case 0x30: _XOR_RM8_reg8(this); break;
    case 0x31: CALL_HANDLER(_XOR_RM16_reg16, _XOR_RM32_reg32); break;
    case 0x32: _XOR_reg8_RM8(this); break;
    case 0x33: CALL_HANDLER(_XOR_reg16_RM16, _XOR_reg32_RM32); break;
    case 0x34: _XOR_AL_imm8(this); break;
    case 0x35: CALL_HANDLER(_XOR_AX_imm16, _XOR_EAX_imm32); break;
    case 0x36: _SS(this); break;
    case 0x37: _AAA(this); break;
    case 0x38: _CMP_RM8_reg8(this); break;
    case 0x39: CALL_HANDLER(_CMP_RM16_reg16, _CMP_RM32_reg32); break;
    case 0x3A: _CMP_reg8_RM8(this); break;
    case 0x3B: CALL_HANDLER(_CMP_reg16_RM16, _CMP_reg32_RM32); break;
    case 0x3C: _CMP_AL_imm8(this); break;
    case 0x3D: CALL_HANDLER(_CMP_AX_imm16, _CMP_EAX_imm32); break;
    case 0x3E: _DS(this); break;
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47: CALL_HANDLER(_INC_reg16, _INC_reg32); break;
    case 0x48:
    case 0x49:
    case 0x4A:
    case 0x4B:
    case 0x4C:
    case 0x4D:
    case 0x4E:
    case 0x4F: CALL_HANDLER(_DEC_reg16, _DEC_reg32); break;
    case 0x50: CALL_HANDLER(_PUSH_AX, _PUSH_EAX); break;
    case 0x51: CALL_HANDLER(_PUSH_CX, _PUSH_ECX); break;
    case 0x52: CALL_HANDLER(_PUSH_DX, _PUSH_EDX); break;
    case 0x53: CALL_HANDLER(_PUSH_BX, _PUSH_EBX); break;
    case 0x54: CALL_HANDLER(_PUSH_SP, _PUSH_ESP); break;
    case 0x55: CALL_HANDLER(_PUSH_BP, _PUSH_EBP); break;
    case 0x56: CALL_HANDLER(_PUSH_SI, _PUSH_ESI); break;
    case 0x57: CALL_HANDLER(_PUSH_DI, _PUSH_EDI); break;
    case 0x58: CALL_HANDLER(_POP_AX, _POP_EAX); break;
    case 0x59: CALL_HANDLER(_POP_CX, _POP_ECX); break;
    case 0x5A: CALL_HANDLER(_POP_DX, _POP_EDX); break;
    case 0x5B: CALL_HANDLER(_POP_BX, _POP_EBX); break;
    case 0x5C: CALL_HANDLER(_POP_SP, _POP_ESP); break;
    case 0x5D: CALL_HANDLER(_POP_BP, _POP_EBP); break;
    case 0x5E: CALL_HANDLER(_POP_SI, _POP_ESI); break;
    case 0x5F: CALL_HANDLER(_POP_DI, _POP_EDI); break;
    case 0x60: CALL_HANDLER(_PUSHA, _PUSHAD); break;
    case 0x61: CALL_HANDLER(_POPA, _POPAD); break;
    case 0x64: _FS(this); break;
    case 0x65: _GS(this); break;
    case 0x66: _OperationSizeOverride(this); break;
    case 0x67: _AddressSizeOverride(this); break;
    case 0x68: CALL_HANDLER(_PUSH_imm16, _PUSH_imm32); break;
    case 0x6A: _PUSH_imm8(this); break;
    case 0x6E: _OUTSB(this); break;
    case 0x70: _JO_imm8(this); break;
    case 0x71: _JNO_imm8(this); break;
    case 0x72: _JC_imm8(this); break;
    case 0x73: _JNC_imm8(this); break;
    case 0x74: _JZ_imm8(this); break;
    case 0x75: _JNZ_imm8(this); break;
    case 0x76: _JNA_imm8(this); break;
    case 0x77: _JA_imm8(this); break;
    case 0x78: _JS_imm8(this); break;
    case 0x79: _JNS_imm8(this); break;
    case 0x7A: _JP_imm8(this); break;
    case 0x7B: _JNP_imm8(this); break;
    case 0x7C: _JL_imm8(this); break;
    case 0x7D: _JNL_imm8(this); break;
    case 0x7E: _JNG_imm8(this); break;
    case 0x7F: _JG_imm8(this); break;
    case 0x80: _wrap_0x80(this); break;
    case 0x81: CALL_HANDLER(_wrap_0x81_16, _wrap_0x81_32); break;
    case 0x83: CALL_HANDLER(_wrap_0x83_16, _wrap_0x83_32); break;
    case 0x84: _TEST_RM8_reg8(this); break;
    case 0x85: CALL_HANDLER(_TEST_RM16_reg16, _TEST_RM32_reg32); break;
    case 0x86: _XCHG_reg8_RM8(this); break;
    case 0x87: CALL_HANDLER(_XCHG_reg16_RM16, _XCHG_reg32_RM32); break;
    case 0x88: _MOV_RM8_reg8(this); break;
    case 0x89: CALL_HANDLER(_MOV_RM16_reg16, _MOV_RM32_reg32); break;
    case 0x8A: _MOV_reg8_RM8(this); break;
    case 0x8B: CALL_HANDLER(_MOV_reg16_RM16, _MOV_reg32_RM32); break;
    case 0x8C: CALL_HANDLER(_MOV_RM16_seg, _MOV_RM32_seg); break;
    case 0x8D: CALL_HANDLER(_LEA_reg16_mem16, _LEA_reg32_mem32); break;
    case 0x8E: CALL_HANDLER(_MOV_seg_RM16, _MOV_seg_RM32); break;
    case 0x8F: CALL_HANDLER(_wrap_0x8F_16, _wrap_0x8F_32); break;
    case 0x90: /* NOP */ break;
    case 0x91:
    case 0x92:
    case 0x93:
    case 0x94:
    case 0x95:
    case 0x96:
    case 0x97: CALL_HANDLER(_XCHG_AX_reg16, _XCHG_EAX_reg32); break;
    case 0x98: CALL_HANDLER(_CBW, _CWDE); break;
    case 0x99: CALL_HANDLER(_CWD, _CDQ); break;
    case 0x9A: CALL_HANDLER(_CALL_imm16_imm16, _CALL_imm16_imm32); break;
    case 0x9C: CALL_HANDLER(_PUSHF, _PUSHFD); break;
    case 0x9D: CALL_HANDLER(_POPF, _POPFD); break;
    case 0x9E: _SAHF(this); break;
    case 0x9F: _LAHF(this); break;
    case 0xA0: _MOV_AL_moff8(this); break;
    case 0xA1: CALL_HANDLER(_MOV_AX_moff16, _MOV_EAX_moff32); break;
    case 0xA2: _MOV_moff8_AL(this); break;
    case 0xA3: CALL_HANDLER(_MOV_moff16_AX, _MOV_moff32_EAX); break;
    case 0xA4: _MOVSB(this); break;
    case 0xA5: CALL_HANDLER(_MOVSW, _MOVSD); break;
    case 0xA6: _CMPSB(this); break;
    case 0xA7: CALL_HANDLER(_CMPSW, _CMPSD); break;
    case 0xA8: _TEST_AL_imm8(this); break;
    case 0xA9: CALL_HANDLER(_TEST_AX_imm16, _TEST_EAX_imm32); break;
    case 0xAA: _STOSB(this); break;
    case 0xAB: CALL_HANDLER(_STOSW, _STOSD); break;
    case 0xAC: _LODSB(this); break;
    case 0xAD: CALL_HANDLER(_LODSW, _LODSD); break;
    case 0xAE: _SCASB(this); break;
    case 0xAF: CALL_HANDLER(_SCASW, _SCASD); break;
    case 0xB0: _MOV_AL_imm8(this); break;
    case 0xB1: _MOV_CL_imm8(this); break;
    case 0xB2: _MOV_DL_imm8(this); break;
    case 0xB3: _MOV_BL_imm8(this); break;
    case 0xB4: _MOV_AH_imm8(this); break;
    case 0xB5: _MOV_CH_imm8(this); break;
    case 0xB6: _MOV_DH_imm8(this); break;
    case 0xB7: _MOV_BH_imm8(this); break;
    case 0xB8: CALL_HANDLER(_MOV_AX_imm16, _MOV_EAX_imm32); break;
    case 0xB9: CALL_HANDLER(_MOV_CX_imm16, _MOV_ECX_imm32); break;
    case 0xBA: CALL_HANDLER(_MOV_DX_imm16, _MOV_EDX_imm32); break;
    case 0xBB: CALL_HANDLER(_MOV_BX_imm16, _MOV_EBX_imm32); break;
    case 0xBC: CALL_HANDLER(_MOV_SP_imm16, _MOV_ESP_imm32); break;
    case 0xBD: CALL_HANDLER(_MOV_BP_imm16, _MOV_EBP_imm32); break;
    case 0xBE: CALL_HANDLER(_MOV_SI_imm16, _MOV_ESI_imm32); break;
    case 0xBF: CALL_HANDLER(_MOV_DI_imm16, _MOV_EDI_imm32); break;
    case 0xC0: _wrap_0xC0(this); break;
    case 0xC1: CALL_HANDLER(_wrap_0xC1_16, _wrap_0xC1_32); break;
    case 0xC2: _RET_imm16(this); break;
    case 0xC3: _RET(this); break;
    case 0xC4: CALL_HANDLER(_LES_reg16_mem16, _LES_reg32_mem32); break;
    case 0xC5: CALL_HANDLER(_LDS_reg16_mem16, _LDS_reg32_mem32); break;
    case 0xC6: _MOV_RM8_imm8(this); break;
    case 0xC7: CALL_HANDLER(_MOV_RM16_imm16, _MOV_RM32_imm32); break;
    case 0xC8: _ENTER(this); break;
    case 0xC9: _LEAVE(this); break;
    case 0xCA: _RETF_imm16(this); break;
    case 0xCB: _RETF(this); break;
    case 0xCD: _INT_imm8(this); break;
    case 0xCF: _IRET(this); break;
    case 0xD0: _wrap_0xD0(this); break;
    case 0xD1: CALL_HANDLER(_wrap_0xD1_16, _wrap_0xD1_32); break;
    case 0xD2: _wrap_0xD2(this); break;
    case 0xD3: CALL_HANDLER(_wrap_0xD3_16, _wrap_0xD3_32); break;
    case 0xD5: _AAD(this); break;
    case 0xD7: _XLAT(this); break;
    // BEGIN FPU STUBS
    case 0xD8:
    case 0xD9:
    case 0xDA:
    case 0xDB:
    case 0xDC:
    case 0xDD:
    case 0xDE:
    case 0xDF: _ESCAPE(this); break;
    // END FPU STUBS
    case 0xE0: _LOOPNE_imm8(this); break;
    case 0xE1: _LOOPE_imm8(this); break;
    case 0xE2: _LOOP_imm8(this); break;
    case 0xE3: CALL_HANDLER(_JCXZ_imm8, _JECXZ_imm8); break;
    case 0xE4: _IN_AL_imm8(this); break;
    case 0xE6: _OUT_imm8_AL(this); break;
    case 0xE7: CALL_HANDLER(_OUT_imm8_AX, _OUT_imm8_EAX); break;
    case 0xE8: CALL_HANDLER(_CALL_imm16, _CALL_imm32); break;
    case 0xE9: CALL_HANDLER(_JMP_imm16, _JMP_imm32); break;
    case 0xEA: CALL_HANDLER(_JMP_imm16_imm16, _JMP_imm16_imm32); break;
    case 0xEB: _JMP_short_imm8(this); break;
    case 0xEC: _IN_AL_DX(this); break;
    case 0xED: CALL_HANDLER(_IN_AX_DX, _IN_EAX_DX); break;
    case 0xEE: _OUT_DX_AL(this); break;
    case 0xEF: CALL_HANDLER(_OUT_DX_AX, _OUT_DX_EAX); break;
    case 0xF0: /* LOCK */ break;
    case 0xF2: _REPNE(this); break;
    case 0xF3: _REP(this); break;
    case 0xF4: _HLT(this); break;
    case 0xF5: _CMC(this); break;
    case 0xF6: _wrap_0xF6(this); break;
    case 0xF7: CALL_HANDLER(_wrap_0xF7_16, _wrap_0xF7_32); break;
    case 0xF8: _CLC(this); break;
    case 0xF9: _STC(this); break;
    case 0xFA: _CLI(this); break;
    case 0xFB: _STI(this); break;
    case 0xFC: _CLD(this); break;
    case 0xFD: _STD(this); break;
    case 0xFE: _wrap_0xFE(this); break;
    case 0xFF: CALL_HANDLER(_wrap_0xFF_16, _wrap_0xFF_32); break;
    default:
        this->rmbyte = fetchOpcodeByte();
fffuuu:
        vlog(VM_ALERT, "%04X:%08X FFFFUUUU unsupported opcode %02X /%u or %02X %02X or %02X %02X /%u",
            getCS(), getEIP(),
            this->opcode, vomit_modRMRegisterPart(this->rmbyte),
            this->opcode, this->rmbyte,
            this->opcode, this->rmbyte, vomit_modRMRegisterPart(this->subrmbyte)
        );
        exception(6);
    }
}

void VCpu::GP(int code)
{
    vlog(VM_CPUMSG, "#GP(%d) :-(", code);
    vm_exit(1);
}

VCpu::VCpu(QObject* parent)
    : QObject(parent)
{
    this->memory = new BYTE[(8192 * 1024) + 65536];
    if (!this->memory) {
        vlog(VM_INITMSG, "Insufficient memory available.");
        vm_exit(1);
    }

#ifdef VOMIT_DETECT_UNINITIALIZED_ACCESS
    m_dirtMap = new bool[1048576 + 65536];
    if (!m_dirtMap) {
        vlog(VM_INITMSG, "Insufficient memory available.");
        vm_exit(1);
    }
    memset(m_dirtMap, 0, 1048576 + 65536);
#endif

    memset(this->memory, 0, 1048576 + 65536);

    m_vgaMemory = new VgaMemory(this);

#ifdef VOMIT_PREFETCH_QUEUE
    if (m_prefetchQueue)
        delete [] m_prefetchQueue;

    m_prefetchQueueSize = 4;
    m_prefetchQueue = new BYTE[m_prefetchQueueSize];
    m_prefetchQueueIndex = 0;
#endif

    m_a20Enabled = false;

    memset(&regs, 0, sizeof(regs));
    this->CS = 0;
    this->DS = 0;
    this->ES = 0;
    this->SS = 0;
    this->FS = 0;
    this->GS = 0;
    this->CR0 = 0;
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
    this->LDTR.index = 0;

    memset(m_selector, 0, sizeof(m_selector));

    this->creg[0] = &this->CR0;
    this->creg[1] = &this->CR1;
    this->creg[2] = &this->CR2;
    this->creg[3] = &this->CR3;
    this->creg[4] = &this->CR4;
    this->creg[5] = &this->CR5;
    this->creg[6] = &this->CR6;
    this->creg[7] = &this->CR7;

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

    this->tseg[RegisterCS] = &this->CS;
    this->tseg[RegisterDS] = &this->DS;
    this->tseg[RegisterES] = &this->ES;
    this->tseg[RegisterSS] = &this->SS;
    this->tseg[RegisterFS] = &this->FS;
    this->tseg[RegisterGS] = &this->GS;
    this->tseg[6] = 0;
    this->tseg[7] = 0;

    m_segmentPrefix = 0x0000;
    m_currentSegment = &this->DS;

    jump32(0xF000, 0x00000000);

    setFlags(0x0200);

    setIOPL(3);

    m_instructionsPerTick = 0x50000;
    m_pitCountdown = m_instructionsPerTick;
    m_state = Alive;

    m_addressSize32 = false;
    m_operationSize32 = false;

#ifdef VOMIT_DEBUG
    m_inDebugger = false;
#endif
}

VCpu::~VCpu()
{
#ifdef VOMIT_PREFETCH_QUEUE
    delete [] m_prefetchQueue;
    m_prefetchQueue = 0;
#endif

#ifdef VOMIT_DETECT_UNINITIALIZED_ACCESS
    delete [] m_dirtMap;
#endif

    delete [] this->memory;
    this->memory = 0;
    m_codeMemory = 0;
}

#ifdef FOR_REFERENCE_ONLY
void VCpu::registerDefaultOpcodeHandlers()
{
    // Install a default handler for unsupported instructions
    setOpcodeHandler(0x00, 0xFF, _UNSUPP           );

    // Stubs
    setOpcodeHandler(0x66, 0x66, _OperationSizeOverride);

    // FPU stubs
    setOpcodeHandler(0x9B, 0x9B, _ESCAPE           );
    setOpcodeHandler(0xD8, 0xDF, _ESCAPE           );

    // 8086+ instructions
    setOpcodeHandler(0x00, 0x00, _ADD_RM8_reg8     );
    setOpcodeHandler(0x01, 0x01, _ADD_RM16_reg16   );
    setOpcodeHandler(0x02, 0x02, _ADD_reg8_RM8     );
    setOpcodeHandler(0x03, 0x03, _ADD_reg16_RM16   );
    setOpcodeHandler(0x04, 0x04, _ADD_AL_imm8      );
    setOpcodeHandler(0x05, 0x05, _ADD_AX_imm16     );
    setOpcodeHandler(0x06, 0x06, _PUSH_ES          );
    setOpcodeHandler(0x07, 0x07, _POP_ES           );
    setOpcodeHandler(0x08, 0x08, _OR_RM8_reg8      );
    setOpcodeHandler(0x09, 0x09, _OR_RM16_reg16    );
    setOpcodeHandler(0x0A, 0x0A, _OR_reg8_RM8      );
    setOpcodeHandler(0x0B, 0x0B, _OR_reg16_RM16    );
    setOpcodeHandler(0x0C, 0x0C, _OR_AL_imm8       );
    setOpcodeHandler(0x0D, 0x0D, _OR_AX_imm16      );
    setOpcodeHandler(0x0E, 0x0E, _PUSH_CS          );
    setOpcodeHandler(0x0F, 0x0F, _POP_CS           );
    setOpcodeHandler(0x10, 0x10, _ADC_RM8_reg8     );
    setOpcodeHandler(0x11, 0x11, _ADC_RM16_reg16   );
    setOpcodeHandler(0x12, 0x12, _ADC_reg8_RM8     );
    setOpcodeHandler(0x13, 0x13, _ADC_reg16_RM16   );
    setOpcodeHandler(0x14, 0x14, _ADC_AL_imm8      );
    setOpcodeHandler(0x15, 0x15, _ADC_AX_imm16     );
    setOpcodeHandler(0x16, 0x16, _PUSH_SS          );
    setOpcodeHandler(0x17, 0x17, _POP_SS           );
    setOpcodeHandler(0x18, 0x18, _SBB_RM8_reg8     );
    setOpcodeHandler(0x19, 0x19, _SBB_RM16_reg16   );
    setOpcodeHandler(0x1A, 0x1A, _SBB_reg8_RM8     );
    setOpcodeHandler(0x1B, 0x1B, _SBB_reg16_RM16   );
    setOpcodeHandler(0x1C, 0x1C, _SBB_AL_imm8      );
    setOpcodeHandler(0x1D, 0x1D, _SBB_AX_imm16     );
    setOpcodeHandler(0x1E, 0x1E, _PUSH_DS          );
    setOpcodeHandler(0x1F, 0x1F, _POP_DS           );
    setOpcodeHandler(0x20, 0x20, _AND_RM8_reg8     );
    setOpcodeHandler(0x21, 0x21, _AND_RM16_reg16   );
    setOpcodeHandler(0x22, 0x22, _AND_reg8_RM8     );
    setOpcodeHandler(0x23, 0x23, _AND_reg16_RM16   );
    setOpcodeHandler(0x24, 0x24, _AND_AL_imm8      );
    setOpcodeHandler(0x25, 0x25, _AND_AX_imm16     );
    setOpcodeHandler(0x26, 0x26, _ES               );
    setOpcodeHandler(0x27, 0x27, _DAA              );
    setOpcodeHandler(0x28, 0x28, _SUB_RM8_reg8     );
    setOpcodeHandler(0x29, 0x29, _SUB_RM16_reg16   );
    setOpcodeHandler(0x2A, 0x2A, _SUB_reg8_RM8     );
    setOpcodeHandler(0x2B, 0x2B, _SUB_reg16_RM16   );
    setOpcodeHandler(0x2C, 0x2C, _SUB_AL_imm8      );
    setOpcodeHandler(0x2D, 0x2D, _SUB_AX_imm16     );
    setOpcodeHandler(0x2E, 0x2E, _CS               );
    setOpcodeHandler(0x2F, 0x2F, _DAS              );
    setOpcodeHandler(0x30, 0x30, _XOR_RM8_reg8     );
    setOpcodeHandler(0x31, 0x31, _XOR_RM16_reg16   );
    setOpcodeHandler(0x32, 0x32, _XOR_reg8_RM8     );
    setOpcodeHandler(0x33, 0x33, _XOR_reg16_RM16   );
    setOpcodeHandler(0x34, 0x34, _XOR_AL_imm8      );
    setOpcodeHandler(0x35, 0x35, _XOR_AX_imm16     );
    setOpcodeHandler(0x36, 0x36, _SS               );
    setOpcodeHandler(0x37, 0x37, _AAA              );
    setOpcodeHandler(0x38, 0x38, _CMP_RM8_reg8     );
    setOpcodeHandler(0x39, 0x39, _CMP_RM16_reg16   );
    setOpcodeHandler(0x3A, 0x3A, _CMP_reg8_RM8     );
    setOpcodeHandler(0x3B, 0x3B, _CMP_reg16_RM16   );
    setOpcodeHandler(0x3C, 0x3C, _CMP_AL_imm8      );
    setOpcodeHandler(0x3D, 0x3D, _CMP_AX_imm16     );
    setOpcodeHandler(0x3E, 0x3E, _DS               );
    setOpcodeHandler(0x3F, 0x3F, _AAS              );
    setOpcodeHandler(0x40, 0x47, _INC_reg16        );
    setOpcodeHandler(0x48, 0x4F, _DEC_reg16        );

    setOpcodeHandler(0x50, 0x50, _PUSH_AX          );
    setOpcodeHandler(0x51, 0x51, _PUSH_CX          );
    setOpcodeHandler(0x52, 0x52, _PUSH_DX          );
    setOpcodeHandler(0x53, 0x53, _PUSH_BX          );
    setOpcodeHandler(0x54, 0x54, _PUSH_SP          );
    setOpcodeHandler(0x55, 0x55, _PUSH_BP          );
    setOpcodeHandler(0x56, 0x56, _PUSH_SI          );
    setOpcodeHandler(0x57, 0x57, _PUSH_DI          );

    setOpcodeHandler(0x58, 0x58, _POP_AX           );
    setOpcodeHandler(0x59, 0x59, _POP_CX           );
    setOpcodeHandler(0x5A, 0x5A, _POP_DX           );
    setOpcodeHandler(0x5B, 0x5B, _POP_BX           );
    setOpcodeHandler(0x5C, 0x5C, _POP_SP           );
    setOpcodeHandler(0x5D, 0x5D, _POP_BP           );
    setOpcodeHandler(0x5E, 0x5E, _POP_SI           );
    setOpcodeHandler(0x5F, 0x5F, _POP_DI           );

    setOpcodeHandler(0x64, 0x64, _FS               );
    setOpcodeHandler(0x65, 0x65, _GS               );

    setOpcodeHandler(0x6E, 0x6E, _OUTSB            );
    setOpcodeHandler(0x6F, 0x6F, _OUTSW            );
    setOpcodeHandler(0x70, 0x70, _JO_imm8          );
    setOpcodeHandler(0x71, 0x71, _JNO_imm8         );
    setOpcodeHandler(0x72, 0x72, _JC_imm8          );
    setOpcodeHandler(0x73, 0x73, _JNC_imm8         );
    setOpcodeHandler(0x74, 0x74, _JZ_imm8          );
    setOpcodeHandler(0x75, 0x75, _JNZ_imm8         );
    setOpcodeHandler(0x76, 0x76, _JNA_imm8         );
    setOpcodeHandler(0x77, 0x77, _JA_imm8          );
    setOpcodeHandler(0x78, 0x78, _JS_imm8          );
    setOpcodeHandler(0x79, 0x79, _JNS_imm8         );
    setOpcodeHandler(0x7A, 0x7A, _JP_imm8          );
    setOpcodeHandler(0x7B, 0x7B, _JNP_imm8         );
    setOpcodeHandler(0x7C, 0x7C, _JL_imm8          );
    setOpcodeHandler(0x7D, 0x7D, _JNL_imm8         );
    setOpcodeHandler(0x7E, 0x7E, _JNG_imm8         );
    setOpcodeHandler(0x7F, 0x7F, _JG_imm8          );
    setOpcodeHandler(0x80, 0x80, _wrap_0x80        );
    setOpcodeHandler(0x81, 0x81, _wrap_0x81_16     );
    setOpcodeHandler(0x83, 0x83, _wrap_0x83_16     );
    setOpcodeHandler(0x84, 0x84, _TEST_RM8_reg8    );
    setOpcodeHandler(0x85, 0x85, _TEST_RM16_reg16  );
    setOpcodeHandler(0x86, 0x86, _XCHG_reg8_RM8    );
    setOpcodeHandler(0x87, 0x87, _XCHG_reg16_RM16  );
    setOpcodeHandler(0x88, 0x88, _MOV_RM8_reg8     );
    setOpcodeHandler(0x89, 0x89, _MOV_RM16_reg16   );
    setOpcodeHandler(0x8A, 0x8A, _MOV_reg8_RM8     );
    setOpcodeHandler(0x8B, 0x8B, _MOV_reg16_RM16   );
    setOpcodeHandler(0x8C, 0x8C, _MOV_RM16_seg     );
    setOpcodeHandler(0x8D, 0x8D, _LEA_reg16_mem16  );
    setOpcodeHandler(0x8E, 0x8E, _MOV_seg_RM16     );
    setOpcodeHandler(0x8F, 0x8F, _wrap_0x8F_16     );
    setOpcodeHandler(0x90, 0x90, _NOP              );
    setOpcodeHandler(0x91, 0x97, _XCHG_AX_reg16    );
    setOpcodeHandler(0x98, 0x98, _CBW              );
    setOpcodeHandler(0x99, 0x99, _CWD              );
    setOpcodeHandler(0x9A, 0x9A, _CALL_imm16_imm16 );
    setOpcodeHandler(0x9C, 0x9C, _PUSHF            );
    setOpcodeHandler(0x9D, 0x9D, _POPF             );
    setOpcodeHandler(0x9E, 0x9E, _SAHF             );
    setOpcodeHandler(0x9F, 0x9F, _LAHF             );
    setOpcodeHandler(0xA0, 0xA0, _MOV_AL_moff8     );
    setOpcodeHandler(0xA1, 0xA1, _MOV_AX_moff16    );
    setOpcodeHandler(0xA2, 0xA2, _MOV_moff8_AL     );
    setOpcodeHandler(0xA3, 0xA3, _MOV_moff16_AX    );
    setOpcodeHandler(0xA4, 0xA4, _MOVSB            );
    setOpcodeHandler(0xA5, 0xA5, _MOVSW            );
    setOpcodeHandler(0xA6, 0xA6, _CMPSB            );
    setOpcodeHandler(0xA7, 0xA7, _CMPSW            );
    setOpcodeHandler(0xA8, 0xA8, _TEST_AL_imm8     );
    setOpcodeHandler(0xA9, 0xA9, _TEST_AX_imm16    );
    setOpcodeHandler(0xAA, 0xAA, _STOSB            );
    setOpcodeHandler(0xAB, 0xAB, _STOSW            );
    setOpcodeHandler(0xAC, 0xAC, _LODSB            );
    setOpcodeHandler(0xAD, 0xAD, _LODSW            );
    setOpcodeHandler(0xAE, 0xAE, _SCASB            );
    setOpcodeHandler(0xAF, 0xAF, _SCASW            );

    setOpcodeHandler(0xB0, 0xB0, _MOV_AL_imm8      );
    setOpcodeHandler(0xB1, 0xB1, _MOV_CL_imm8      );
    setOpcodeHandler(0xB2, 0xB2, _MOV_DL_imm8      );
    setOpcodeHandler(0xB3, 0xB3, _MOV_BL_imm8      );
    setOpcodeHandler(0xB4, 0xB4, _MOV_AH_imm8      );
    setOpcodeHandler(0xB5, 0xB5, _MOV_CH_imm8      );
    setOpcodeHandler(0xB6, 0xB6, _MOV_DH_imm8      );
    setOpcodeHandler(0xB7, 0xB7, _MOV_BH_imm8      );

    setOpcodeHandler(0xB8, 0xB8, _MOV_AX_imm16     );
    setOpcodeHandler(0xB9, 0xB9, _MOV_CX_imm16     );
    setOpcodeHandler(0xBA, 0xBA, _MOV_DX_imm16     );
    setOpcodeHandler(0xBB, 0xBB, _MOV_BX_imm16     );
    setOpcodeHandler(0xBC, 0xBC, _MOV_SP_imm16     );
    setOpcodeHandler(0xBD, 0xBD, _MOV_BP_imm16     );
    setOpcodeHandler(0xBE, 0xBE, _MOV_SI_imm16     );
    setOpcodeHandler(0xBF, 0xBF, _MOV_DI_imm16     );

    setOpcodeHandler(0xC2, 0xC2, _RET_imm16        );
    setOpcodeHandler(0xC3, 0xC3, _RET              );
    setOpcodeHandler(0xC4, 0xC4, _LES_reg16_mem16  );
    setOpcodeHandler(0xC5, 0xC5, _LDS_reg16_mem16  );
    setOpcodeHandler(0xC6, 0xC6, _MOV_RM8_imm8     );
    setOpcodeHandler(0xC7, 0xC7, _MOV_RM16_imm16   );
    setOpcodeHandler(0xCA, 0xCA, _RETF_imm16       );
    setOpcodeHandler(0xCB, 0xCB, _RETF             );
    setOpcodeHandler(0xCC, 0xCC, _INT3             );
    setOpcodeHandler(0xCD, 0xCD, _INT_imm8         );
    setOpcodeHandler(0xCE, 0xCE, _INTO             );
    setOpcodeHandler(0xCF, 0xCF, _IRET             );
    setOpcodeHandler(0xD0, 0xD0, _wrap_0xD0        );
    setOpcodeHandler(0xD1, 0xD1, _wrap_0xD1_16     );
    setOpcodeHandler(0xD2, 0xD2, _wrap_0xD2        );
    setOpcodeHandler(0xD3, 0xD3, _wrap_0xD3_16     );
    setOpcodeHandler(0xD4, 0xD4, _AAM              );
    setOpcodeHandler(0xD5, 0xD5, _AAD              );
    setOpcodeHandler(0xD6, 0xD6, _SALC             );
    setOpcodeHandler(0xD7, 0xD7, _XLAT             );
    setOpcodeHandler(0xE0, 0xE0, _LOOPNE_imm8      );
    setOpcodeHandler(0xE1, 0xE1, _LOOPE_imm8       );
    setOpcodeHandler(0xE2, 0xE2, _LOOP_imm8        );
    setOpcodeHandler(0xE3, 0xE3, _JCXZ_imm8        );
    setOpcodeHandler(0xE4, 0xE4, _IN_AL_imm8       );
    setOpcodeHandler(0xE5, 0xE5, _IN_AX_imm8       );
    setOpcodeHandler(0xE6, 0xE6, _OUT_imm8_AL      );
    setOpcodeHandler(0xE7, 0xE7, _OUT_imm8_AX      );
    setOpcodeHandler(0xE8, 0xE8, _CALL_imm16       );
    setOpcodeHandler(0xE9, 0xE9, _JMP_imm16        );
    setOpcodeHandler(0xEA, 0xEA, _JMP_imm16_imm16  );
    setOpcodeHandler(0xEB, 0xEB, _JMP_short_imm8   );
    setOpcodeHandler(0xEC, 0xEC, _IN_AL_DX         );
    setOpcodeHandler(0xED, 0xED, _IN_AX_DX         );
    setOpcodeHandler(0xEE, 0xEE, _OUT_DX_AL        );
    setOpcodeHandler(0xEF, 0xEF, _OUT_DX_AX        );
    setOpcodeHandler(0xF2, 0xF2, _REPNE            );
    setOpcodeHandler(0xF3, 0xF3, _REP              );
    setOpcodeHandler(0xF4, 0xF4, _HLT              );
    setOpcodeHandler(0xF5, 0xF5, _CMC              );
    setOpcodeHandler(0xF6, 0xF6, _wrap_0xF6        );
    setOpcodeHandler(0xF7, 0xF7, _wrap_0xF7_16     );
    setOpcodeHandler(0xF8, 0xF8, _CLC              );
    setOpcodeHandler(0xF9, 0xF9, _STC              );
    setOpcodeHandler(0xFA, 0xFA, _CLI              );
    setOpcodeHandler(0xFB, 0xFB, _STI              );
    setOpcodeHandler(0xFC, 0xFC, _CLD              );
    setOpcodeHandler(0xFD, 0xFD, _STD              );
    setOpcodeHandler(0xFE, 0xFE, _wrap_0xFE        );
    setOpcodeHandler(0xFF, 0xFF, _wrap_0xFF_16     );

    setOpcodeHandler(0xC0, 0xC0, _wrap_0xC0        );
    setOpcodeHandler(0xC1, 0xC1, _wrap_0xC1_16     );

    // 80186+ instructions
    setOpcodeHandler(0x0F, 0x0F, _wrap_0x0F        );
    setOpcodeHandler(0x60, 0x60, _PUSHA            );
    setOpcodeHandler(0x61, 0x61, _POPA             );
    setOpcodeHandler(0x62, 0x62, _BOUND            );
    setOpcodeHandler(0x68, 0x68, _PUSH_imm16       );
    setOpcodeHandler(0x6A, 0x6A, _PUSH_imm8        );
    setOpcodeHandler(0x6B, 0x6B, _IMUL_reg16_RM16_imm8 );
    setOpcodeHandler(0xC0, 0xC0, _wrap_0xC0        );
    setOpcodeHandler(0xC1, 0xC1, _wrap_0xC1_16     );
    setOpcodeHandler(0xC8, 0xC8, _ENTER            );
    setOpcodeHandler(0xC9, 0xC9, _LEAVE            );
}
#endif

void VCpu::exec()
{
    saveBaseAddress();
    decodeNext();
}

void VCpu::haltedLoop()
{
    while (state() == VCpu::Halted) {
        usleep(500);
        if (PIC::hasPendingIRQ())
            PIC::serviceIRQ(this);
    }
}

void VCpu::mainLoop()
{
    flushFetchQueue();

    forever {

        // HACK: This can be set by an external force to make us break out.
        if (g_vomit_exit_main_loop)
            return;

#ifdef VOMIT_DEBUG

        DWORD flatPC = vomit_toFlatAddress(getCS(), getEIP());
        foreach (DWORD breakPoint, m_breakPoints) {
            if (flatPC == breakPoint) {
                attachDebugger();
                break;
            }
        }

        if (inDebugger()) {
            saveBaseAddress();
            debugger();
            if (!inDebugger())
                continue;
        }
#endif

#ifdef VOMIT_TRACE
        if (options.trace) {
            //dumpDisassembled(getCS(), getEIP());
            dumpTrace();
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

        // HACK: Countdown towards fake PIT interrupt.
        if (tick()) {
            // Raise the timer IRQ. This is ugly, I know.
            PIC::raiseIRQ(0);
        }

        if (PIC::hasPendingIRQ() && getIF())
            PIC::serviceIRQ(this);
    }
}

#ifdef VOMIT_PREFETCH_QUEUE
//* flushFetchQueue() is inline !VOMIT_PREFETCH_QUEUE

BYTE VCpu::fetchOpcodeByte()
{
    BYTE b = m_prefetchQueue[m_prefetchQueueIndex];
    m_prefetchQueue[m_prefetchQueueIndex] = m_codeMemory[getIP() + m_prefetchQueueSize];
    if (++m_prefetchQueueIndex == m_prefetchQueueSize)
        m_prefetchQueueIndex = 0;
    ++this->IP;
    return b;
}

WORD VCpu::fetchOpcodeWord()
{
    WORD w = m_prefetchQueue[m_prefetchQueueIndex];
    m_prefetchQueue[m_prefetchQueueIndex] = this->m_codeMemory[getIP() + m_prefetchQueueSize];
    if (++m_prefetchQueueIndex == m_prefetchQueueSize)
        m_prefetchQueueIndex = 0;
    w |= m_prefetchQueue[m_prefetchQueueIndex] << 8;
    ++this->IP;
    m_prefetchQueue[m_prefetchQueueIndex] = this->m_codeMemory[getIP() + m_prefetchQueueSize];
    if (++m_prefetchQueueIndex == m_prefetchQueueSize)
        m_prefetchQueueIndex = 0;
    ++this->IP;
    return w;
}

void VCpu::flushFetchQueue()
{
    VM_ASSERT(this);
    VM_ASSERT(m_prefetchQueue);
    VM_ASSERT(m_codeMemory);

    // Flush the prefetch queue. MUST be done after all jumps/calls.
    m_prefetchQueueIndex = 0;
    for (int i = 0; i < m_prefetchQueueSize; ++i)
        m_prefetchQueue[i] = m_codeMemory[getIP() + i];
}
#endif

void VCpu::jumpRelative8(SIGNED_BYTE displacement)
{
    this->IP += displacement;
    flushFetchQueue();
}

void VCpu::jumpRelative16(SIGNED_WORD displacement)
{
    this->IP += displacement;
    flushFetchQueue();
}

void VCpu::jumpRelative32(SIGNED_DWORD displacement)
{
    this->EIP += displacement;
    flushFetchQueue();
}

void VCpu::jumpAbsolute16(WORD address)
{
    this->IP = address;
    flushFetchQueue();
}

void VCpu::jump32(WORD segment, DWORD offset)
{
    // Jump to specified location.
    this->CS = segment;
    this->EIP = offset;

    // Point m_codeMemory to CS:0 for fast opcode fetching.
    m_codeMemory = this->memory + (segment << 4);
    flushFetchQueue();
}

void VCpu::jump16(WORD segment, WORD offset)
{
    jump32(segment, offset);
}

void VCpu::setInterruptHandler(BYTE isr, WORD segment, WORD offset)
{
    writeMemory16(0x0000, (isr * 4), offset);
    writeMemory16(0x0000, (isr * 4) + 2, segment);
}

void _UNSUPP(VCpu* cpu)
{
    // We've come across an unsupported instruction, log it, then vector to the "illegal instruction" ISR.
    vlog(VM_ALERT, "%04X:%08X: Unsupported opcode %02X", cpu->getBaseCS(), cpu->getBaseEIP(), cpu->opcode);
    QString ndis = "db ";
    DWORD baseEIP = cpu->getBaseEIP();
    QStringList dbs;
    for (int i = 0; i < 16; ++i) {
        QString s;
        s.sprintf("0x%02X", cpu->codeMemory()[baseEIP + i]);
        dbs.append(s);
    }
    ndis.append(dbs.join(", "));
    vlog(VM_ALERT, qPrintable(ndis));
    cpu->dumpAll();
    cpu->exception(6);
}

void _NOP(VCpu*)
{
}

void _HLT(VCpu* cpu)
{
    cpu->setState(VCpu::Halted);

    if (!cpu->getIF()) {
        vlog(VM_ALERT, "%04X:%08X: Halted with IF=0", cpu->getBaseCS(), cpu->getBaseEIP());
        vm_exit(0);
    }

    vlog(VM_CPUMSG, "%04X:%08X Halted", cpu->getBaseCS(), cpu->getBaseEIP());

    cpu->haltedLoop();
}

void _XLAT(VCpu* cpu)
{
    cpu->regs.B.AL = cpu->readMemory8(cpu->currentSegment(), cpu->regs.W.BX + cpu->regs.B.AL);
}

void _CS(VCpu* cpu)
{
    cpu->setSegmentPrefix(cpu->getCS());
    cpu->decode(cpu->fetchOpcodeByte());
    cpu->resetSegmentPrefix();
}

void _DS(VCpu* cpu)
{
    cpu->setSegmentPrefix(cpu->getDS());
    cpu->decode(cpu->fetchOpcodeByte());
    cpu->resetSegmentPrefix();
}

void _ES(VCpu* cpu)
{
    cpu->setSegmentPrefix(cpu->getES());
    cpu->decode(cpu->fetchOpcodeByte());
    cpu->resetSegmentPrefix();
}

void _SS(VCpu* cpu)
{
    cpu->setSegmentPrefix(cpu->getSS());
    cpu->decode(cpu->fetchOpcodeByte());
    cpu->resetSegmentPrefix();
}

void _FS(VCpu* cpu)
{
    cpu->setSegmentPrefix(cpu->getFS());
    cpu->decode(cpu->fetchOpcodeByte());
    cpu->resetSegmentPrefix();
}

void _GS(VCpu* cpu)
{
    cpu->setSegmentPrefix(cpu->getGS());
    cpu->decode(cpu->fetchOpcodeByte());
    cpu->resetSegmentPrefix();
}

void _XCHG_AX_reg16(VCpu* cpu)
{
    qSwap(*cpu->treg16[cpu->opcode & 7], cpu->regs.W.AX);
}

void _XCHG_EAX_reg32(VCpu* cpu)
{
    qSwap(*cpu->treg32[cpu->opcode & 7], cpu->regs.D.EAX);
}

void _XCHG_reg8_RM8(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    BYTE &reg(*cpu->treg8[vomit_modRMRegisterPart(rm)]);

    BYTE value = cpu->readModRM8(rm);
    BYTE tmp = reg;
    reg = value;
    cpu->updateModRM8(tmp);
}

void _XCHG_reg16_RM16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    WORD value = cpu->readModRM16(rm);
    WORD tmp = *cpu->treg16[vomit_modRMRegisterPart(rm)];
    *cpu->treg16[vomit_modRMRegisterPart(rm)] = value;
    cpu->updateModRM16(tmp);
}

void _XCHG_reg32_RM32(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    DWORD value = cpu->readModRM32(rm);
    DWORD tmp = *cpu->treg32[vomit_modRMRegisterPart(rm)];
    *cpu->treg32[vomit_modRMRegisterPart(rm)] = value;
    cpu->updateModRM32(tmp);
}

void _DEC_reg16(VCpu* cpu)
{
    WORD &reg(*cpu->treg16[cpu->opcode & 7]);
    DWORD i = reg;

    /* Overflow if we'll wrap. */
    cpu->setOF(reg == 0x8000);

    --i;
    cpu->adjustFlag32(i, reg, 1);
    cpu->updateFlags16(i);
    --reg;
}

void _DEC_reg32(VCpu* cpu)
{
    DWORD &reg(*cpu->treg32[cpu->opcode & 7]);
    QWORD i = reg;

    /* Overflow if we'll wrap. */
    cpu->setOF(reg == 0x80000000);

    --i;
    cpu->adjustFlag32(i, reg, 1);
    cpu->updateFlags32(i);
    --reg;
}

void _INC_reg16(VCpu* cpu)
{
    WORD &reg(*cpu->treg16[cpu->opcode & 7]);
    DWORD i = reg;

    /* Overflow if we'll wrap. */
    cpu->setOF(i == 0x7FFF);

    ++i;
    cpu->adjustFlag32(i, reg, 1);
    cpu->updateFlags16(i);
    ++reg;
}

void _INC_reg32(VCpu* cpu)
{
    DWORD &reg(*cpu->treg32[cpu->opcode & 7]);
    QWORD i = reg;

    /* Overflow if we'll wrap. */
    cpu->setOF(i == 0x7FFFFFFF);

    ++i;
    cpu->adjustFlag32(i, reg, 1);
    cpu->updateFlags32(i);
    ++reg;
}

void _INC_RM16(VCpu* cpu)
{
    WORD value = cpu->readModRM16(cpu->rmbyte);
    DWORD i = value;

    /* Overflow if we'll wrap. */
    cpu->setOF(value == 0x7FFF);

    ++i;
    cpu->adjustFlag32(i, value, 1);
    cpu->updateFlags16(i);
    cpu->updateModRM16(value + 1);
    cpu->updateModRM16(value + 1);
}

void _DEC_RM16(VCpu* cpu)
{
    WORD value = cpu->readModRM16(cpu->rmbyte);
    DWORD i = value;

    /* Overflow if we'll wrap. */
    cpu->setOF(value == 0x8000);

    --i;
    cpu->adjustFlag32(i, value, 1); // XXX: i can be (dword)(-1)...
    cpu->updateFlags16(i);
    cpu->updateModRM16(value - 1);
}

void _LDS_reg16_mem16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    WORD* ptr = static_cast<WORD*>(cpu->resolveModRM8(rm));
    *cpu->treg16[vomit_modRMRegisterPart(rm)] = vomit_read16FromPointer(ptr);
    cpu->DS = vomit_read16FromPointer(ptr + 1);}

void _LDS_reg32_mem32(VCpu* cpu)
{
#warning FIXME: need readModRM48
    vlog(VM_ALERT, "LDS reg32 mem32");
    vm_exit(0);
}

void _LES_reg16_mem16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    WORD* ptr = static_cast<WORD*>(cpu->resolveModRM8(rm));
    *cpu->treg16[vomit_modRMRegisterPart(rm)] = vomit_read16FromPointer(ptr);
    cpu->ES = vomit_read16FromPointer(ptr + 1);
}

void _LES_reg32_mem32(VCpu* cpu)
{
#warning FIXME: need readModRM48
    vlog(VM_ALERT, "LES reg32 mem32");
    vm_exit(0);
}

void _LFS_reg16_mem16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    WORD* ptr = static_cast<WORD*>(cpu->resolveModRM8(rm));
    *cpu->treg16[vomit_modRMRegisterPart(rm)] = vomit_read16FromPointer(ptr);
    cpu->FS = vomit_read16FromPointer(ptr + 1);
}

void _LFS_reg32_mem32(VCpu* cpu)
{
#warning FIXME: need readModRM48
    vlog(VM_ALERT, "LFS reg32 mem32");
    vm_exit(0);
}

void _LGS_reg16_mem16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    WORD* ptr = static_cast<WORD*>(cpu->resolveModRM8(rm));
    *cpu->treg16[vomit_modRMRegisterPart(rm)] = vomit_read16FromPointer(ptr);
    cpu->GS = vomit_read16FromPointer(ptr + 1);
}

void _LGS_reg32_mem32(VCpu* cpu)
{
#warning FIXME: need readModRM48
    vlog(VM_ALERT, "LGS reg32 mem32");
    vm_exit(0);
}

void _LEA_reg32_mem32(VCpu* cpu)
{
#warning FIXME: need evaluateSIB()
    vlog(VM_ALERT, "LEA reg32 mem32");
    vm_exit(0);
}

void _LEA_reg16_mem16(VCpu* cpu)
{
    WORD retv = 0x0000;
    BYTE b = cpu->fetchOpcodeByte();
    switch (b & 0xC0) {
        case 0:
            switch(b & 0x07)
            {
                case 0: retv = cpu->regs.W.BX+cpu->regs.W.SI; break;
                case 1: retv = cpu->regs.W.BX+cpu->regs.W.DI; break;
                case 2: retv = cpu->regs.W.BP+cpu->regs.W.SI; break;
                case 3: retv = cpu->regs.W.BP+cpu->regs.W.DI; break;
                case 4: retv = cpu->regs.W.SI; break;
                case 5: retv = cpu->regs.W.DI; break;
                case 6: retv = cpu->fetchOpcodeWord(); break;
                default: retv = cpu->regs.W.BX; break;
            }
            break;
        case 64:
            switch(b & 0x07)
            {
                case 0: retv = cpu->regs.W.BX+cpu->regs.W.SI + vomit_signExtend(cpu->fetchOpcodeByte()); break;
                case 1: retv = cpu->regs.W.BX+cpu->regs.W.DI + vomit_signExtend(cpu->fetchOpcodeByte()); break;
                case 2: retv = cpu->regs.W.BP+cpu->regs.W.SI + vomit_signExtend(cpu->fetchOpcodeByte()); break;
                case 3: retv = cpu->regs.W.BP+cpu->regs.W.DI + vomit_signExtend(cpu->fetchOpcodeByte()); break;
                case 4: retv = cpu->regs.W.SI + vomit_signExtend(cpu->fetchOpcodeByte()); break;
                case 5: retv = cpu->regs.W.DI + vomit_signExtend(cpu->fetchOpcodeByte()); break;
                case 6: retv = cpu->regs.W.BP + vomit_signExtend(cpu->fetchOpcodeByte()); break;
                default: retv = cpu->regs.W.BX + vomit_signExtend(cpu->fetchOpcodeByte()); break;
            }
            break;
        case 128:
            switch(b & 0x07)
            {
                case 0: retv = cpu->regs.W.BX+cpu->regs.W.SI+cpu->fetchOpcodeWord(); break;
                case 1: retv = cpu->regs.W.BX+cpu->regs.W.DI+cpu->fetchOpcodeWord(); break;
                case 2: retv = cpu->regs.W.BP+cpu->regs.W.SI+cpu->fetchOpcodeWord(); break;
                case 3: retv = cpu->regs.W.BP+cpu->regs.W.DI+cpu->fetchOpcodeWord(); break;
                case 4: retv = cpu->regs.W.SI + cpu->fetchOpcodeWord(); break;
                case 5: retv = cpu->regs.W.DI + cpu->fetchOpcodeWord(); break;
                case 6: retv = cpu->regs.W.BP + cpu->fetchOpcodeWord(); break;
                default: retv = cpu->regs.W.BX + cpu->fetchOpcodeWord(); break;
            }
            break;
        case 192:
            vlog(VM_ALERT, "LEA with register source!");
            /* LEA with register source, an invalid instruction.
             * Call INT6 (invalid opcode exception) */
            cpu->exception(6);
            break;
    }
    *cpu->treg16[vomit_modRMRegisterPart(b)] = retv;
}

inline bool isVGAMemory(DWORD address)
{
    return address >= 0xA0000 && address < 0xB0000;
}

void VCpu::writeMemory32(DWORD address, DWORD data)
{
#warning FIXME: writeMemory32 to VGA memory
#if 0
    if (isVGAMemory(address)) {
        this->vgaMemory()->write8(address, value);
        return;
    }
#endif

    assert (address < (0xFFFFF - 4));
    DWORD* ptr = reinterpret_cast<DWORD*>(this->memory + address);
    vomit_write32ToPointer(ptr, data);
}

DWORD VCpu::readMemory32(DWORD address) const
{
#warning FIXME: readMemory32 from VGA memory
#if 0
    if (isVGAMemory(address))
        return this->vgaMemory()->read16(address) | (this->vgaMemory()->read16(address + 2) << 16);
#endif
#ifdef VOMIT_DETECT_UNINITIALIZED_ACCESS
    assert (address < (0xFFFFF - 4));
    if (!m_dirtMap[address] || !m_dirtMap[address + 1] || !m_dirtMap[address + 2] || !m_dirtMap[address + 3])
        vlog(VM_MEMORYMSG, "%04X:%08X: Uninitialized read from %08X", getBaseCS(), getBaseEIP(), address);
#endif
    return vomit_read32FromPointer(reinterpret_cast<DWORD*>(this->memory + address));
}

BYTE VCpu::readMemory8(DWORD address) const
{
    if (!isA20Enabled()) {
#ifdef VOMIT_DEBUG
        if (address > 0xFFFFF) {
            vlog(VM_MEMORYMSG, "%04X:%08X Read byte from %08X with A20 disabled, wrapping to %08X", getBaseCS(), getBaseEIP(), address, address & 0xFFFFF);
        }
#endif
        address &= 0xFFFFF;
    }

    if (isVGAMemory(address))
        return this->vgaMemory()->read8(address);
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
    if (!isA20Enabled()) {
        assert(address != 0xFFFFF);
#ifdef VOMIT_DEBUG
        if (address > 0xFFFFF) {
            vlog(VM_MEMORYMSG, "%04X:%08X Read word from %08X with A20 disabled, wrapping to %08X", getBaseCS(), getBaseEIP(), address, address & 0xFFFFF);
        }
#endif
        address &= 0xFFFFF;
    }

    if (isVGAMemory(address))
        return this->vgaMemory()->read16(address);
#ifdef VOMIT_DETECT_UNINITIALIZED_ACCESS
    if (!m_dirtMap[address] || !m_dirtMap[address + 1])
        vlog(VM_MEMORYMSG, "%04X:%04X: Uninitialized read from %08X", getBaseCS(), getBaseIP(), address);
#endif
    return vomit_read16FromPointer(reinterpret_cast<WORD*>(this->memory + address));
}

WORD VCpu::readMemory16(WORD segment, WORD offset) const
{
    return readMemory16(vomit_toFlatAddress(segment, offset));
}

DWORD VCpu::readMemory32(WORD segment, WORD offset) const
{
    return readMemory32(vomit_toFlatAddress(segment, offset));
}

void VCpu::writeMemory8(DWORD address, BYTE value)
{
    if (!isA20Enabled()) {
#ifdef VOMIT_DEBUG
        if (address > 0xFFFFF) {
            vlog(VM_MEMORYMSG, "%04X:%08X Write byte to %08X with A20 disabled, wrapping to %08X", getBaseCS(), getBaseEIP(), address, address & 0xFFFFF);
        }
#endif
        address &= 0xFFFFF;
    }

    if (isVGAMemory(address)) {
        this->vgaMemory()->write8(address, value);
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
    if (!isA20Enabled()) {
        assert(address != 0xFFFFF);

#ifdef VOMIT_DEBUG
        if (address > 0xFFFFF) {
            vlog(VM_MEMORYMSG, "%04X:%08X Write word to %08X with A20 disabled, wrapping to %08X", getBaseCS(), getBaseEIP(), address, address & 0xFFFFF);
        }
#endif

        address &= 0xFFFFF;
    }

    if (isVGAMemory(address)) {
        this->vgaMemory()->write16(address, value);
        return;
    }

#ifdef VOMIT_DETECT_UNINITIALIZED_ACCESS
    m_dirtMap[address] = true;
    m_dirtMap[address + 1] = true;
#endif

    WORD* ptr = reinterpret_cast<WORD*>(this->memory + address);
    vomit_write16ToPointer(ptr, value);
}


void VCpu::writeMemory32(WORD segment, WORD offset, DWORD value)
{
    writeMemory32(vomit_toFlatAddress(segment, offset), value);
}

void VCpu::writeMemory16(WORD segment, WORD offset, WORD value)
{
    writeMemory16(vomit_toFlatAddress(segment, offset), value);
}

#ifdef VOMIT_DEBUG
bool VCpu::inDebugger() const
{
    return m_inDebugger;
}

void VCpu::attachDebugger()
{
    m_inDebugger = true;
}

void VCpu::detachDebugger()
{
    m_inDebugger = false;
}
#endif
