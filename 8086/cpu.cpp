/* 8086/cpu.cpp
 * 8086 emulation
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "vomit.h"
#include "debug.h"

#define INSNS_PER_PIT_IRQ 400000

vomit_cpu_t g_cpu;
unsigned int g_vomit_exit_main_loop = 0;

/* The black hole of 386 segment selectors. */
static word segment_dummy;

static void vomit_cpu_install_default_handlers(vomit_cpu_t *cpu);

void _OpOverride(vomit_cpu_t *)
{
    vlog(VM_LOGMSG, "Operation size override detected!");
}

void vomit_cpu_init(vomit_cpu_t *cpu)
{
    cpu->memory = (BYTE *)malloc(1048576 + 65536);
    if (!cpu->memory) {
        vlog(VM_INITMSG, "Insufficient memory available.");
        vm_exit(1);
    }

    memset(cpu->memory, 0, 1048576 + 65536);

#ifdef VOMIT_PREFETCH_QUEUE
    if (cpu->pfq)
        free(cpu->pfq);

    cpu->pfq_size = 4;
    cpu->pfq = malloc(cpu->pfq_size);
    cpu->pfq_current = 0;
#endif

    cpu->treg16[REG_AX] = &cpu->regs.W.AX;
    cpu->treg16[REG_BX] = &cpu->regs.W.BX;
    cpu->treg16[REG_CX] = &cpu->regs.W.CX;
    cpu->treg16[REG_DX] = &cpu->regs.W.DX;
    cpu->treg16[REG_SP] = &cpu->regs.W.SP;
    cpu->treg16[REG_BP] = &cpu->regs.W.BP;
    cpu->treg16[REG_SI] = &cpu->regs.W.SI;
    cpu->treg16[REG_DI] = &cpu->regs.W.DI;

    cpu->treg8[REG_AH] = &cpu->regs.B.AH;
    cpu->treg8[REG_BH] = &cpu->regs.B.BH;
    cpu->treg8[REG_CH] = &cpu->regs.B.CH;
    cpu->treg8[REG_DH] = &cpu->regs.B.DH;
    cpu->treg8[REG_AL] = &cpu->regs.B.AL;
    cpu->treg8[REG_BL] = &cpu->regs.B.BL;
    cpu->treg8[REG_CL] = &cpu->regs.B.CL;
    cpu->treg8[REG_DL] = &cpu->regs.B.DL;

    cpu->tseg[REG_CS] = &cpu->CS;
    cpu->tseg[REG_DS] = &cpu->DS;
    cpu->tseg[REG_ES] = &cpu->ES;
    cpu->tseg[REG_SS] = &cpu->SS;
    cpu->tseg[REG_FS] = &cpu->FS;
    cpu->tseg[REG_GS] = &cpu->GS;
    cpu->tseg[6] = &segment_dummy;
    cpu->tseg[7] = &segment_dummy;

    memset(cpu, 0, sizeof(cpu));

    cpu->CurrentSegment = &cpu->DS;

    vomit_cpu_jump(cpu, 0xF000, 0x0000);

    vomit_cpu_set_flags(cpu, 0x0200 | vomit_cpu_static_flags(cpu));

    cpu->pit_counter = INSNS_PER_PIT_IRQ;
    cpu->state = CPU_ALIVE;

#ifdef VOMIT_PREFETCH_QUEUE
    cpu->pfq = 0;
#endif

    vomit_cpu_install_default_handlers(cpu);
}

static void vomit_cpu_set_handler(vomit_cpu_t *cpu, BYTE range_start, BYTE range_end, vomit_opcode_handler handler)
{
    for (int i = range_start; i <= range_end; ++i) {
        cpu->opcode_handler[i] = handler;
    }
}

