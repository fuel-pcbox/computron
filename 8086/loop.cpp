/* 8086/loop.cpp
 * Loop instructions
 *
 *
 */

#include "vomit.h"
#include "debug.h"

void _LOOP_imm8(vomit_cpu_t *cpu)
{
    SIGNED_BYTE disp = vomit_cpu_pfq_getbyte(cpu);
    --cpu->regs.W.CX;
    if (cpu->regs.W.CX)
        vomit_cpu_jump_relative8(cpu, disp);
}

void _LOOPE_imm8(vomit_cpu_t *cpu)
{
    SIGNED_BYTE disp = vomit_cpu_pfq_getbyte(cpu);
    --cpu->regs.W.CX;
    if (cpu->regs.W.CX && cpu->ZF)
        vomit_cpu_jump_relative8(cpu, disp);
}

void _LOOPNE_imm8(vomit_cpu_t *cpu)
{
    SIGNED_BYTE disp = vomit_cpu_pfq_getbyte(cpu);
    --cpu->regs.W.CX;
    if (cpu->regs.W.CX && !cpu->ZF)
        vomit_cpu_jump_relative8(cpu, disp);
}

#define DO_REP(func) for (; cpu->regs.W.CX; --cpu->regs.W.CX) { func(cpu); }
#define DO_REPZ(func) for (cpu->ZF = should_equal; cpu->regs.W.CX && (cpu->ZF == should_equal); --cpu->regs.W.CX) { func(cpu); }

static void __rep(vomit_cpu_t *cpu, byte opcode, bool should_equal)
{
    switch(opcode) {
    case 0x26: SET_SEGMENT_PREFIX(cpu, ES); break;
    case 0x2E: SET_SEGMENT_PREFIX(cpu, CS); break;
    case 0x36: SET_SEGMENT_PREFIX(cpu, SS); break;
    case 0x3E: SET_SEGMENT_PREFIX(cpu, DS); break;

    case 0xA4: DO_REP(_MOVSB); return;
    case 0xA5: DO_REP(_MOVSW); return;
    case 0xAA: DO_REP(_STOSB); return;
    case 0xAB: DO_REP(_STOSW); return;
    case 0xAC: DO_REP(_LODSB); return;
    case 0xAD: DO_REP(_LODSW); return;

    case 0xA6: DO_REPZ(_CMPSB); return;
    case 0xA7: DO_REPZ(_CMPSW); return;
    case 0xAE: DO_REPZ(_SCASB); return;
    case 0xAF: DO_REPZ(_SCASW); return;

    default:
        vlog(VM_ALERT, "SUSPICIOUS: Opcode %02X used with REP* prefix", opcode);
        cpu->opcode_handler[opcode](cpu);
        return;
    }

	/* Recurse if this opcode was a segment prefix. */
	/* FIXME: Infinite recursion IS possible here. */
	__rep(cpu, vomit_cpu_pfq_getbyte(cpu), should_equal);
}

void _REP(vomit_cpu_t *cpu)
{
	__rep(cpu, vomit_cpu_pfq_getbyte(cpu), true);
	RESET_SEGMENT_PREFIX(cpu);
}

void _REPNE(vomit_cpu_t *cpu)
{
	__rep(cpu, vomit_cpu_pfq_getbyte(cpu), false);
	RESET_SEGMENT_PREFIX(cpu);
}
