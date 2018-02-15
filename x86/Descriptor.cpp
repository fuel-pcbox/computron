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

#include "Descriptor.h"
#include "CPU.h"
#include "debugger.h"

Descriptor CPU::getDescriptor(WORD index)
{
    Descriptor descriptor;

    if (!getPE()) {
        descriptor.m_index = index;
        descriptor.m_base = (DWORD)index << 4;
        descriptor.m_limit = 0xFFFFF;
        descriptor.m_D = false;
        descriptor.m_DT = true;
        descriptor.m_isGlobal = true;
        return descriptor;
    }

    descriptor.m_isGlobal = (index & 0x04) == 0;
    descriptor.m_RPL = index & 3;
    index &= 0xfffffff8;
    descriptor.m_index = index;
    WORD tableLimit = descriptor.m_isGlobal ? GDTR.limit : LDTR.limit;
    if (index >= tableLimit) {
        vlog(LogCPU, "Segment selector index 0x%04x >= %s.limit (0x%04x).", index, descriptor.m_isGlobal ? "GDTR" : "LDTR", tableLimit);
        VM_ASSERT(false);
        //dumpAll();
        debugger().enter();
        //hard_exit(1);
    }

    DWORD descriptorTableBase = descriptor.m_isGlobal ? GDTR.base : LDTR.base;

    DWORD hi = readMemory32(descriptorTableBase + index + 4);
    DWORD lo = readMemory32(descriptorTableBase + index);

    descriptor.m_G = (hi >> 23) & 1; // Limit granularity, 0=1b, 1=4kB
    descriptor.m_D = (hi >> 22) & 1;
    descriptor.m_AVL = (hi >> 20) & 1;
    descriptor.m_P = (hi >> 15) & 1;
    descriptor.m_DPL = (hi >> 13) & 3; // Privilege (ring) level
    descriptor.m_DT = (hi >> 12) & 1;
    descriptor.m_type = (hi >> 8) & 0xF;

    if (descriptor.isSystemDescriptor() && descriptor.asSystemDescriptor().isCallGate()) {
        descriptor.m_callGateSelector = lo >> 16;
        descriptor.m_callGateCount = hi & 0xf; // FIXME: verify field size
        descriptor.m_callGateOffset = (hi & 0xffff0000) | (lo & 0xffff);
    } else {
        descriptor.m_base = (hi & 0xFF000000) | ((hi & 0xFF) << 16) | ((lo >> 16) & 0xFFFF);
        descriptor.m_limit = (hi & 0xF0000) | (lo & 0xFFFF);
    }

    if (options.pedebug) {
        vlog(LogCPU, "getDescriptor: GDTR.base{%08x} + index{%04x} => base:%08x, limit:%08x, type:%02x, [hi=%08x, lo=%08x]", GDTR.base, index, descriptor.base(), descriptor.limit(), (unsigned)descriptor.type(), hi, lo);
    }
    return descriptor;
}

const char* SystemDescriptor::typeName() const
{
    switch (m_type) {
    case SystemDescriptor::Invalid: return "Invalid";
    case SystemDescriptor::AvailableTSS_16bit: return "AvailableTSS_16bit";
    case SystemDescriptor::LDT: return "LDT";
    case SystemDescriptor::BusyTSS_16bit: return "BusyTSS_16bit";
    case SystemDescriptor::CallGate_16bit: return "CallGate_16bit";
    case SystemDescriptor::TaskGate_16bit: return "TaskGate_16bit";
    case SystemDescriptor::InterruptGate_16bit: return "InterruptGate_16bit";
    case SystemDescriptor::TrapGate_16bit: return "TrapGate_16bit";
    case SystemDescriptor::AvailableTSS_32bit: return "AvailableTSS_32bit";
    case SystemDescriptor::BusyTSS_32bit: return "BusyTSS_32bit";
    case SystemDescriptor::CallGate_32bit: return "CallGate_32bit";
    case SystemDescriptor::InterruptGate_32bit: return "InterruptGate_32bit";
    case SystemDescriptor::TaskGate_32bit: return "TaskGate_32bit";
    default: return "(Reserved)";
    }
}