void vomit_cpu_install_default_handlers(vomit_cpu_t *cpu)
{
    /* Install a default handler for unsupported instructions. */
    vomit_cpu_set_handler(cpu, 0x00, 0xFF, _UNSUPP           );

    vomit_cpu_set_handler(cpu, 0x00, 0x00, _ADD_RM8_reg8     );
    vomit_cpu_set_handler(cpu, 0x01, 0x01, _ADD_RM16_reg16   );
    vomit_cpu_set_handler(cpu, 0x02, 0x02, _ADD_reg8_RM8     );
    vomit_cpu_set_handler(cpu, 0x03, 0x03, _ADD_reg16_RM16   );
    vomit_cpu_set_handler(cpu, 0x04, 0x04, _ADD_AL_imm8      );
    vomit_cpu_set_handler(cpu, 0x05, 0x05, _ADD_AX_imm16     );
    vomit_cpu_set_handler(cpu, 0x06, 0x06, _PUSH_seg         );
    vomit_cpu_set_handler(cpu, 0x07, 0x07, _POP_seg          );
    vomit_cpu_set_handler(cpu, 0x08, 0x08, _OR_RM8_reg8      );
    vomit_cpu_set_handler(cpu, 0x09, 0x09, _OR_RM16_reg16    );
    vomit_cpu_set_handler(cpu, 0x0A, 0x0A, _OR_reg8_RM8      );
    vomit_cpu_set_handler(cpu, 0x0B, 0x0B, _OR_reg16_RM16    );
    vomit_cpu_set_handler(cpu, 0x0C, 0x0C, _OR_AL_imm8       );
    vomit_cpu_set_handler(cpu, 0x0D, 0x0D, _OR_AX_imm16      );
    vomit_cpu_set_handler(cpu, 0x0E, 0x0E, _PUSH_seg         );
    vomit_cpu_set_handler(cpu, 0x0F, 0x0F, _POP_CS           );
    vomit_cpu_set_handler(cpu, 0x10, 0x10, _ADC_RM8_reg8     );
    vomit_cpu_set_handler(cpu, 0x11, 0x11, _ADC_RM16_reg16   );
    vomit_cpu_set_handler(cpu, 0x12, 0x12, _ADC_reg8_RM8     );
    vomit_cpu_set_handler(cpu, 0x13, 0x13, _ADC_reg16_RM16   );
    vomit_cpu_set_handler(cpu, 0x14, 0x14, _ADC_AL_imm8      );
    vomit_cpu_set_handler(cpu, 0x15, 0x15, _ADC_AX_imm16     );
    vomit_cpu_set_handler(cpu, 0x16, 0x16, _PUSH_seg         );
    vomit_cpu_set_handler(cpu, 0x17, 0x17, _POP_seg          );
    vomit_cpu_set_handler(cpu, 0x18, 0x18, _SBB_RM8_reg8     );
    vomit_cpu_set_handler(cpu, 0x19, 0x19, _SBB_RM16_reg16   );
    vomit_cpu_set_handler(cpu, 0x1A, 0x1A, _SBB_reg8_RM8     );
    vomit_cpu_set_handler(cpu, 0x1B, 0x1B, _SBB_reg16_RM16   );
    vomit_cpu_set_handler(cpu, 0x1C, 0x1C, _SBB_AL_imm8      );
    vomit_cpu_set_handler(cpu, 0x1D, 0x1D, _SBB_AX_imm16     );
    vomit_cpu_set_handler(cpu, 0x1E, 0x1E, _PUSH_seg         );
    vomit_cpu_set_handler(cpu, 0x1F, 0x1F, _POP_seg          );
    vomit_cpu_set_handler(cpu, 0x20, 0x20, _AND_RM8_reg8     );
    vomit_cpu_set_handler(cpu, 0x21, 0x21, _AND_RM16_reg16   );
    vomit_cpu_set_handler(cpu, 0x22, 0x22, _AND_reg8_RM8     );
    vomit_cpu_set_handler(cpu, 0x23, 0x23, _AND_reg16_RM16   );
    vomit_cpu_set_handler(cpu, 0x24, 0x24, _AND_AL_imm8      );
    vomit_cpu_set_handler(cpu, 0x25, 0x25, _AND_AX_imm16     );
    vomit_cpu_set_handler(cpu, 0x26, 0x26, _ES               );
    vomit_cpu_set_handler(cpu, 0x27, 0x27, _DAA              );
    vomit_cpu_set_handler(cpu, 0x28, 0x28, _SUB_RM8_reg8     );
    vomit_cpu_set_handler(cpu, 0x29, 0x29, _SUB_RM16_reg16   );
    vomit_cpu_set_handler(cpu, 0x2A, 0x2A, _SUB_reg8_RM8     );
    vomit_cpu_set_handler(cpu, 0x2B, 0x2B, _SUB_reg16_RM16   );
    vomit_cpu_set_handler(cpu, 0x2C, 0x2C, _SUB_AL_imm8      );
    vomit_cpu_set_handler(cpu, 0x2D, 0x2D, _SUB_AX_imm16     );
    vomit_cpu_set_handler(cpu, 0x2E, 0x2E, _CS               );
    vomit_cpu_set_handler(cpu, 0x2F, 0x2F, _DAS              );
    vomit_cpu_set_handler(cpu, 0x30, 0x30, _XOR_RM8_reg8     );
    vomit_cpu_set_handler(cpu, 0x31, 0x31, _XOR_RM16_reg16   );
    vomit_cpu_set_handler(cpu, 0x32, 0x32, _XOR_reg8_RM8     );
    vomit_cpu_set_handler(cpu, 0x33, 0x33, _XOR_reg16_RM16   );
    vomit_cpu_set_handler(cpu, 0x34, 0x34, _XOR_AL_imm8      );
    vomit_cpu_set_handler(cpu, 0x35, 0x35, _XOR_AX_imm16     );
    vomit_cpu_set_handler(cpu, 0x36, 0x36, _SS               );
    vomit_cpu_set_handler(cpu, 0x37, 0x37, _AAA              );
    vomit_cpu_set_handler(cpu, 0x38, 0x38, _CMP_RM8_reg8     );
    vomit_cpu_set_handler(cpu, 0x39, 0x39, _CMP_RM16_reg16   );
    vomit_cpu_set_handler(cpu, 0x3A, 0x3A, _CMP_reg8_RM8     );
    vomit_cpu_set_handler(cpu, 0x3B, 0x3B, _CMP_reg16_RM16   );
    vomit_cpu_set_handler(cpu, 0x3C, 0x3C, _CMP_AL_imm8      );
    vomit_cpu_set_handler(cpu, 0x3D, 0x3D, _CMP_AX_imm16     );
    vomit_cpu_set_handler(cpu, 0x3E, 0x3E, _DS               );
    vomit_cpu_set_handler(cpu, 0x3F, 0x3F, _AAS              );
    vomit_cpu_set_handler(cpu, 0x40, 0x47, _INC_reg16        );
    vomit_cpu_set_handler(cpu, 0x48, 0x4F, _DEC_reg16        );
    vomit_cpu_set_handler(cpu, 0x50, 0x57, _PUSH_reg16       );
    vomit_cpu_set_handler(cpu, 0x58, 0x5F, _POP_reg16        );
    vomit_cpu_set_handler(cpu, 0x6E, 0x6E, _OUTSB            );
    vomit_cpu_set_handler(cpu, 0x6F, 0x6F, _OUTSW            );
    vomit_cpu_set_handler(cpu, 0x70, 0x70, _JO_imm8          );
    vomit_cpu_set_handler(cpu, 0x71, 0x71, _JNO_imm8         );
    vomit_cpu_set_handler(cpu, 0x72, 0x72, _JC_imm8          );
    vomit_cpu_set_handler(cpu, 0x73, 0x73, _JNC_imm8         );
    vomit_cpu_set_handler(cpu, 0x74, 0x74, _JZ_imm8          );
    vomit_cpu_set_handler(cpu, 0x75, 0x75, _JNZ_imm8         );
    vomit_cpu_set_handler(cpu, 0x76, 0x76, _JNA_imm8         );
    vomit_cpu_set_handler(cpu, 0x77, 0x77, _JA_imm8          );
    vomit_cpu_set_handler(cpu, 0x78, 0x78, _JS_imm8          );
    vomit_cpu_set_handler(cpu, 0x79, 0x79, _JNS_imm8         );
    vomit_cpu_set_handler(cpu, 0x7A, 0x7A, _JP_imm8          );
    vomit_cpu_set_handler(cpu, 0x7B, 0x7B, _JNP_imm8         );
    vomit_cpu_set_handler(cpu, 0x7C, 0x7C, _JL_imm8          );
    vomit_cpu_set_handler(cpu, 0x7D, 0x7D, _JNL_imm8         );
    vomit_cpu_set_handler(cpu, 0x7E, 0x7E, _JNG_imm8         );
    vomit_cpu_set_handler(cpu, 0x7F, 0x7F, _JG_imm8          );
    vomit_cpu_set_handler(cpu, 0x80, 0x80, _wrap_0x80        );
    vomit_cpu_set_handler(cpu, 0x81, 0x81, _wrap_0x81        );
    vomit_cpu_set_handler(cpu, 0x83, 0x83, _wrap_0x83        );
    vomit_cpu_set_handler(cpu, 0x84, 0x84, _TEST_RM8_reg8    );
    vomit_cpu_set_handler(cpu, 0x85, 0x85, _TEST_RM16_reg16  );
    vomit_cpu_set_handler(cpu, 0x86, 0x86, _XCHG_reg8_RM8    );
    vomit_cpu_set_handler(cpu, 0x87, 0x87, _XCHG_reg16_RM16  );
    vomit_cpu_set_handler(cpu, 0x88, 0x88, _MOV_RM8_reg8     );
    vomit_cpu_set_handler(cpu, 0x89, 0x89, _MOV_RM16_reg16   );
    vomit_cpu_set_handler(cpu, 0x8A, 0x8A, _MOV_reg8_RM8     );
    vomit_cpu_set_handler(cpu, 0x8B, 0x8B, _MOV_reg16_RM16   );
    vomit_cpu_set_handler(cpu, 0x8C, 0x8C, _MOV_RM16_seg     );
    vomit_cpu_set_handler(cpu, 0x8D, 0x8D, _LEA_reg16_mem16  );
    vomit_cpu_set_handler(cpu, 0x8E, 0x8E, _MOV_seg_RM16     );
    vomit_cpu_set_handler(cpu, 0x8F, 0x8F, _wrap_0x8F        );
    vomit_cpu_set_handler(cpu, 0x90, 0x90, _NOP              );
    vomit_cpu_set_handler(cpu, 0x91, 0x97, _XCHG_AX_reg16    );
    vomit_cpu_set_handler(cpu, 0x98, 0x98, _CBW              );
    vomit_cpu_set_handler(cpu, 0x99, 0x99, _CWD              );
    vomit_cpu_set_handler(cpu, 0x9A, 0x9A, _CALL_imm16_imm16 );
    vomit_cpu_set_handler(cpu, 0x9C, 0x9C, _PUSHF            );
    vomit_cpu_set_handler(cpu, 0x9D, 0x9D, _POPF             );
    vomit_cpu_set_handler(cpu, 0x9E, 0x9E, _SAHF             );
    vomit_cpu_set_handler(cpu, 0x9F, 0x9F, _LAHF             );
    vomit_cpu_set_handler(cpu, 0xA0, 0xA0, _MOV_AL_moff8     );
    vomit_cpu_set_handler(cpu, 0xA1, 0xA1, _MOV_AX_moff16    );
    vomit_cpu_set_handler(cpu, 0xA2, 0xA2, _MOV_moff8_AL     );
    vomit_cpu_set_handler(cpu, 0xA3, 0xA3, _MOV_moff16_AX    );
    vomit_cpu_set_handler(cpu, 0xA4, 0xA4, _MOVSB            );
    vomit_cpu_set_handler(cpu, 0xA5, 0xA5, _MOVSW            );
    vomit_cpu_set_handler(cpu, 0xA6, 0xA6, _CMPSB            );
    vomit_cpu_set_handler(cpu, 0xA7, 0xA7, _CMPSW            );
    vomit_cpu_set_handler(cpu, 0xA8, 0xA8, _TEST_AL_imm8     );
    vomit_cpu_set_handler(cpu, 0xA9, 0xA9, _TEST_AX_imm16    );
    vomit_cpu_set_handler(cpu, 0xAA, 0xAA, _STOSB            );
    vomit_cpu_set_handler(cpu, 0xAB, 0xAB, _STOSW            );
    vomit_cpu_set_handler(cpu, 0xAC, 0xAC, _LODSB            );
    vomit_cpu_set_handler(cpu, 0xAD, 0xAD, _LODSW            );
    vomit_cpu_set_handler(cpu, 0xAE, 0xAE, _SCASB            );
    vomit_cpu_set_handler(cpu, 0xAF, 0xAF, _SCASW            );
    vomit_cpu_set_handler(cpu, 0xB0, 0xB7, _MOV_reg8_imm8    );
    vomit_cpu_set_handler(cpu, 0xB8, 0xBF, _MOV_reg16_imm16  );
    vomit_cpu_set_handler(cpu, 0xC2, 0xC2, _RET_imm16        );
    vomit_cpu_set_handler(cpu, 0xC3, 0xC3, _RET              );
    vomit_cpu_set_handler(cpu, 0xC4, 0xC4, _LES_reg16_mem16  );
    vomit_cpu_set_handler(cpu, 0xC5, 0xC5, _LDS_reg16_mem16  );
    vomit_cpu_set_handler(cpu, 0xC6, 0xC6, _MOV_RM8_imm8     );
    vomit_cpu_set_handler(cpu, 0xC7, 0xC7, _MOV_RM16_imm16   );
    vomit_cpu_set_handler(cpu, 0xCA, 0xCA, _RETF_imm16       );
    vomit_cpu_set_handler(cpu, 0xCB, 0xCB, _RETF             );
    vomit_cpu_set_handler(cpu, 0xCC, 0xCC, _INT3             );
    vomit_cpu_set_handler(cpu, 0xCD, 0xCD, _INT_imm8         );
    vomit_cpu_set_handler(cpu, 0xCE, 0xCE, _INTO             );
    vomit_cpu_set_handler(cpu, 0xCF, 0xCF, _IRET             );
    vomit_cpu_set_handler(cpu, 0xD0, 0xD0, _wrap_0xD0        );
    vomit_cpu_set_handler(cpu, 0xD1, 0xD1, _wrap_0xD1        );
    vomit_cpu_set_handler(cpu, 0xD2, 0xD2, _wrap_0xD2        );
    vomit_cpu_set_handler(cpu, 0xD3, 0xD3, _wrap_0xD3        );
    vomit_cpu_set_handler(cpu, 0xD4, 0xD4, _AAM              );
    vomit_cpu_set_handler(cpu, 0xD5, 0xD5, _AAD              );
    vomit_cpu_set_handler(cpu, 0xD6, 0xD6, _SALC             );
    vomit_cpu_set_handler(cpu, 0xD7, 0xD7, _XLAT             );
    vomit_cpu_set_handler(cpu, 0xE0, 0xE0, _LOOPNE_imm8      );
    vomit_cpu_set_handler(cpu, 0xE1, 0xE1, _LOOPE_imm8       );
    vomit_cpu_set_handler(cpu, 0xE2, 0xE2, _LOOP_imm8        );
    vomit_cpu_set_handler(cpu, 0xE3, 0xE3, _JCXZ_imm8        );
    vomit_cpu_set_handler(cpu, 0xE4, 0xE4, _IN_AL_imm8       );
    vomit_cpu_set_handler(cpu, 0xE5, 0xE5, _IN_AX_imm8       );
    vomit_cpu_set_handler(cpu, 0xE6, 0xE6, _OUT_imm8_AL      );
    vomit_cpu_set_handler(cpu, 0xE7, 0xE7, _OUT_imm8_AX      );
    vomit_cpu_set_handler(cpu, 0xE8, 0xE8, _CALL_imm16       );
    vomit_cpu_set_handler(cpu, 0xE9, 0xE9, _JMP_imm16        );
    vomit_cpu_set_handler(cpu, 0xEA, 0xEA, _JMP_imm16_imm16  );
    vomit_cpu_set_handler(cpu, 0xEB, 0xEB, _JMP_short_imm8   );
    vomit_cpu_set_handler(cpu, 0xEC, 0xEC, _IN_AL_DX         );
    vomit_cpu_set_handler(cpu, 0xED, 0xED, _IN_AX_DX         );
    vomit_cpu_set_handler(cpu, 0xEE, 0xEE, _OUT_DX_AL        );
    vomit_cpu_set_handler(cpu, 0xEF, 0xEF, _OUT_DX_AX        );
    vomit_cpu_set_handler(cpu, 0xF2, 0xF2, _REPNE            );
    vomit_cpu_set_handler(cpu, 0xF3, 0xF3, _REP              );
    vomit_cpu_set_handler(cpu, 0xF4, 0xF4, _HLT              );
    vomit_cpu_set_handler(cpu, 0xF5, 0xF5, _CMC              );
    vomit_cpu_set_handler(cpu, 0xF6, 0xF6, _wrap_0xF6        );
    vomit_cpu_set_handler(cpu, 0xF7, 0xF7, _wrap_0xF7        );
    vomit_cpu_set_handler(cpu, 0xF8, 0xF8, _CLC              );
    vomit_cpu_set_handler(cpu, 0xF9, 0xF9, _STC              );
    vomit_cpu_set_handler(cpu, 0xFA, 0xFA, _CLI              );
    vomit_cpu_set_handler(cpu, 0xFB, 0xFB, _STI              );
    vomit_cpu_set_handler(cpu, 0xFC, 0xFC, _CLD              );
    vomit_cpu_set_handler(cpu, 0xFD, 0xFD, _STD              );
    vomit_cpu_set_handler(cpu, 0xFE, 0xFE, _wrap_0xFE        );
    vomit_cpu_set_handler(cpu, 0xFF, 0xFF, _wrap_0xFF        );

    vomit_cpu_set_handler(cpu, 0xC0, 0xC0, _wrap_0xC0        );
    vomit_cpu_set_handler(cpu, 0xC1, 0xC1, _wrap_0xC1        );

    if( cpu->type >= INTEL_80186 )
    {
        vomit_cpu_set_handler(cpu, 0x0F, 0x0F, _wrap_0x0F    );
        vomit_cpu_set_handler(cpu, 0x60, 0x60, _PUSHA        );
        vomit_cpu_set_handler(cpu, 0x61, 0x61, _POPA         );
        vomit_cpu_set_handler(cpu, 0x62, 0x62, _BOUND        );
        vomit_cpu_set_handler(cpu, 0x68, 0x68, _PUSH_imm16   );
        vomit_cpu_set_handler(cpu, 0x6A, 0x6A, _PUSH_imm8    );
        vomit_cpu_set_handler(cpu, 0x6B, 0x6B, _IMUL_reg16_RM16_imm8 );
        vomit_cpu_set_handler(cpu, 0xC0, 0xC0, _wrap_0xC0    );
        vomit_cpu_set_handler(cpu, 0xC1, 0xC1, _wrap_0xC1    );
        vomit_cpu_set_handler(cpu, 0xC8, 0xC8, _ENTER        );
        vomit_cpu_set_handler(cpu, 0xC9, 0xC9, _LEAVE        );
    }

    /* Some cheap solutions. */
    vomit_cpu_set_handler(cpu, 0x66, 0x66, _OpOverride       );

    /* There is no FPU yet. */
    vomit_cpu_set_handler(cpu, 0x9B, 0x9B, _ESCAPE           );
    vomit_cpu_set_handler(cpu, 0xD8, 0xDF, _ESCAPE           );
}

