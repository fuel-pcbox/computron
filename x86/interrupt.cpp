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
#include "vcpu.h"
#include "debug.h"
#include "debugger.h"

void VCpu::_INT_imm8()
{
    BYTE isr = fetchOpcodeByte();
    jumpToInterruptHandler(isr);
}

void VCpu::_INT3()
{
    jumpToInterruptHandler(3);
}

void VCpu::_INTO()
{
    /* XXX: I've never seen this used, so it's probably good to log it. */
    vlog(LogAlert, "INTO used, can you believe it?");

    if (getOF())
        jumpToInterruptHandler(4);
}

void VCpu::_IRET()
{
    VM_ASSERT(!x32());
    WORD nip = pop();
    WORD ncs = pop();
    jump16(ncs, nip);
    setFlags(pop());
}

void VCpu::jumpToInterruptHandler(int isr, bool requestedByPIC)
{
    VM_ASSERT(!x32());
#ifdef VOMIT_DEBUG
    if (options.trapint)
        vlog(LogCPU, "Interrupt %02X,%02X trapped%s", isr, this->regs.B.AH, requestedByPIC ? " (from PIC)" : "");

    if (isr == 0x06) {
        vlog(LogCPU, "Invalid opcode trap (%02X)", *(codeMemory() + this->getBaseEIP()));
        dumpAll();
        debugger()->enter();
        //return;
    }
#endif

    if (o16()) {
        push(getFlags());
        setIF(0);
        setTF(0);
        push(getCS());
        push(getIP());

        // FIXME: should use PE-safe reads
        WORD segment = (m_memory[isr * 4 + 3] << 8) | m_memory[isr * 4 + 2];
        WORD offset = (m_memory[isr * 4 + 1] << 8) | m_memory[isr * 4];

        jump16(segment, offset);
        return;
    }
    push(getEFlags());
    setIF(0);
    setTF(0);
    push(getCS());
    push(getEIP());

    FarPointer iv = getInterruptVector32(isr);
    jump32(iv.segment, iv.offset);
}

FarPointer VCpu::getInterruptVector16(int isr)
{
    vlog(LogAlert, "getInterruptVector16(%d)", isr);
    vomit_exit(1);
    return { 0, 0 };
}

FarPointer VCpu::getInterruptVector32(int isr)
{
    vlog(LogAlert, "getInterruptVector32(%d)", isr);
    return { 0, 0 };
}
