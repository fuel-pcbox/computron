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
        cpu->decode(opcode);
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