void vomit_cpu_kill(vomit_cpu_t *cpu)
{
#ifdef VOMIT_PREFETCH_QUEUE
    free(cpu->pfq);
    cpu->pfq = 0;
#endif

    free(cpu->memory);
    cpu->memory = 0;
    cpu->code_memory = 0;
}

void vomit_cpu_main(vomit_cpu_t *cpu)
{
#ifdef VOMIT_PREFETCH_QUEUE
    vomit_cpu_pfq_flush(cpu);
#endif

    for (;;) {
        /* HACK: This can be set by an external force to make us break out. */
        if( g_vomit_exit_main_loop )
            return;

        /* TODO: Be more clever with this, it's mostly a waste of time. */
        cpu->base_CS = cpu->CS;
        cpu->base_IP = cpu->IP;

        /* TODO: Refactor this to spin in a separate mainloop when halted. */
        if (cpu->state == CPU_HALTED) {
            if (!cpu->IF) {
                vlog(VM_ALERT, "%04X:%04X: Halted with IF=0", cpu->base_CS, cpu->base_IP);
                return;
            }

            if (g_pic_pending_requests) {
                pic_service_irq(cpu);
            }
            continue;
        }

#ifdef VOMIT_TRACE
        if (options.trace) {
            dump_disasm(cpu->base_CS, cpu->base_IP);
            dump_regs(cpu);
        }
#endif

        cpu->opcode = vomit_cpu_pfq_getbyte(cpu);
        cpu->opcode_handler[cpu->opcode](cpu);

        if (cpu->TF) {
            /* The Trap Flag is set, so we'll execute one instruction and
             * call ISR 1 as soon as it's finished.
             *
             * This is used by tools like DEBUG to implement step-by-step
             * execution :-) */
            vomit_cpu_isr_call(cpu, 1);

            /* NOTE: vomit_cpu_isr_call() just set IF=0. */
        }

        if (!--cpu->pit_counter) {
            cpu->pit_counter = INSNS_PER_PIT_IRQ;

            /* Raise the timer IRQ. This is ugly, I know. */
            irq(0);
        }

        if (g_pic_pending_requests && cpu->IF) {
            pic_service_irq(cpu);
        }
    }
}

