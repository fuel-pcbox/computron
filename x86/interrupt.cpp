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

#include "Common.h"
#include "CPU.h"
#include "debug.h"
#include "debugger.h"
#include "Tasking.h"

void CPU::_INT_imm8(Instruction& insn)
{
    jumpToInterruptHandler(insn.imm8(), InterruptSource::Internal);
}

void CPU::_INT3(Instruction&)
{
    jumpToInterruptHandler(3, InterruptSource::Internal);
}

void CPU::_INTO(Instruction&)
{
    /* XXX: I've never seen this used, so it's probably good to log it. */
    vlog(LogAlert, "INTO used, can you believe it?");

    if (getOF())
        jumpToInterruptHandler(4, InterruptSource::Internal);
}

void CPU::_IRET(Instruction&)
{
    if (getPE()) {
        if (getNT()) {
            ASSERT(!getVM());
            auto tss = currentTSS();
#ifdef DEBUG_TASK_SWITCH
            vlog(LogCPU, "IRET with NT=1 switching tasks. Inner TSS @ %08X -> Outer TSS sel %04X...", TR.base, tss.getBacklink());
#endif
            taskSwitch(tss.getBacklink(), JumpType::IRET);
            return;
        }
    }

    if (o16()) {
        WORD nip = pop16();
        WORD ncs = pop16();
        WORD flags = pop16();
#ifdef DEBUG_JUMPS
        vlog(LogCPU, "Popped 16-bit cs:ip!flags %04x:%04x!%04x @stack{%04x:%08x}", ncs, nip, flags, getSS(), getESP());
#endif
        jump16(ncs, nip, JumpType::IRET);
        setEFlagsRespectfully(flags);
    } else {
        DWORD nip = pop32();
        WORD ncs = pop32();
        DWORD flags = pop32();
#ifdef DEBUG_JUMPS
        vlog(LogCPU, "Popped 32-bit cs:eip!flags %04x:%08x!%08x @stack{%04x:%08x}", ncs, nip, flags, getSS(), getESP());
#endif
        jump32(ncs, nip, JumpType::IRET);
        setEFlagsRespectfully(flags);
    }
}

static WORD makeErrorCode(WORD num, bool idt, CPU::InterruptSource source)
{
    if (idt)
        return (num << 3) | 2 | (WORD)source;
    return num & 0xfc;
}

void CPU::interruptToTaskGate(int, InterruptSource source, std::optional<WORD> errorCode, Gate& gate)
{
    Descriptor descriptor = getDescriptor(gate.selector());
    if (!descriptor.isGlobal()) {
        throw GeneralProtectionFault(makeErrorCode(gate.selector(), 0, source), "Interrupt to task gate referencing local descriptor");
    }
    if (!descriptor.isTSS()) {
        throw GeneralProtectionFault(makeErrorCode(gate.selector(), 0, source), "Interrupt to task gate referencing non-TSS descriptor");
    }
    auto& tssDescriptor = descriptor.asTSSDescriptor();
    if (tssDescriptor.isBusy()) {
        throw GeneralProtectionFault(makeErrorCode(gate.selector(), 0, source), "Interrupt to task gate referencing busy TSS descriptor");
    }
    if (!tssDescriptor.present()) {
        throw GeneralProtectionFault(makeErrorCode(gate.selector(), 0, source), "Interrupt to task gate referencing non-present TSS descriptor");
    }
    taskSwitch(tssDescriptor, JumpType::INT);
    if (errorCode.has_value()) {
        if (tssDescriptor.is32Bit())
            push32(errorCode.value());
        else
            push16(errorCode.value());
    }
}

void CPU::jumpToInterruptHandler(int isr, InterruptSource source, std::optional<WORD> errorCode)
{
    bool isTrap = false;
    FarPointer vector;

    Gate gate;

    if (getPE()) {
        gate = getInterruptGate(isr);

        if (source == InterruptSource::Internal) {
            if (gate.DPL() < getCPL()) {
                throw GeneralProtectionFault(makeErrorCode(isr, 1, source), "Software interrupt trying to escalate privilege");
            }
        }

        if (!gate.present()) {
            throw NotPresent(makeErrorCode(isr, 1, source), "Interrupt gate not present");
        }

        if (gate.isNull()) {
            throw GeneralProtectionFault(makeErrorCode(isr, 1, source), "Interrupt gate is null");
        }

        vector.segment = gate.selector();
        vector.offset = gate.offset();

        if (options.trapint)
            vlog(LogCPU, "PE=1 Interrupt %02x trapped%s, type: %s (%1x), %04x:%08x", isr, source == InterruptSource::External ? " (from PIC)" : "", gate.typeName(), gate.type(), vector.segment, vector.offset);

        if (gate.isTaskGate()) {
            interruptToTaskGate(isr, source, errorCode, gate);
            return;
        }

        Descriptor codeDescriptor = getDescriptor(gate.selector());
        if (codeDescriptor.isError()) {
            throw GeneralProtectionFault(makeErrorCode(gate.selector(), 0, source), "Interrupt gate to segment outside table limit");
        }

        if (!codeDescriptor.isCode()) {
            throw GeneralProtectionFault(makeErrorCode(gate.selector(), 0, source), "Interrupt gate to non-code segment");
        }

        if (codeDescriptor.DPL() > getCPL()) {
            throw GeneralProtectionFault(makeErrorCode(gate.selector(), 0, source), QString("Interrupt gate to segment with DPL(%1)>CPL(%2)").arg(codeDescriptor.DPL()).arg(getCPL()));
        }

        if (!codeDescriptor.present()) {
            throw NotPresent(makeErrorCode(gate.selector(), 0, source), "Interrupt to non-present segment");
        }

        switch (gate.type()) {
        case 0x7: // 80286 Trap Gate (16-bit)
        case 0xf: // 80386 Trap Gate (32-bit)
            isTrap = true;
            break;
        case 0x6: // 80286 Interrupt Gate (16-bit)
        case 0xe: // 80386 Interrupt Gate (32-bit)
            break;
        default:
            // FIXME: What should be the error code here?
            throw GeneralProtectionFault(isr, "Interrupt to bad gate type");
            break;
        }
    } else {
        if (options.trapint)
            vlog(LogCPU, "PE=0 Interrupt %02X,%02X trapped%s", isr, this->regs.B.AH, source == InterruptSource::External ? " (from PIC)" : "");

        vector.segment = (m_memory[isr * 4 + 3] << 8) | m_memory[isr * 4 + 2];
        vector.offset = (m_memory[isr * 4 + 1] << 8) | m_memory[isr * 4];
    }

    if (o16())
        jump16(vector.segment, vector.offset, JumpType::INT, isr, getFlags(), &gate, errorCode);
    else
        jump32(vector.segment, vector.offset, JumpType::INT, isr, getEFlags(), &gate, errorCode);

    if (!isTrap)
        setIF(0);
    setTF(0);
    setRF(0);
    setNT(0);
}
