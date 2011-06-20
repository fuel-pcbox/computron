/*
 * Copyright (C) 2003-2011 Andreas Kling <kling@webkit.org>
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

#include "vomit.h"
#include "vcpu.h"
#include "debug.h"
#include "debugger.h"
#include "vga_memory.h"
#include "pic.h"
#include <QtCore/QStringList>

VCpu* g_cpu = 0;
bool g_vomit_exit_main_loop = 0;

#define CALL_HANDLER(handler16, handler32) if (o16()) { handler16(); } else { handler32(); }

void VCpu::_UD0()
{
    vlog(LogAlert, "Undefined opcode 0F FF (UD0)");
    exception(6);
}

void VCpu::_OperationSizeOverride()
{
    m_operationSize32 = !m_operationSize32;

#ifdef VOMIT_DEBUG_OVERRIDE_OPCODES
    vlog(LogCPU, "%04X:%08X Operation size override detected! Opcode: db 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X ",
         getBaseCS(),
         getBaseEIP(),
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

    m_operationSize32 = !m_operationSize32;
}

void VCpu::_AddressSizeOverride()
{
    m_addressSize32 = !m_addressSize32;

#ifdef VOMIT_DEBUG_OVERRIDE_OPCODES
    vlog(LogCPU, "%04X:%08X Address size override detected! Opcode: db 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X ",
         getBaseCS(),
         getBaseEIP(),
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

    m_addressSize32 = !m_addressSize32;
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
    case 0x00: _ADD_RM8_reg8(); break;
    case 0x01: CALL_HANDLER(_ADD_RM16_reg16, _ADD_RM32_reg32); break;
    case 0x02: _ADD_reg8_RM8(); break;
    case 0x03: CALL_HANDLER(_ADD_reg16_RM16, _ADD_reg32_RM32); break;
    case 0x04: _ADD_AL_imm8(); break;
    case 0x05: CALL_HANDLER(_ADD_AX_imm16, _ADD_EAX_imm32); break;
    case 0x06: _PUSH_ES(); break;
    case 0x07: _POP_ES(); break;
    case 0x08: _OR_RM8_reg8(); break;
    case 0x09: CALL_HANDLER(_OR_RM16_reg16, _OR_RM32_reg32); break;
    case 0x0A: _OR_reg8_RM8(); break;
    case 0x0B: CALL_HANDLER(_OR_reg16_RM16, _OR_reg32_RM32); break;
    case 0x0C: _OR_AL_imm8(); break;
    case 0x0D: CALL_HANDLER(_OR_AX_imm16, _OR_EAX_imm32); break;
    case 0x0E: _PUSH_CS(); break;
    case 0x0F:
        this->rmbyte = fetchOpcodeByte();
        switch (this->rmbyte) {
        case 0x01:
            this->subrmbyte = fetchOpcodeByte();
            switch (vomit_modRMRegisterPart(this->subrmbyte)) {
            case 0: _SGDT(); break;
            case 1: _SIDT(); break;
            case 2: _LGDT(); break;
            case 3: _LIDT(); break;
            case 6: _LMSW(); break;
            default: goto fffuuu;
            }
            break;
        case 0x20: _MOV_reg32_CR(); break;
        case 0x22: _MOV_CR_reg32(); break;
        case 0x80: _JO_NEAR_imm(); break;
        case 0x81: _JNO_NEAR_imm(); break;
        case 0x82: _JC_NEAR_imm(); break;
        case 0x83: _JNC_NEAR_imm(); break;
        case 0x84: _JZ_NEAR_imm(); break;
        case 0x85: _JNZ_NEAR_imm(); break;
        case 0x86: _JNA_NEAR_imm(); break;
        case 0x87: _JA_NEAR_imm(); break;
        case 0x88: _JS_NEAR_imm(); break;
        case 0x89: _JNS_NEAR_imm(); break;
        case 0x8A: _JP_NEAR_imm(); break;
        case 0x8B: _JNP_NEAR_imm(); break;
        case 0x8C: _JL_NEAR_imm(); break;
        case 0x8D: _JNL_NEAR_imm(); break;
        case 0x8E: _JNG_NEAR_imm(); break;
        case 0x8F: _JG_NEAR_imm(); break;
        case 0xA0: _PUSH_FS(); break;
        case 0xA1: _POP_FS(); break;
        case 0xA8: _PUSH_GS(); break;
        case 0xA9: _POP_GS(); break;
        case 0xB4: CALL_HANDLER(_LFS_reg16_mem16, _LFS_reg32_mem32); break;
        case 0xB5: CALL_HANDLER(_LFS_reg16_mem16, _LFS_reg32_mem32); break;
        case 0xB6: CALL_HANDLER(_MOVZX_reg16_RM8, _MOVZX_reg32_RM8); break;
        case 0xB7: CALL_HANDLER(_UNSUPP, _MOVZX_reg32_RM16); break;
        case 0xFF: _UD0(); break;
        default: goto fffuuu;
        }
        break;
    case 0x10: _ADC_RM8_reg8(); break;
    case 0x11: CALL_HANDLER(_ADC_RM16_reg16, _ADC_RM32_reg32); break;
    case 0x12: _ADC_reg8_RM8(); break;
    case 0x13: CALL_HANDLER(_ADC_reg16_RM16, _ADC_reg32_RM32); break;
    case 0x14: _ADC_AL_imm8(); break;
    case 0x15: CALL_HANDLER(_ADC_AX_imm16, _ADC_EAX_imm32); break;
    case 0x16: _PUSH_SS(); break;
    case 0x17: _POP_SS(); break;
    case 0x18: _SBB_RM8_reg8(); break;
    case 0x19: CALL_HANDLER(_SBB_RM16_reg16, _SBB_RM32_reg32); break;
    case 0x1A: _SBB_reg8_RM8(); break;
    case 0x1B: CALL_HANDLER(_SBB_reg16_RM16, _SBB_reg32_RM32); break;
    case 0x1C: _SBB_AL_imm8(); break;
    case 0x1D: CALL_HANDLER(_SBB_AX_imm16, _SBB_EAX_imm32); break;
    case 0x1E: _PUSH_DS(); break;
    case 0x1F: _POP_DS(); break;
    case 0x20: _AND_RM8_reg8(); break;
    case 0x21: CALL_HANDLER(_AND_RM16_reg16, _AND_RM32_reg32); break;
    case 0x22: _AND_reg8_RM8(); break;
    case 0x23: CALL_HANDLER(_AND_reg16_RM16, _AND_reg32_RM32); break;
    case 0x24: _AND_AL_imm8(); break;
    case 0x25: CALL_HANDLER(_AND_AX_imm16, _AND_EAX_imm32); break;
    case 0x26: _ES(); break;
    case 0x27: _DAA(); break;
    case 0x28: _SUB_RM8_reg8(); break;
    case 0x29: CALL_HANDLER(_SUB_RM16_reg16, _SUB_RM32_reg32); break;
    case 0x2A: _SUB_reg8_RM8(); break;
    case 0x2B: CALL_HANDLER(_SUB_reg16_RM16, _SUB_reg32_RM32); break;
    case 0x2C: _SUB_AL_imm8(); break;
    case 0x2D: CALL_HANDLER(_SUB_AX_imm16, _SUB_EAX_imm32); break;
    case 0x2E: _CS(); break;
    case 0x2F: _DAS(); break;
    case 0x30: _XOR_RM8_reg8(); break;
    case 0x31: CALL_HANDLER(_XOR_RM16_reg16, _XOR_RM32_reg32); break;
    case 0x32: _XOR_reg8_RM8(); break;
    case 0x33: CALL_HANDLER(_XOR_reg16_RM16, _XOR_reg32_RM32); break;
    case 0x34: _XOR_AL_imm8(); break;
    case 0x35: CALL_HANDLER(_XOR_AX_imm16, _XOR_EAX_imm32); break;
    case 0x36: _SS(); break;
    case 0x37: _AAA(); break;
    case 0x38: _CMP_RM8_reg8(); break;
    case 0x39: CALL_HANDLER(_CMP_RM16_reg16, _CMP_RM32_reg32); break;
    case 0x3A: _CMP_reg8_RM8(); break;
    case 0x3B: CALL_HANDLER(_CMP_reg16_RM16, _CMP_reg32_RM32); break;
    case 0x3C: _CMP_AL_imm8(); break;
    case 0x3D: CALL_HANDLER(_CMP_AX_imm16, _CMP_EAX_imm32); break;
    case 0x3E: _DS(); break;
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
    case 0x64: _FS(); break;
    case 0x65: _GS(); break;
    case 0x66: _OperationSizeOverride(); break;
    case 0x67: _AddressSizeOverride(); break;
    case 0x68: CALL_HANDLER(_PUSH_imm16, _PUSH_imm32); break;
    case 0x6A: _PUSH_imm8(); break;
    case 0x6E: _OUTSB(); break;
    case 0x70: _JO_imm8(); break;
    case 0x71: _JNO_imm8(); break;
    case 0x72: _JC_imm8(); break;
    case 0x73: _JNC_imm8(); break;
    case 0x74: _JZ_imm8(); break;
    case 0x75: _JNZ_imm8(); break;
    case 0x76: _JNA_imm8(); break;
    case 0x77: _JA_imm8(); break;
    case 0x78: _JS_imm8(); break;
    case 0x79: _JNS_imm8(); break;
    case 0x7A: _JP_imm8(); break;
    case 0x7B: _JNP_imm8(); break;
    case 0x7C: _JL_imm8(); break;
    case 0x7D: _JNL_imm8(); break;
    case 0x7E: _JNG_imm8(); break;
    case 0x7F: _JG_imm8(); break;
    case 0x80: _wrap_0x80(); break;
    case 0x81: CALL_HANDLER(_wrap_0x81_16, _wrap_0x81_32); break;
    case 0x83: CALL_HANDLER(_wrap_0x83_16, _wrap_0x83_32); break;
    case 0x84: _TEST_RM8_reg8(); break;
    case 0x85: CALL_HANDLER(_TEST_RM16_reg16, _TEST_RM32_reg32); break;
    case 0x86: _XCHG_reg8_RM8(); break;
    case 0x87: CALL_HANDLER(_XCHG_reg16_RM16, _XCHG_reg32_RM32); break;
    case 0x88: _MOV_RM8_reg8(); break;
    case 0x89: CALL_HANDLER(_MOV_RM16_reg16, _MOV_RM32_reg32); break;
    case 0x8A: _MOV_reg8_RM8(); break;
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
    case 0x9E: _SAHF(); break;
    case 0x9F: _LAHF(); break;
    case 0xA0: _MOV_AL_moff8(); break;
    case 0xA1: CALL_HANDLER(_MOV_AX_moff16, _MOV_EAX_moff32); break;
    case 0xA2: _MOV_moff8_AL(); break;
    case 0xA3: CALL_HANDLER(_MOV_moff16_AX, _MOV_moff32_EAX); break;
    case 0xA4: _MOVSB(); break;
    case 0xA5: CALL_HANDLER(_MOVSW, _MOVSD); break;
    case 0xA6: _CMPSB(); break;
    case 0xA7: CALL_HANDLER(_CMPSW, _CMPSD); break;
    case 0xA8: _TEST_AL_imm8(); break;
    case 0xA9: CALL_HANDLER(_TEST_AX_imm16, _TEST_EAX_imm32); break;
    case 0xAA: _STOSB(); break;
    case 0xAB: CALL_HANDLER(_STOSW, _STOSD); break;
    case 0xAC: _LODSB(); break;
    case 0xAD: CALL_HANDLER(_LODSW, _LODSD); break;
    case 0xAE: _SCASB(); break;
    case 0xAF: CALL_HANDLER(_SCASW, _SCASD); break;
    case 0xB0: _MOV_AL_imm8(); break;
    case 0xB1: _MOV_CL_imm8(); break;
    case 0xB2: _MOV_DL_imm8(); break;
    case 0xB3: _MOV_BL_imm8(); break;
    case 0xB4: _MOV_AH_imm8(); break;
    case 0xB5: _MOV_CH_imm8(); break;
    case 0xB6: _MOV_DH_imm8(); break;
    case 0xB7: _MOV_BH_imm8(); break;
    case 0xB8: CALL_HANDLER(_MOV_AX_imm16, _MOV_EAX_imm32); break;
    case 0xB9: CALL_HANDLER(_MOV_CX_imm16, _MOV_ECX_imm32); break;
    case 0xBA: CALL_HANDLER(_MOV_DX_imm16, _MOV_EDX_imm32); break;
    case 0xBB: CALL_HANDLER(_MOV_BX_imm16, _MOV_EBX_imm32); break;
    case 0xBC: CALL_HANDLER(_MOV_SP_imm16, _MOV_ESP_imm32); break;
    case 0xBD: CALL_HANDLER(_MOV_BP_imm16, _MOV_EBP_imm32); break;
    case 0xBE: CALL_HANDLER(_MOV_SI_imm16, _MOV_ESI_imm32); break;
    case 0xBF: CALL_HANDLER(_MOV_DI_imm16, _MOV_EDI_imm32); break;
    case 0xC0: _wrap_0xC0(); break;
    case 0xC1: CALL_HANDLER(_wrap_0xC1_16, _wrap_0xC1_32); break;
    case 0xC2: _RET_imm16(); break;
    case 0xC3: _RET(); break;
    case 0xC4: CALL_HANDLER(_LES_reg16_mem16, _LES_reg32_mem32); break;
    case 0xC5: CALL_HANDLER(_LDS_reg16_mem16, _LDS_reg32_mem32); break;
    case 0xC6: _MOV_RM8_imm8(); break;
    case 0xC7: CALL_HANDLER(_MOV_RM16_imm16, _MOV_RM32_imm32); break;
    case 0xC8: _ENTER(); break;
    case 0xC9: _LEAVE(); break;
    case 0xCA: _RETF_imm16(); break;
    case 0xCB: _RETF(); break;
    case 0xCD: _INT_imm8(); break;
    case 0xCF: _IRET(); break;
    case 0xD0: _wrap_0xD0(); break;
    case 0xD1: CALL_HANDLER(_wrap_0xD1_16, _wrap_0xD1_32); break;
    case 0xD2: _wrap_0xD2(); break;
    case 0xD3: CALL_HANDLER(_wrap_0xD3_16, _wrap_0xD3_32); break;
    case 0xD5: _AAD(); break;
    case 0xD7: _XLAT(); break;
    // BEGIN FPU STUBS
    case 0xD8:
    case 0xD9:
    case 0xDA:
    case 0xDB:
    case 0xDC:
    case 0xDD:
    case 0xDE:
    case 0xDF: _ESCAPE(); break;
    // END FPU STUBS
    case 0xE0: _LOOPNE_imm8(); break;
    case 0xE1: _LOOPE_imm8(); break;
    case 0xE2: _LOOP_imm8(); break;
    case 0xE3: CALL_HANDLER(_JCXZ_imm8, _JECXZ_imm8); break;
    case 0xE4: _IN_AL_imm8(); break;
    case 0xE5: CALL_HANDLER(_IN_AX_imm8, _IN_EAX_imm8); break;
    case 0xE6: _OUT_imm8_AL(); break;
    case 0xE7: CALL_HANDLER(_OUT_imm8_AX, _OUT_imm8_EAX); break;
    case 0xE8: CALL_HANDLER(_CALL_imm16, _CALL_imm32); break;
    case 0xE9: CALL_HANDLER(_JMP_imm16, _JMP_imm32); break;
    case 0xEA: CALL_HANDLER(_JMP_imm16_imm16, _JMP_imm16_imm32); break;
    case 0xEB: _JMP_short_imm8(); break;
    case 0xEC: _IN_AL_DX(); break;
    case 0xED: CALL_HANDLER(_IN_AX_DX, _IN_EAX_DX); break;
    case 0xEE: _OUT_DX_AL(); break;
    case 0xEF: CALL_HANDLER(_OUT_DX_AX, _OUT_DX_EAX); break;
    case 0xF0: /* LOCK */ break;
    case 0xF2: _REPNE(); break;
    case 0xF3: _REP(); break;
    case 0xF4: _HLT(); break;
    case 0xF5: _CMC(); break;
    case 0xF6: _wrap_0xF6(); break;
    case 0xF7: CALL_HANDLER(_wrap_0xF7_16, _wrap_0xF7_32); break;
    case 0xF8: _CLC(); break;
    case 0xF9: _STC(); break;
    case 0xFA: _CLI(); break;
    case 0xFB: _STI(); break;
    case 0xFC: _CLD(); break;
    case 0xFD: _STD(); break;
    case 0xFE: _wrap_0xFE(); break;
    case 0xFF: CALL_HANDLER(_wrap_0xFF_16, _wrap_0xFF_32); break;
    default:
        this->rmbyte = fetchOpcodeByte();