#ifdef VOMIT_PREFETCH_QUEUE
/* vomit_cpu_pfq_getbyte() is a macro if !VOMIT_PREFETCH_QUEUE */

BYTE vomit_cpu_pfq_getbyte(vomit_cpu_t *cpu)
{
    BYTE b = cpu->pfq[cpu->pfq_current];
    cpu_pfq[cpu_pfq_current] = cpu->code_memory[cpu->IP + cpu->pfq_size];
    if (++cpu_pfq_current == cpu->pfq_size)
        cpu->pfq_current = 0;
    ++cpu->IP;
    return b;
}
#endif

WORD vomit_cpu_pfq_getword(vomit_cpu_t *cpu)
{
#ifdef VOMIT_PREFETCH_QUEUE
    WORD w = cpu->pfq[cpu->pfq_current];
    cpu->pfq[cpu->pfq_current] = code_memory[cpu->IP + cpu->pfq_size];
    if (++cpu->pfq_current == cpu->pfq_size)
        cpu_pfq_current = 0;
    w |= cpu->pfq[cpu->pfq_current] << 8;
    ++cpu->IP;
    cpu->pfq[cpu->pfq_current] = code_memory[cpu->IP + cpu->pfq_size];
    if (++cpu->pfq_current == cpu->pfq_size)
        cpu_pfq_current = 0;
    ++cpu->IP;
    return w;
#else
    WORD w = *(word *)(&cpu->code_memory[cpu->IP]);
#ifdef VOMIT_BIG_ENDIAN
    w = V_BYTESWAP(w);
#endif
    cpu->IP += 2;
    return w;
#endif
}

