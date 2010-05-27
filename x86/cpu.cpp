/* 8086/cpu.cpp
 * 8086 emulation
 *
 */

#include "vomit.h"
#include "debug.h"
#include "vga_memory.h"

VCpu* g_cpu = 0;
bool g_vomit_exit_main_loop = 0;

// The black hole of 386 segment selectors.
static WORD segment_dummy;

void _OpOverride(VCpu*)
{
    vlog(VM_LOGMSG, "Operation size override detected!");
}

void VCpu::init()
{
    // FIXME: This is silly.
    memset(this, 0, sizeof(VCpu));

    this->memory = new BYTE[1048576 + 65536];
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

	this->vgaMemory = new VgaMemory(this);

#ifdef VOMIT_PREFETCH_QUEUE
    if (m_prefetchQueue)
        delete [] m_prefetchQueue;

    m_prefetchQueueSize = 4;
    m_prefetchQueue = new BYTE[m_prefetchQueueSize];
    m_prefetchQueueIndex = 0;
#endif

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
    this->tseg[6] = &segment_dummy;
    this->tseg[7] = &segment_dummy;

    m_segmentPrefix = 0x0000;
    m_currentSegment = &this->DS;

    jump(0xF000, 0x0000);

    setFlags(0x0200);

    m_instructionsPerTick = 0x50000;
    m_pitCountdown = m_instructionsPerTick;
    m_state = Alive;

#ifdef VOMIT_DEBUG
    m_inDebugger = false;
#endif
}

void VCpu::setOpcodeHandler(BYTE rangeStart, BYTE rangeEnd, OpcodeHandler handler)
{
    for (int i = rangeStart; i <= rangeEnd; ++i) {
        this->opcode_handler[i] = handler;
    }
}

void VCpu::registerDefaultOpcodeHandlers()
{
    // Install a default handler for unsupported instructions
    setOpcodeHandler(0x00, 0xFF, _UNSUPP           );

    // Stubs
    setOpcodeHandler(0x66, 0x66, _OpOverride       );

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
    setOpcodeHandler(0x81, 0x81, _wrap_0x81        );
    setOpcodeHandler(0x83, 0x83, _wrap_0x83        );
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
    setOpcodeHandler(0x8F, 0x8F, _wrap_0x8F        );
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
    setOpcodeHandler(0xD1, 0xD1, _wrap_0xD1        );
    setOpcodeHandler(0xD2, 0xD2, _wrap_0xD2        );
    setOpcodeHandler(0xD3, 0xD3, _wrap_0xD3        );
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
    setOpcodeHandler(0xF7, 0xF7, _wrap_0xF7        );
    setOpcodeHandler(0xF8, 0xF8, _CLC              );
    setOpcodeHandler(0xF9, 0xF9, _STC              );
    setOpcodeHandler(0xFA, 0xFA, _CLI              );
    setOpcodeHandler(0xFB, 0xFB, _STI              );
    setOpcodeHandler(0xFC, 0xFC, _CLD              );
    setOpcodeHandler(0xFD, 0xFD, _STD              );
    setOpcodeHandler(0xFE, 0xFE, _wrap_0xFE        );
    setOpcodeHandler(0xFF, 0xFF, _wrap_0xFF        );

    setOpcodeHandler(0xC0, 0xC0, _wrap_0xC0        );
    setOpcodeHandler(0xC1, 0xC1, _wrap_0xC1        );

#if VOMIT_CPU_LEVEL <= 1
    // Specialized PUSH SP for Intel 8086/80186
    setOpcodeHandler(0x54, 0x54, _PUSH_SP_8086_80186);
#endif

#if VOMIT_CPU_LEVEL >= 1
    // 80186+ instructions
    setOpcodeHandler(0x0F, 0x0F, _wrap_0x0F        );
    setOpcodeHandler(0x60, 0x60, _PUSHA            );
    setOpcodeHandler(0x61, 0x61, _POPA             );
    setOpcodeHandler(0x62, 0x62, _BOUND            );
    setOpcodeHandler(0x68, 0x68, _PUSH_imm16       );
    setOpcodeHandler(0x6A, 0x6A, _PUSH_imm8        );
    setOpcodeHandler(0x6B, 0x6B, _IMUL_reg16_RM16_imm8 );
    setOpcodeHandler(0xC0, 0xC0, _wrap_0xC0        );
    setOpcodeHandler(0xC1, 0xC1, _wrap_0xC1        );
    setOpcodeHandler(0xC8, 0xC8, _ENTER            );
    setOpcodeHandler(0xC9, 0xC9, _LEAVE            );
#endif
}

