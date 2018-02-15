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

#pragma once

#include "debug.h"
#include "types.h"
#include "vomit.h"

class SegmentDescriptor;
class SystemDescriptor;

class Descriptor {
    friend class CPU;
public:
    DWORD effectiveLimit() const { return granularity() ? (limit() * 4096) : limit(); }

    unsigned index() const { return m_index; }
    bool isGlobal() const { return m_isGlobal; }
    bool RPL() const { return m_RPL; }

    bool isSegmentDescriptor() const { return m_DT; }
    bool isSystemDescriptor() const { return !m_DT; }

    unsigned DPL() const { return m_DPL; }
    bool present() const { return m_P; }
    bool granularity() const { return m_G; }
    bool D() const { return m_D; }
    bool available() const { return m_AVL; }
    DWORD base() const { return m_base; }
    WORD limit() const { return m_limit; }

    unsigned type() const { return m_type; }

    SegmentDescriptor& asSegmentDescriptor();
    SystemDescriptor& asSystemDescriptor();
    const SegmentDescriptor& asSegmentDescriptor() const;
    const SystemDescriptor& asSystemDescriptor() const;

protected:
    union {
        struct {
            DWORD m_base;
            DWORD m_limit;
        };
        struct {
            WORD m_callGateCount;
            WORD m_callGateSelector;
            DWORD m_callGateOffset;
        };
    };
    unsigned m_DPL { 0 };
    unsigned m_type { 0 };
    bool m_G { false };
    bool m_D { false };
    bool m_P { false };
    bool m_AVL { false };
    bool m_DT { false };

    // These three are not part of the descriptor, but metadata about the lookup that found this descriptor.
    unsigned m_index { 0xFFFFFFFF };
    bool m_isGlobal { false };
    BYTE m_RPL { 0 };
};

class CallGate;

class SystemDescriptor : public Descriptor {
public:
    enum Type {
        Invalid = 0,
        AvailableTSS_16bit = 0x1,
        LDT = 0x2,
        BusyTSS_16bit = 0x3,
        CallGate_16bit = 0x4,
        TaskGate_16bit = 0x5,
        InterruptGate_16bit = 0x6,
        TrapGate_16bit = 0x7,
        AvailableTSS_32bit = 0x9,
        BusyTSS_32bit = 0xb,
        CallGate_32bit = 0xc,
        InterruptGate_32bit = 0xe,
        TaskGate_32bit = 0xf,
    };

    Type type() const { return static_cast<Type>(m_type); }
    const char* typeName() const;

    bool isCallGate() const { return type() == CallGate_16bit || type() == CallGate_32bit; }

    CallGate& asCallGate();
    const CallGate& asCallGate() const;
};

class CallGate : public SystemDescriptor {
public:
    WORD selector() const { return m_callGateSelector; }
    DWORD offset() const { return m_callGateOffset; }
    WORD count() const { return m_callGateCount; }
};

inline CallGate& SystemDescriptor::asCallGate()
{
    VM_ASSERT(isCallGate());
    return static_cast<CallGate&>(*this);
}

inline const CallGate& SystemDescriptor::asCallGate() const
{
    VM_ASSERT(isCallGate());
    return static_cast<const CallGate&>(*this);
}

class CodeSegmentDescriptor;
class DataSegmentDescriptor;

class SegmentDescriptor : public Descriptor {
public:
    bool isCode() const { return (m_type & 0x8) != 0; }
    bool isData() const { return (m_type & 0x8) == 0; }
    bool accessed() const { return m_type & 0x1; }

    CodeSegmentDescriptor& asCodeSegmentDescriptor();
    DataSegmentDescriptor& asDataSegmentDescriptor();
    const CodeSegmentDescriptor& asCodeSegmentDescriptor() const;
    const DataSegmentDescriptor& asDataSegmentDescriptor() const;
};

class CodeSegmentDescriptor : public SegmentDescriptor {
public:
    bool readable() const { return m_type & 0x2; }
    bool conforming() const { return m_type & 0x4; }
};

class DataSegmentDescriptor : public SegmentDescriptor {
public:
    bool writable() const { return m_type & 0x2; }
    bool expandDown() const { return m_type & 0x4; }
};

inline SegmentDescriptor& Descriptor::asSegmentDescriptor()
{
    VM_ASSERT(isSegmentDescriptor());
    return static_cast<SegmentDescriptor&>(*this);
}

inline SystemDescriptor& Descriptor::asSystemDescriptor()
{
    VM_ASSERT(isSystemDescriptor());
    return static_cast<SystemDescriptor&>(*this);
}

inline CodeSegmentDescriptor& SegmentDescriptor::asCodeSegmentDescriptor()
{
    VM_ASSERT(isCode());
    return static_cast<CodeSegmentDescriptor&>(*this);
}

inline DataSegmentDescriptor& SegmentDescriptor::asDataSegmentDescriptor()
{
    VM_ASSERT(isData());
    return static_cast<DataSegmentDescriptor&>(*this);
}

inline const SegmentDescriptor& Descriptor::asSegmentDescriptor() const
{
    VM_ASSERT(isSegmentDescriptor());
    return static_cast<const SegmentDescriptor&>(*this);
}

inline const SystemDescriptor& Descriptor::asSystemDescriptor() const
{
    VM_ASSERT(isSystemDescriptor());
    return static_cast<const SystemDescriptor&>(*this);
}

inline const CodeSegmentDescriptor& SegmentDescriptor::asCodeSegmentDescriptor() const
{
    VM_ASSERT(isCode());
    return static_cast<const CodeSegmentDescriptor&>(*this);
}

inline const DataSegmentDescriptor& SegmentDescriptor::asDataSegmentDescriptor() const
{
    VM_ASSERT(isData());
    return static_cast<const DataSegmentDescriptor&>(*this);
}