fffuuu:
        vlog(LogAlert, "%04X:%08X FFFFUUUU unsupported opcode %02X /%u or %02X %02X or %02X %02X /%u",
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
    vlog(LogCPU, "#GP(%d) :-(", code);
    vomit_exit(1);
}

VCpu::VCpu(QObject* parent)
    : QObject(parent)
{
    VM_ASSERT(!g_cpu);
    g_cpu = this;

    m_memory = new BYTE[(8192 * 1024) + 65536];
    if (!m_memory) {
        vlog(LogInit, "Insufficient memory available.");
        vomit_exit(1);
    }

    memset(m_memory, 0, 1048576 + 65536);

    m_vgaMemory = new VGAMemory(this);

    m_debugger = new Debugger(this);

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

    m_segmentMap[RegisterCS] = &this->CS;
    m_segmentMap[RegisterDS] = &this->DS;
    m_segmentMap[RegisterES] = &this->ES;
    m_segmentMap[RegisterSS] = &this->SS;
    m_segmentMap[RegisterFS] = &this->FS;
    m_segmentMap[RegisterGS] = &this->GS;
    m_segmentMap[6] = 0;
    m_segmentMap[7] = 0;

    m_segmentPrefix = 0x0000;
    m_currentSegment = &this->DS;

    jump32(0xF000, 0x00000000);

    setFlags(0x0200);

    setIOPL(3);

    m_state = Alive;

    m_addressSize32 = false;
    m_operationSize32 = false;
}