void VCpu::kill()
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

void VCpu::exec()
{
    // TODO: Be more clever with this, it's mostly a waste of time.
    m_baseCS = getCS();
    m_baseIP = getIP();

    this->opcode = fetchOpcodeByte();
    this->opcode_handler[this->opcode](this);
}

void VCpu::haltedLoop()
{
    while (state() == VCpu::Halted) {
        usleep(500);
        if (g_pic_pending_requests)
            pic_service_irq(this);
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
        if (inDebugger()) {

            m_baseCS = getCS();
            m_baseIP = getIP();

            debugger();

            if (!inDebugger())
                continue;
        }
#endif

#ifdef VOMIT_TRACE
        if (options.trace) {
            dump_disasm(getCS(), getIP());
            dump_regs(this);
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
            irq(0);
        }

        if (g_pic_pending_requests && getIF())
            pic_service_irq(this);
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

void VCpu::jumpAbsolute16(WORD address)
{
    this->IP = address;
    flushFetchQueue();
}

void VCpu::jump(WORD segment, WORD offset)
{
    // Jump to specified location.
    this->CS = segment;
    this->IP = offset;

    // Point m_codeMemory to CS:0 for fast opcode fetching.
    m_codeMemory = this->memory + (segment << 4);
    flushFetchQueue();
}

void VCpu::setInterruptHandler(BYTE isr, WORD segment, WORD offset)
{
    writeMemory16(0x0000, (isr * 4), offset);
    writeMemory16(0x0000, (isr * 4) + 2, segment);
}

void _UNSUPP(VCpu* cpu)
{
    // We've come across an unsupported instruction, log it, then vector to the "illegal instruction" ISR.
    vlog(VM_ALERT, "%04X:%04X: Unsupported opcode %02X", cpu->getBaseCS(), cpu->getBaseIP(), cpu->opcode);
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
        vlog(VM_ALERT, "%04X:%04X: Halted with IF=0", cpu->getBaseCS(), cpu->getBaseIP());
        vm_exit(0);
    }

    vlog(VM_CPUMSG, "%04X:%04X Halted", cpu->getBaseCS(), cpu->getBaseIP());

    cpu->haltedLoop();
}

void _XLAT(VCpu* cpu)
{
    cpu->regs.B.AL = cpu->readMemory8(cpu->currentSegment(), cpu->regs.W.BX + cpu->regs.B.AL);
}

void _CS(VCpu* cpu)
{
    cpu->setSegmentPrefix(cpu->getCS());
    cpu->opcode_handler[cpu->fetchOpcodeByte()](cpu);
    cpu->resetSegmentPrefix();
}

void _DS(VCpu* cpu)
{
    cpu->setSegmentPrefix(cpu->getDS());
    cpu->opcode_handler[cpu->fetchOpcodeByte()](cpu);
    cpu->resetSegmentPrefix();
}

void _ES(VCpu* cpu)
{
    cpu->setSegmentPrefix(cpu->getES());
    cpu->opcode_handler[cpu->fetchOpcodeByte()](cpu);
    cpu->resetSegmentPrefix();
}

void _SS(VCpu* cpu)
{
    cpu->setSegmentPrefix(cpu->getSS());
    cpu->opcode_handler[cpu->fetchOpcodeByte()](cpu);
    cpu->resetSegmentPrefix();
}

void _XCHG_AX_reg16(VCpu* cpu)
{
    swap(*cpu->treg16[cpu->opcode & 7], cpu->regs.W.AX);
}

void _XCHG_reg8_RM8(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    BYTE &reg(*cpu->treg8[rmreg(rm)]);

    BYTE value = vomit_cpu_modrm_read8(cpu, rm);
    BYTE tmp = reg;
    reg = value;
    vomit_cpu_modrm_update8(cpu, tmp);
}

void _XCHG_reg16_RM16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    WORD value = vomit_cpu_modrm_read16(cpu, rm);
    WORD tmp = *cpu->treg16[rmreg(rm)];
    *cpu->treg16[rmreg(rm)] = value;
    vomit_cpu_modrm_update16(cpu, tmp);
}

