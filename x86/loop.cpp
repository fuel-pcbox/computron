// x86/loop.cpp
// Loop & repetition instructions

#include "vcpu.h"
#include "debug.h"

void _LOOP_imm8(VCpu* cpu)
{
    SIGNED_BYTE displacement = cpu->fetchOpcodeByte();
    --cpu->regs.W.CX;
    if (cpu->regs.W.CX)
        cpu->jumpRelative8(displacement);
}

void _LOOPE_imm8(VCpu* cpu)
{
    SIGNED_BYTE displacement = cpu->fetchOpcodeByte();
    --cpu->regs.W.CX;
    if (cpu->regs.W.CX && cpu->getZF())
        cpu->jumpRelative8(displacement);
}

void _LOOPNE_imm8(VCpu* cpu)
{
    SIGNED_BYTE displacement = cpu->fetchOpcodeByte();
    --cpu->regs.W.CX;
    if (cpu->regs.W.CX && !cpu->getZF())
        cpu->jumpRelative8(displacement);
}

#define CALL_HANDLER(handler16, handler32) if (cpu->o16()) { handler16(cpu); } else { handler32(cpu); }
#define DO_REP_NEW(handler16, handler32) for (; cpu->regs.W.CX; --cpu->regs.W.CX) { CALL_HANDLER(handler16, handler32); }
#define DO_REP(func) for (; cpu->regs.W.CX; --cpu->regs.W.CX) { func(cpu); }
#define DO_REPZ(func) for (cpu->setZF(should_equal); cpu->regs.W.CX && (cpu->getZF() == should_equal); --cpu->regs.W.CX) { func(cpu); }

static void __rep(VCpu* cpu, BYTE opcode, bool should_equal)
{
    switch(opcode) {
    case 0x26: cpu->setSegmentPrefix(cpu->getES()); break;
    case 0x2E: cpu->setSegmentPrefix(cpu->getCS()); break;
    case 0x36: cpu->setSegmentPrefix(cpu->getSS()); break;
    case 0x3E: cpu->setSegmentPrefix(cpu->getDS()); break;

    case 0x66: {
        cpu->m_operationSize32 = !cpu->m_operationSize32;
        BYTE op = cpu->fetchOpcodeByte();
	    __rep(cpu, op, should_equal);
        cpu->m_operationSize32 = !cpu->m_operationSize32;
        return;
    }

    case 0x6E: DO_REP(_OUTSB); return;
    case 0x6F: DO_REP(_OUTSW); return;

    case 0xA4: DO_REP(_MOVSB); return;
    case 0xA5: DO_REP_NEW(_MOVSW, _MOVSD); return;
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

    // Recurse if this opcode was a segment prefix.
    // FIXME: Infinite recursion IS possible here.
    __rep(cpu, cpu->fetchOpcodeByte(), should_equal);
}

void _REP(VCpu* cpu)
{
    __rep(cpu, cpu->fetchOpcodeByte(), true);
    cpu->resetSegmentPrefix();
}

void _REPNE(VCpu* cpu)
{
    __rep(cpu, cpu->fetchOpcodeByte(), false);
    cpu->resetSegmentPrefix();
}