VCpu::~VCpu()
{
    delete [] m_memory;
    m_memory = 0;
    m_codeMemory = 0;

    delete m_debugger;
    delete m_vgaMemory;
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
            PIC::serviceIRQ(this);
    }
}

void VCpu::mainLoop()
{
    forever {

        // HACK: This can be set by an external force to make us break out.
        if (g_vomit_exit_main_loop)
            return;

#ifdef VOMIT_DEBUG

        if (!m_breakPoints.isEmpty()) {
            DWORD flatPC = vomit_toFlatAddress(getCS(), getEIP());
            foreach (DWORD breakPoint, m_breakPoints) {
                if (flatPC == breakPoint) {
                    debugger()->enter();
                    break;
                }
            }
        }

        if (debugger()->isActive()) {
            saveBaseAddress();
            debugger()->doConsole();
            if (!debugger()->isActive())
                continue;
        }
#endif

#ifdef VOMIT_TRACE
        if (options.trace)
            dumpTrace();
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
            PIC::serviceIRQ(this);
    }
}

void VCpu::jumpRelative8(SIGNED_BYTE displacement)
{
    this->IP += displacement;
}

void VCpu::jumpRelative16(SIGNED_WORD displacement)
{
    this->IP += displacement;
}

void VCpu::jumpRelative32(SIGNED_DWORD displacement)
{
    this->EIP += displacement;
}

