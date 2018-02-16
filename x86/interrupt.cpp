/*
 * Copyright (C) 2003-2018 Andreas Kling <awesomekling@gmail.com>
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
#include "CPU.h"
#include "debug.h"
#include "debugger.h"

void CPU::_INT_imm8(Instruction& insn)
{
    jumpToInterruptHandler(insn.imm8());
}

void CPU::_INT3(Instruction&)
{
    jumpToInterruptHandler(3);
}

void CPU::_INTO(Instruction&)
{
    /* XXX: I've never seen this used, so it's probably good to log it. */
    vlog(LogAlert, "INTO used, can you believe it?");

    if (getOF())
        jumpToInterruptHandler(4);
}

void CPU::_IRET(Instruction&)
{
    if (getPE()) {
        if (getNT()) {
            VM_ASSERT(!getVM());
            auto* tss = currentTSS();
            VM_ASSERT(tss);
            vlog(LogCPU, "IRET with NT=1 switching tasks. Inner TSS @ %08X -> Outer TSS sel %04X...", TR.base, tss->backlink);
            taskSwitch(tss->backlink);
            return;
        }

    }
    if (o16()) {
        WORD nip = pop();
        WORD ncs = pop();
        jump16(ncs, nip);
        setFlags(pop());
    } else {
        DWORD nip = pop32();
        WORD ncs = pop32();
        jump32(ncs, nip);
        setFlags(pop32());
    }
}

void CPU::jumpToInterruptHandler(int isr, bool requestedByPIC)
{
    bool isTrap = false;
    FarPointer vector;

    if (getPE()) {
        auto gate = getInterruptGate(isr);

        vector.segment = gate.selector();
        vector.offset = gate.offset();

        vlog(LogCPU, "PE=1 Interrupt %02x trapped%s, type: %s (%1x), %04x:%08x", isr, requestedByPIC ? " (from PIC)" : "", gate.typeName(), gate.type(), vector.segment, vector.offset);

        switch (gate.type()) {
        case 0x7: // 80286 Trap Gate (16-bit)
        case 0xf: // 80386 Trap Gate (32-bit)
            isTrap = true;
            break;
        case 0x6: // 80286 Interrupt Gate (16-bit)
        case 0xe: // 80386 Interrupt Gate (32-bit)
            break;
        default:
            VM_ASSERT(false);
        }
    } else {
        // FIXME: should use PE-safe reads
        vector.segment = (m_memory[isr * 4 + 3] << 8) | m_memory[isr * 4 + 2];
        vector.offset = (m_memory[isr * 4 + 1] << 8) | m_memory[isr * 4];
    }

#ifdef VOMIT_DEBUG
    if (options.trapint)
        vlog(LogCPU, "Interrupt %02X,%02X trapped%s", isr, this->regs.B.AH, requestedByPIC ? " (from PIC)" : "");

    if (isr == 0x06) {
        vlog(LogCPU, "Invalid opcode trap (%02X)", *(codeMemory() + this->getBaseEIP()));
        dumpAll();
        debugger().enter();
        //return;
    }
#endif

    if (o16()) {
        push(getFlags());
        if (!isTrap)
            setIF(0);
        setTF(0);
        push(getCS());
        push(getIP());

        jump16(vector.segment, vector.offset);
        return;
    }
    push32(getFlags());
    if (!isTrap)
        setIF(0);
    setTF(0);
    push32(getCS());
    push32(getEIP());

    jump32(vector.segment, vector.offset);
}

FarPointer CPU::getInterruptVector16(int isr)
{
    vlog(LogAlert, "getInterruptVector16(%d)", isr);
    hard_exit(1);
    return { 0, 0 };
}

FarPointer CPU::getInterruptVector32(int isr)
{
    vlog(LogAlert, "getInterruptVector32(%d)", isr);
    return { 0, 0 };
}