#ifdef VOMIT_PREFETCH_QUEUE
void vomit_cpu_pfq_flush()
{
    /* Flush the prefetch queue. MUST be done after all jumps/calls. */
    cpu->pfq_current = 0;
    for (int i = 0; i < cpu->pfq_size; ++i)
        cpu->pfq[i] = code_memory[cpu->IP + i];
}
#endif

void vomit_cpu_jump_relative8(vomit_cpu_t *cpu, SIGNED_BYTE displacement)
{
    cpu->IP += displacement;
    vomit_cpu_pfq_flush();
}

void vomit_cpu_jump_relative16(vomit_cpu_t *cpu, SIGNED_WORD displacement)
{
    cpu->IP += displacement;
    vomit_cpu_pfq_flush();
}

void vomit_cpu_jump_absolute16(vomit_cpu_t *cpu, WORD address)
{
    cpu->IP = address;
    vomit_cpu_pfq_flush(cpu);
}

void vomit_cpu_jump(vomit_cpu_t *cpu, WORD segment, WORD offset)
{
    /* Jump to specified location. */
    cpu->CS = segment;
    cpu->IP = offset;

    cpu->code_memory = cpu->memory + (segment << 4);
    vomit_cpu_pfq_flush(cpu);
}

void vomit_cpu_set_flags(vomit_cpu_t *cpu, WORD flags)
{
    cpu->CF = (flags & 0x0001) != 0;
    cpu->PF = (flags & 0x0004) != 0;
    cpu->AF = (flags & 0x0010) != 0;
    cpu->ZF = (flags & 0x0040) != 0;
    cpu->SF = (flags & 0x0080) != 0;
    cpu->TF = (flags & 0x0100) != 0;
    cpu->IF = (flags & 0x0200) != 0;
    cpu->DF = (flags & 0x0400) != 0;
    cpu->OF = (flags & 0x0800) != 0;
}