void VCpu::jumpAbsolute16(WORD address)
{
    this->IP = address;
}

void VCpu::jump32(WORD segment, DWORD offset)
{
    // Jump to specified location.
    this->CS = segment;
    this->EIP = offset;

    // Point m_codeMemory to CS:0 for fast opcode fetching.
    m_codeMemory = m_memory + (segment << 4);
}

void VCpu::jump16(WORD segment, WORD offset)
{
    jump32(segment, offset);
}

void VCpu::_UNSUPP()
{
    // We've come across an unsupported instruction, log it, then vector to the "illegal instruction" ISR.
    vlog(LogAlert, "%04X:%08X: Unsupported opcode %02X", getBaseCS(), getBaseEIP(), opcode);
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

void VCpu::_NOP()
{
}

void VCpu::_HLT()
{
    setState(VCpu::Halted);

    if (!getIF())
        vlog(LogAlert, "%04X:%08X: Halted with IF=0", getBaseCS(), getBaseEIP());
    else
        vlog(LogCPU, "%04X:%08X Halted", getBaseCS(), getBaseEIP());

    haltedLoop();
}

void VCpu::_XLAT()
{
    setAL(readMemory8(currentSegment(), getBX() + getAL()));
}

void VCpu::_CS()
{
    setSegmentPrefix(getCS());
    decode(fetchOpcodeByte());
    resetSegmentPrefix();
}

void VCpu::_DS()
{
    setSegmentPrefix(getDS());
    decode(fetchOpcodeByte());
    resetSegmentPrefix();
}

void VCpu::_ES()
{
    setSegmentPrefix(getES());
    decode(fetchOpcodeByte());
    resetSegmentPrefix();
}

void VCpu::_SS()
{
    setSegmentPrefix(getSS());
    decode(fetchOpcodeByte());
    resetSegmentPrefix();
}

void VCpu::_FS()
{
    setSegmentPrefix(getFS());
    decode(fetchOpcodeByte());
    resetSegmentPrefix();
}

void VCpu::_GS()
{
    setSegmentPrefix(getGS());
    decode(fetchOpcodeByte());
    resetSegmentPrefix();
}

void VCpu::_XCHG_AX_reg16()
{
    qSwap(*treg16[opcode & 7], regs.W.AX);
}

void VCpu::_XCHG_EAX_reg32()
{
    qSwap(*treg32[opcode & 7], regs.D.EAX);
}

void VCpu::_XCHG_reg8_RM8()
{
    BYTE rm = fetchOpcodeByte();
    BYTE &reg(*treg8[vomit_modRMRegisterPart(rm)]);

    BYTE value = readModRM8(rm);
    BYTE tmp = reg;
    reg = value;
    updateModRM8(tmp);
}

void VCpu::_XCHG_reg16_RM16()
{
    BYTE rm = fetchOpcodeByte();
    WORD value = readModRM16(rm);
    WORD tmp = getRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm)));
    setRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm)), value);
    updateModRM16(tmp);
}

