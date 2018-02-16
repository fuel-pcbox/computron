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

class CodeSegmentDescriptor;
class DataSegmentDescriptor;
class Gate;
class LDTDescriptor;
class SegmentDescriptor;
class SystemDescriptor;
class TSSDescriptor;

class Descriptor {
    friend class CPU;
public:
    enum Error {
        NoError = 0,
        LimitExceeded = 1,
    };

    Descriptor() { }

    unsigned index() const { return m_index; }
    bool isGlobal() const { return m_isGlobal; }
    bool RPL() const { return m_RPL; }

    bool isSegmentDescriptor() const { return m_DT; }
    bool isSystemDescriptor() const { return !m_DT; }

    bool isError() const { return m_error != NoError; }
    Error error() const { return m_error; }

    unsigned DPL() const { return m_DPL; }
    bool present() const { return m_P; }
    bool D() const { return m_D; }
    bool available() const { return m_AVL; }

    unsigned type() const { return m_type; }

    bool isCode() const;
    bool isData() const;
    bool isGate() const;
    bool isTSS() const;
    bool isLDT() const;
    bool isNull() const { return m_DT == 0 && m_type == 0; }

    SegmentDescriptor& asSegmentDescriptor();
    SystemDescriptor& asSystemDescriptor();
    Gate& asGate();
    TSSDescriptor& asTSSDescriptor();
    LDTDescriptor& asLDTDescriptor();
    CodeSegmentDescriptor& asCodeSegmentDescriptor();
    DataSegmentDescriptor& asDataSegmentDescriptor();

    const SegmentDescriptor& asSegmentDescriptor() const;
    const SystemDescriptor& asSystemDescriptor() const;
    const Gate& asGate() const;
    const TSSDescriptor& asTSSDescriptor() const;
    const LDTDescriptor& asLDTDescriptor() const;
    const CodeSegmentDescriptor& asCodeSegmentDescriptor() const;
    const DataSegmentDescriptor& asDataSegmentDescriptor() const;

protected:
    union {
        struct {
            DWORD m_segmentBase { 0 };
            DWORD m_segmentLimit { 0 };
        };
        struct {
            WORD m_gateParameterCount;
            WORD m_gateSelector;
            DWORD m_gateOffset;
        };
    };
    unsigned m_DPL { 0 };
    unsigned m_type { 0 };
    bool m_G { false };
    bool m_D { false };
    bool m_P { false };
    bool m_AVL { false };
    bool m_DT { false };

    // These are not part of the descriptor, but metadata about the lookup that found this descriptor.
    unsigned m_index { 0xFFFFFFFF };
    bool m_isGlobal { false };
    BYTE m_RPL { 0 };
    Error m_error { NoError };
};

class ErrorDescriptor : public Descriptor {
public:
    explicit ErrorDescriptor(Error error) { m_error = error; }
};

class SystemDescriptor : public Descriptor {
public:
    enum Type {
        Invalid = 0,
        AvailableTSS_16bit = 0x1,
        LDT = 0x2,
        BusyTSS_16bit = 0x3,
        CallGate_16bit = 0x4,
        TaskGate = 0x5,
        InterruptGate_16bit = 0x6,
        TrapGate_16bit = 0x7,
        AvailableTSS_32bit = 0x9,
        BusyTSS_32bit = 0xb,
        CallGate_32bit = 0xc,
        InterruptGate_32bit = 0xe,
        TrapGate_32bit = 0xf,
    };

    Type type() const { return static_cast<Type>(m_type); }
    const char* typeName() const;

    bool isCallGate() const { return type() == CallGate_16bit || type() == CallGate_32bit; }
    bool isInterruptGate() const { return type() == InterruptGate_16bit || type() == InterruptGate_32bit; }
    bool isTrapGate() const { return type() == TrapGate_16bit || type() == TrapGate_32bit; }
    bool isTaskGate() const { return type() == TaskGate; }
    bool isGate() const { return isCallGate() || isInterruptGate() || isTrapGate() || isTaskGate(); }
    bool isTSS() const { return type() == AvailableTSS_16bit || type() == BusyTSS_16bit || type() == AvailableTSS_32bit || type() == BusyTSS_32bit; }
    bool isLDT() const { return type() == LDT; }
};

class Gate : public SystemDescriptor {
public:
    WORD selector() const { return m_gateSelector; }
    DWORD offset() const { return m_gateOffset; }
    WORD parameterCount() const { return m_gateParameterCount; }
};

class TSSDescriptor : public SystemDescriptor {
public:
    DWORD base() const { return m_segmentBase; }
    WORD limit() const { return m_segmentLimit; }
};

class LDTDescriptor : public SystemDescriptor {
public:
    DWORD base() const { return m_segmentBase; }
    WORD limit() const { return m_segmentLimit; }
};

inline Gate& Descriptor::asGate()
{
    VM_ASSERT(isGate());
    return static_cast<Gate&>(*this);
}

inline const Gate& Descriptor::asGate() const
{
    VM_ASSERT(isGate());
    return static_cast<const Gate&>(*this);
}

inline TSSDescriptor& Descriptor::asTSSDescriptor()
{
    VM_ASSERT(isTSS());
    return static_cast<TSSDescriptor&>(*this);
}

inline const TSSDescriptor& Descriptor::asTSSDescriptor() const
{
    VM_ASSERT(isTSS());
    return static_cast<const TSSDescriptor&>(*this);
}

inline LDTDescriptor& Descriptor::asLDTDescriptor()
{
    VM_ASSERT(isLDT());
    return static_cast<LDTDescriptor&>(*this);
}

inline const LDTDescriptor& Descriptor::asLDTDescriptor() const
{
    VM_ASSERT(isLDT());
    return static_cast<const LDTDescriptor&>(*this);
}

class SegmentDescriptor : public Descriptor {
public:
    DWORD base() const { return m_segmentBase; }
    WORD limit() const { return m_segmentLimit; }

    bool isCode() const { return (m_type & 0x8) != 0; }
    bool isData() const { return (m_type & 0x8) == 0; }
    bool accessed() const { return m_type & 0x1; }

    DWORD effectiveLimit() const { return granularity() ? (limit() * 4096) : limit(); }
    bool granularity() const { return m_G; }
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

inline CodeSegmentDescriptor& Descriptor::asCodeSegmentDescriptor()
{
    VM_ASSERT(isCode());
    return static_cast<CodeSegmentDescriptor&>(*this);
}

inline DataSegmentDescriptor& Descriptor::asDataSegmentDescriptor()
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

inline const CodeSegmentDescriptor& Descriptor::asCodeSegmentDescriptor() const
{
    VM_ASSERT(isCode());
    return static_cast<const CodeSegmentDescriptor&>(*this);
}

inline const DataSegmentDescriptor& Descriptor::asDataSegmentDescriptor() const
{
    VM_ASSERT(isData());
    return static_cast<const DataSegmentDescriptor&>(*this);
}

inline bool Descriptor::isGate() const
{
    return isSystemDescriptor() && asSystemDescriptor().isGate();
}

inline bool Descriptor::isTSS() const
{
    return isSystemDescriptor() && asSystemDescriptor().isTSS();
}

inline bool Descriptor::isLDT() const
{
    return isSystemDescriptor() && asSystemDescriptor().isLDT();
}

inline bool Descriptor::isCode() const
{
    return isSegmentDescriptor() && asSegmentDescriptor().isCode();
}

inline bool Descriptor::isData() const
{
    return isSegmentDescriptor() && asSegmentDescriptor().isData();
}