void _DEC_reg16(VCpu* cpu)
{
    WORD &reg(*cpu->treg16[cpu->opcode & 7]);

    DWORD i = reg;

    /* Overflow if we'll wrap. */
    cpu->setOF(reg == 0x8000);

    --i;
    vomit_cpu_setAF(cpu, i, reg, 1);
    cpu->updateFlags16(i);
    --reg;
}

void _INC_reg16(VCpu* cpu)
{
    WORD &reg(*cpu->treg16[cpu->opcode & 7]);
    DWORD i = reg;

    /* Overflow if we'll wrap. */
    cpu->setOF(i == 0x7FFF);

    ++i;
    vomit_cpu_setAF(cpu, i, reg, 1);
    cpu->updateFlags16(i);
    ++reg;
}

void _INC_RM16(VCpu* cpu)
{
    WORD value = vomit_cpu_modrm_read16(cpu, cpu->rmbyte);
    DWORD i = value;

    /* Overflow if we'll wrap. */
    cpu->setOF(value == 0x7FFF);

    ++i;
    vomit_cpu_setAF(cpu, i, value, 1);
    cpu->updateFlags16(i);
    vomit_cpu_modrm_update16(cpu, value + 1);
}

void _DEC_RM16(VCpu* cpu)
{
    WORD value = vomit_cpu_modrm_read16(cpu, cpu->rmbyte);
    DWORD i = value;

    /* Overflow if we'll wrap. */
    cpu->setOF(value == 0x8000);

    --i;
    vomit_cpu_setAF(cpu, i, value, 1); // XXX: i can be (dword)(-1)...
    cpu->updateFlags16(i);
    vomit_cpu_modrm_update16(cpu, value - 1);
}

void _LDS_reg16_mem16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    DWORD value = vomit_cpu_modrm_read32(cpu, rm);
    *cpu->treg16[rmreg(rm)] = LSW(value);
    cpu->DS = MSW(value);
}
void _LES_reg16_mem16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    DWORD value = vomit_cpu_modrm_read32(cpu, rm);
    *cpu->treg16[rmreg(rm)] = LSW(value);
    cpu->ES = MSW(value);
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
                case 0: retv = cpu->regs.W.BX+cpu->regs.W.SI + signext(cpu->fetchOpcodeByte()); break;
                case 1: retv = cpu->regs.W.BX+cpu->regs.W.DI + signext(cpu->fetchOpcodeByte()); break;
                case 2: retv = cpu->regs.W.BP+cpu->regs.W.SI + signext(cpu->fetchOpcodeByte()); break;
                case 3: retv = cpu->regs.W.BP+cpu->regs.W.DI + signext(cpu->fetchOpcodeByte()); break;
                case 4: retv = cpu->regs.W.SI + signext(cpu->fetchOpcodeByte()); break;
                case 5: retv = cpu->regs.W.DI + signext(cpu->fetchOpcodeByte()); break;
                case 6: retv = cpu->regs.W.BP + signext(cpu->fetchOpcodeByte()); break;
                default: retv = cpu->regs.W.BX + signext(cpu->fetchOpcodeByte()); break;
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
    *cpu->treg16[rmreg(b)] = retv;
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