void VCpu::_XCHG_reg32_RM32()
{
    BYTE rm = fetchOpcodeByte();
    DWORD value = readModRM32(rm);
    DWORD tmp = getRegister32(static_cast<VCpu::RegisterIndex32>(vomit_modRMRegisterPart(rm)));
    setRegister32(static_cast<VCpu::RegisterIndex32>(vomit_modRMRegisterPart(rm)), value);
    updateModRM32(tmp);
}

void VCpu::_DEC_reg16()
{
    WORD &reg(*treg16[opcode & 7]);
    DWORD i = reg;

    /* Overflow if we'll wrap. */
    setOF(reg == 0x8000);

    --i;
    adjustFlag32(i, reg, 1);
    updateFlags16(i);
    --reg;
}

void VCpu::_DEC_reg32()
{
    DWORD &reg(*treg32[opcode & 7]);
    QWORD i = reg;

    /* Overflow if we'll wrap. */
    setOF(reg == 0x80000000);

    --i;
    adjustFlag32(i, reg, 1);
    updateFlags32(i);
    --reg;
}

void VCpu::_INC_reg16()
{
    WORD &reg(*treg16[opcode & 7]);
    DWORD i = reg;

    /* Overflow if we'll wrap. */
    setOF(i == 0x7FFF);

    ++i;
    adjustFlag32(i, reg, 1);
    updateFlags16(i);
    ++reg;
}