WORD vomit_cpu_get_flags(vomit_cpu_t *cpu)
{
    return cpu->CF | (cpu->PF << 2) | (cpu->AF << 4) | (cpu->ZF << 6) | (cpu->SF << 7) | (cpu->TF << 8) | (cpu->IF << 9) | (cpu->DF << 10) | (cpu->OF << 11) | vomit_cpu_static_flags(cpu);
}

void vomit_cpu_set_interrupt(vomit_cpu_t *cpu, BYTE isr, WORD segment, WORD offset)
{
    vomit_cpu_memory_write16(cpu, 0x0000, (isr << 2), offset);
    vomit_cpu_memory_write16(cpu, 0x0000, (isr << 2) + 2, segment);
}

bool vomit_cpu_evaluate(vomit_cpu_t *cpu, BYTE condition_code)
{
    //FIXME: uncomment
    //V_ASSERT(condition_code <= 0xF);

    switch (condition_code) {
    case  0: return cpu->OF;                          /* O          */
    case  1: return !cpu->OF;                         /* NO         */
    case  2: return cpu->CF;                          /* B, C, NAE  */
    case  3: return !cpu->CF;                         /* NB, NC, AE */
    case  4: return cpu->ZF;                          /* E, Z       */
    case  5: return !cpu->ZF;                         /* NE, NZ     */
    case  6: return (cpu->CF | cpu->ZF);              /* BE, NA     */
    case  7: return !(cpu->CF | cpu->ZF);             /* NBE, A     */
    case  8: return cpu->SF;                          /* S          */
    case  9: return !cpu->SF;                         /* NS         */
    case 10: return cpu->PF;                          /* P, PE      */
    case 11: return !cpu->PF;                         /* NP, PO     */
    case 12: return cpu->SF ^ cpu->OF;                /* L, NGE     */
    case 13: return !(cpu->SF ^ cpu->OF);             /* NL, GE     */
    case 14: return (cpu->SF ^ cpu->OF) | cpu->ZF;    /* LE, NG     */
    case 15: return !((cpu->SF ^ cpu->OF) | cpu->ZF); /* NLE, G     */
    }
    return 0;
}

void _WAIT(vomit_cpu_t *cpu)
{
    /* XXX: Do nothing? */
    vlog(VM_ALERT, "%04X:%04X: WAIT", cpu->base_CS, cpu->base_IP);
}

void _UNSUPP(vomit_cpu_t *cpu)
{
    /* We've come across an unsupported instruction, log it,
     * then vector to the "illegal instruction" ISR. */
    vlog(VM_ALERT, "%04X:%04X: Unsupported opcode %02X", cpu->base_CS, cpu->base_IP, cpu->opcode);
    dump_all();
    vomit_cpu_isr_call(cpu, 6);
}