void VCpu::_INC_reg32()
{
    DWORD &reg(*treg32[opcode & 7]);
    QWORD i = reg;

    /* Overflow if we'll wrap. */
    setOF(i == 0x7FFFFFFF);

    ++i;
    adjustFlag32(i, reg, 1);
    updateFlags32(i);
    ++reg;
}

void VCpu::_INC_RM16()
{
    WORD value = readModRM16(rmbyte);
    DWORD i = value;

    /* Overflow if we'll wrap. */
    setOF(value == 0x7FFF);

    ++i;
    adjustFlag32(i, value, 1);
    updateFlags16(i);
    updateModRM16(value + 1);
    updateModRM16(value + 1);
}

void VCpu::_DEC_RM16()
{
    WORD value = readModRM16(rmbyte);
    DWORD i = value;

    /* Overflow if we'll wrap. */
    setOF(value == 0x8000);

    --i;
    adjustFlag32(i, value, 1); // XXX: i can be (dword)(-1)...
    updateFlags16(i);
    updateModRM16(value - 1);
}

void VCpu::_INC_RM8()
{
    BYTE value = readModRM8(this->rmbyte);
    WORD i = value;
    setOF(value == 0x7F);
    i++;
    adjustFlag32(i, value, 1);
    updateFlags8(i);
    updateModRM8(value + 1);
}

void VCpu::_DEC_RM8()
{
    BYTE value = readModRM8(this->rmbyte);
    WORD i = value;
    setOF(value == 0x80);
    i--;
    adjustFlag32(i, value, 1);
    updateFlags8(i);
    updateModRM8(value - 1);
}

void VCpu::_LDS_reg16_mem16()
{
    BYTE rm = fetchOpcodeByte();
    WORD* ptr = static_cast<WORD*>(resolveModRM8(rm));
    setRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm)), vomit_read16FromPointer(ptr));
    setDS(vomit_read16FromPointer(ptr + 1));
}

void VCpu::_LDS_reg32_mem32()
{
#warning FIXME: need readModRM48
    vlog(LogAlert, "LDS reg32 mem32");
    vomit_exit(0);
}

void VCpu::_LES_reg16_mem16()
{
    BYTE rm = fetchOpcodeByte();
    WORD* ptr = static_cast<WORD*>(resolveModRM8(rm));
    setRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm)), vomit_read16FromPointer(ptr));
    setES(vomit_read16FromPointer(ptr + 1));
}

void VCpu::_LES_reg32_mem32()
{
#warning FIXME: need readModRM48
    vlog(LogAlert, "LES reg32 mem32");
    vomit_exit(0);
}

void VCpu::_LFS_reg16_mem16()
{
    BYTE rm = fetchOpcodeByte();
    WORD* ptr = static_cast<WORD*>(resolveModRM8(rm));
    setRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm)), vomit_read16FromPointer(ptr));
    setFS(vomit_read16FromPointer(ptr + 1));
}

void VCpu::_LFS_reg32_mem32()
{
#warning FIXME: need readModRM48
    vlog(LogAlert, "LFS reg32 mem32");
    vomit_exit(0);
}

void VCpu::_LGS_reg16_mem16()
{
    BYTE rm = fetchOpcodeByte();
    WORD* ptr = static_cast<WORD*>(resolveModRM8(rm));
    setRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm)), vomit_read16FromPointer(ptr));
    setGS(vomit_read16FromPointer(ptr + 1));
}

void VCpu::_LGS_reg32_mem32()
{
#warning FIXME: need readModRM48
    vlog(LogAlert, "LGS reg32 mem32");
    vomit_exit(0);
}

void VCpu::_LEA_reg32_mem32()
{
#warning FIXME: need evaluateSIB()
    vlog(LogAlert, "LEA reg32 mem32");
    vomit_exit(0);
}

void VCpu::_LEA_reg16_mem16()
{
    WORD retv = 0x0000;
    BYTE b = fetchOpcodeByte();
    switch (b & 0xC0) {
        case 0:
            switch(b & 0x07)
            {
                case 0: retv = regs.W.BX+regs.W.SI; break;
                case 1: retv = regs.W.BX+regs.W.DI; break;
                case 2: retv = regs.W.BP+regs.W.SI; break;
                case 3: retv = regs.W.BP+regs.W.DI; break;
                case 4: retv = regs.W.SI; break;
                case 5: retv = regs.W.DI; break;
                case 6: retv = fetchOpcodeWord(); break;
                default: retv = regs.W.BX; break;
            }
            break;
        case 64:
            switch(b & 0x07)
            {
                case 0: retv = regs.W.BX+regs.W.SI + vomit_signExtend(fetchOpcodeByte()); break;
                case 1: retv = regs.W.BX+regs.W.DI + vomit_signExtend(fetchOpcodeByte()); break;
                case 2: retv = regs.W.BP+regs.W.SI + vomit_signExtend(fetchOpcodeByte()); break;
                case 3: retv = regs.W.BP+regs.W.DI + vomit_signExtend(fetchOpcodeByte()); break;
                case 4: retv = regs.W.SI + vomit_signExtend(fetchOpcodeByte()); break;
                case 5: retv = regs.W.DI + vomit_signExtend(fetchOpcodeByte()); break;
                case 6: retv = regs.W.BP + vomit_signExtend(fetchOpcodeByte()); break;
                default: retv = regs.W.BX + vomit_signExtend(fetchOpcodeByte()); break;
            }
            break;
        case 128:
            switch(b & 0x07)
            {
                case 0: retv = regs.W.BX+regs.W.SI+fetchOpcodeWord(); break;
                case 1: retv = regs.W.BX+regs.W.DI+fetchOpcodeWord(); break;
                case 2: retv = regs.W.BP+regs.W.SI+fetchOpcodeWord(); break;
                case 3: retv = regs.W.BP+regs.W.DI+fetchOpcodeWord(); break;
                case 4: retv = regs.W.SI + fetchOpcodeWord(); break;
                case 5: retv = regs.W.DI + fetchOpcodeWord(); break;
                case 6: retv = regs.W.BP + fetchOpcodeWord(); break;
                default: retv = regs.W.BX + fetchOpcodeWord(); break;
            }
            break;
        case 192:
            vlog(LogAlert, "LEA with register source!");
            /* LEA with register source, an invalid instruction.
             * Call INT6 (invalid opcode exception) */
            exception(6);
            break;
    }
    setRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(b)), retv);
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
        vgaMemory()->write8(address, value);
        return;
    }
#endif

    assert (address < (0xFFFFF - 4));
    DWORD* ptr = reinterpret_cast<DWORD*>(m_memory + address);
    vomit_write32ToPointer(ptr, data);
}

DWORD VCpu::readMemory32(DWORD address) const
{
#warning FIXME: readMemory32 from VGA memory
#if 0
    if (isVGAMemory(address))
        return vgaMemory()->read16(address) | (vgaMemory()->read16(address + 2) << 16);
#endif
    return vomit_read32FromPointer(reinterpret_cast<DWORD*>(m_memory + address));
}

BYTE VCpu::readMemory8(DWORD address) const
{
    if (!isA20Enabled()) {
#ifdef VOMIT_DEBUG
        if (address > 0xFFFFF) {
            vlog(LogCPU, "%04X:%08X Read byte from %08X with A20 disabled, wrapping to %08X", getBaseCS(), getBaseEIP(), address, address & 0xFFFFF);
        }
#endif
        address &= 0xFFFFF;
    }

    if (isVGAMemory(address))
        return vgaMemory()->read8(address);
    return m_memory[address];
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
            vlog(LogCPU, "%04X:%08X Read word from %08X with A20 disabled, wrapping to %08X", getBaseCS(), getBaseEIP(), address, address & 0xFFFFF);
        }
#endif
        address &= 0xFFFFF;
    }

    if (isVGAMemory(address))
        return vgaMemory()->read16(address);
    return vomit_read16FromPointer(reinterpret_cast<WORD*>(m_memory + address));
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
            vlog(LogCPU, "%04X:%08X Write byte to %08X with A20 disabled, wrapping to %08X", getBaseCS(), getBaseEIP(), address, address & 0xFFFFF);
        }
#endif
        address &= 0xFFFFF;
    }

    if (isVGAMemory(address)) {
        vgaMemory()->write8(address, value);
        return;
    }

    m_memory[address] = value;
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
            vlog(LogCPU, "%04X:%08X Write word to %08X with A20 disabled, wrapping to %08X", getBaseCS(), getBaseEIP(), address, address & 0xFFFFF);
        }
#endif

        address &= 0xFFFFF;
    }

    if (isVGAMemory(address)) {
        vgaMemory()->write16(address, value);
        return;
    }

    WORD* ptr = reinterpret_cast<WORD*>(m_memory + address);
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
