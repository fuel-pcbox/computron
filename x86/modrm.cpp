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

#define DEFAULT_TO_SS if (!hasSegmentPrefix()) { segment = getSS(); }

MemoryOrRegisterReference::MemoryOrRegisterReference(VCpu& cpu, void* registerPointer, ValueSize size)
    : m_cpu(cpu)
    , m_registerPointer(registerPointer)
    , m_size(size)
{
}

MemoryOrRegisterReference::MemoryOrRegisterReference(VCpu& cpu, WORD segment, DWORD offset, ValueSize size)
    : m_cpu(cpu)
    , m_segment(segment)
    , m_offset(offset)
    , m_size(size)
{
}

WORD MemoryOrRegisterReference::segment()
{
    VM_ASSERT(!isRegister());
    return m_segment;
}

DWORD MemoryOrRegisterReference::offset()
{
    VM_ASSERT(!isRegister());
    return m_offset;
}

void* MemoryOrRegisterReference::memoryPointer()
{
    if (isRegister())
        return m_registerPointer;
    return m_cpu.memoryPointer(m_segment, m_offset);
}

BYTE MemoryOrRegisterReference::read8()
{
    if (isRegister())
        return *reinterpret_cast<BYTE*>(m_registerPointer);
    return m_cpu.readMemory8(m_segment, m_offset);
}

WORD MemoryOrRegisterReference::read16()
{
    if (isRegister())
        return *reinterpret_cast<WORD*>(m_registerPointer);
    return m_cpu.readMemory16(m_segment, m_offset);
}

DWORD MemoryOrRegisterReference::read32()
{
    if (isRegister())
        return *reinterpret_cast<DWORD*>(m_registerPointer);
    return m_cpu.readMemory32(m_segment, m_offset);
}

void MemoryOrRegisterReference::write8(BYTE data)
{
    if (isRegister()) {
        *reinterpret_cast<BYTE*>(m_registerPointer) = data;
        return;
    }
    m_cpu.writeMemory8(m_segment, m_offset, data);
}

void MemoryOrRegisterReference::write16(WORD data)
{
    if (isRegister()) {
        *reinterpret_cast<WORD*>(m_registerPointer) = data;
        return;
    }
    m_cpu.writeMemory16(m_segment, m_offset, data);
}

void MemoryOrRegisterReference::write32(DWORD data)
{
    if (isRegister()) {
        *reinterpret_cast<DWORD*>(m_registerPointer) = data;
        return;
    }
    m_cpu.writeMemory32(m_segment, m_offset, data);
}

MemoryOrRegisterReference VCpu::resolveModRM8(BYTE rmbyte)
{
    return resolveModRM_internal(rmbyte, ByteSize);
}

MemoryOrRegisterReference VCpu::resolveModRM16(BYTE rmbyte)
{
    return resolveModRM_internal(rmbyte, WordSize);
}

MemoryOrRegisterReference VCpu::resolveModRM32(BYTE rmbyte)
{
    return resolveModRM_internal(rmbyte, DWordSize);
}

void VCpu::writeModRM32(BYTE rmbyte, DWORD value)
{
    resolveModRM_internal(rmbyte, DWordSize).write32(value);
}

void VCpu::writeModRM16(BYTE rmbyte, WORD value)
{
    resolveModRM_internal(rmbyte, WordSize).write16(value);
}

void VCpu::writeModRM8(BYTE rmbyte, BYTE value)
{
    resolveModRM_internal(rmbyte, ByteSize).write8(value);
}

WORD VCpu::readModRM16(BYTE rmbyte)
{
    return resolveModRM_internal(rmbyte, WordSize).read16();
}

MemoryOrRegisterReference VCpu::resolveModRM_internal(BYTE rmbyte, ValueSize size)
{
    if (a32())
        return resolveModRM32_internal(rmbyte, size);

    if (size == ByteSize)
        return resolveModRM8_internal(rmbyte);

    return resolveModRM16_internal(rmbyte);
}

BYTE VCpu::readModRM8(BYTE rmbyte)
{
    return resolveModRM_internal(rmbyte, ByteSize).read8();
}

DWORD VCpu::readModRM32(BYTE rmbyte)
{
    return resolveModRM_internal(rmbyte, DWordSize).read32();
}

FarPointer VCpu::readModRMFarPointerSegmentFirst(BYTE rmbyte)
{
    auto location = resolveModRM_internal(rmbyte, DWordSize);
    VM_ASSERT(!location.isRegister());

    FarPointer ptr;
    ptr.segment = readMemory16(location.segment(), location.offset());
    ptr.offset = readMemory32(location.segment(), location.offset() + 2);

    vlog(LogCPU, "Loaded far pointer (segment first) from %04X:%08X [PE=%u], got %04X:%08X", location.segment(), location.offset(), getPE(), ptr.segment, ptr.offset);

    return ptr;
}

FarPointer VCpu::readModRMFarPointerOffsetFirst(BYTE rmbyte)
{
    auto location = resolveModRM_internal(rmbyte, DWordSize);
    VM_ASSERT(!location.isRegister());

    FarPointer ptr;
    ptr.segment = readMemory16(location.segment(), location.offset() + 4);
    ptr.offset = readMemory32(location.segment(), location.offset());

    vlog(LogCPU, "Loaded far pointer (offset first) from %04X:%08X [PE=%u], got %04X:%08X", location.segment(), location.offset(), getPE(), ptr.segment, ptr.offset);

    return ptr;
}

MemoryOrRegisterReference VCpu::resolveModRM8_internal(BYTE rmbyte)
{
    VM_ASSERT(a16());

    WORD segment = currentSegment();
    WORD offset { 0 };

    switch (rmbyte & 0xC0) {
    case 0x00:
        switch (rmbyte & 0x07) {
        case 0: offset = getBX() + getSI(); break;
        case 1: offset = getBX() + getDI(); break;
        case 2: DEFAULT_TO_SS; offset = getBP() + getSI(); break;
        case 3: DEFAULT_TO_SS; offset = getBP() + getDI(); break;
        case 4: offset = getSI(); break;
        case 5: offset = getDI(); break;
        case 6: offset = fetchOpcodeWord(); break;
        default: offset = getBX(); break;
        }
        return { *this, segment, offset, ByteSize };
    case 0x40:
        offset = vomit_signExtend<WORD>(fetchOpcodeByte());
        switch (rmbyte & 0x07) {
        case 0: offset += getBX() + getSI(); break;
        case 1: offset += getBX() + getDI(); break;
        case 2: DEFAULT_TO_SS; offset += getBP() + getSI(); break;
        case 3: DEFAULT_TO_SS; offset += getBP() + getDI(); break;
        case 4: offset += getSI(); break;
        case 5: offset += getDI(); break;
        case 6: DEFAULT_TO_SS; offset += getBP(); break;
        default: offset += getBX(); break;
        }
        return { *this, segment, offset, ByteSize };
        break;
    case 0x80:
        offset = fetchOpcodeWord();
        switch (rmbyte & 0x07) {
        case 0: offset += getBX() + getSI(); break;
        case 1: offset += getBX() + getDI(); break;
        case 2: DEFAULT_TO_SS; offset += getBP() + getSI(); break;
        case 3: DEFAULT_TO_SS; offset += getBP() + getDI(); break;
        case 4: offset += getSI(); break;
        case 5: offset += getDI(); break;
        case 6: DEFAULT_TO_SS; offset += getBP(); break;
        default: offset += getBX(); break;
        }
        return { *this, segment, offset, ByteSize };
    default: // 0xC0
        return { *this, treg8[rmbyte & 7], ByteSize };
    }
}

MemoryOrRegisterReference VCpu::resolveModRM16_internal(BYTE rmbyte)
{
    VM_ASSERT(a16());

    WORD segment = currentSegment();
    WORD offset = 0x0000;

    switch (rmbyte & 0xC0) {
    case 0x00:
        switch (rmbyte & 0x07) {
        case 0: offset = getBX() + getSI(); break;
        case 1: offset = getBX() + getDI(); break;
        case 2: DEFAULT_TO_SS; offset = getBP() + getSI(); break;
        case 3: DEFAULT_TO_SS; offset = getBP() + getDI(); break;
        case 4: offset = getSI(); break;
        case 5: offset = getDI(); break;
        case 6: offset = fetchOpcodeWord(); break;
        default: offset = getBX(); break;
        }
        return { *this, segment, offset, WordSize };
    case 0x40:
        offset = vomit_signExtend<WORD>(fetchOpcodeByte());
        switch (rmbyte & 0x07) {
        case 0: offset += getBX() + getSI(); break;
        case 1: offset += getBX() + getDI(); break;
        case 2: DEFAULT_TO_SS; offset += getBP() + getSI(); break;
        case 3: DEFAULT_TO_SS; offset += getBP() + getDI(); break;
        case 4: offset += getSI(); break;
        case 5: offset += getDI(); break;
        case 6: DEFAULT_TO_SS; offset += getBP(); break;
        default: offset += getBX(); break;
        }
        return { *this, segment, offset, WordSize };
    case 0x80:
        offset = fetchOpcodeWord();
        switch (rmbyte & 0x07) {
        case 0: offset += getBX() + getSI(); break;
        case 1: offset += getBX() + getDI(); break;
        case 2: DEFAULT_TO_SS; offset += getBP() + getSI(); break;
        case 3: DEFAULT_TO_SS; offset += getBP() + getDI(); break;
        case 4: offset += getSI(); break;
        case 5: offset += getDI(); break;
        case 6: DEFAULT_TO_SS; offset += getBP(); break;
        default: offset += getBX(); break;
        }
        return { *this, segment, offset, WordSize };
    default: // 0xC0
        return { *this, treg16[rmbyte & 7], WordSize };
    }
}

MemoryOrRegisterReference VCpu::resolveModRM32_internal(BYTE rmbyte, ValueSize size)
{
    VM_ASSERT(a32());

    WORD segment = currentSegment();
    DWORD offset { 0 };

    switch (rmbyte & 0xC0) {
    case 0x00:
        switch (rmbyte & 0x07) {
        case 0: offset = getEAX(); break;
        case 1: offset = getECX(); break;
        case 2: offset = getEDX(); break;
        case 3: offset = getEBX(); break;
        case 4: offset = evaluateSIB(rmbyte, fetchOpcodeByte(), segment, 0); break;
        case 5: offset = fetchOpcodeDWord(); break;
        case 6: offset = getESI(); break;
        default: offset = getEDI(); break;
        }
        return { *this, segment, offset, size };
    case 0x40:
        switch (rmbyte & 0x07) {
        case 0: offset = getEAX() + vomit_signExtend<DWORD>(fetchOpcodeByte()); break;
        case 1: offset = getECX() + vomit_signExtend<DWORD>(fetchOpcodeByte()); break;
        case 2: offset = getEDX() + vomit_signExtend<DWORD>(fetchOpcodeByte()); break;
        case 3: offset = getEBX() + vomit_signExtend<DWORD>(fetchOpcodeByte()); break;
        case 4: offset = evaluateSIB(rmbyte, fetchOpcodeByte(), segment, 8); break;
        case 5: DEFAULT_TO_SS; offset = getEBP() + vomit_signExtend<DWORD>(fetchOpcodeByte()); break;
        case 6: offset = getESI() + vomit_signExtend<DWORD>(fetchOpcodeByte()); break;
        default: offset = getEDI() + vomit_signExtend<DWORD>(fetchOpcodeByte()); break;
        }
        return { *this, segment, offset, size };
    case 0x80:
        switch (rmbyte & 0x07) {
        case 0: offset = getEAX() + fetchOpcodeDWord(); break;
        case 1: offset = getECX() + fetchOpcodeDWord(); break;
        case 2: offset = getEDX() + fetchOpcodeDWord(); break;
        case 3: offset = getEBX() + fetchOpcodeDWord(); break;
        case 4: offset = evaluateSIB(rmbyte, fetchOpcodeByte(), segment, 32); break;
        case 5: DEFAULT_TO_SS; offset = getEBP() + fetchOpcodeDWord(); break;
        case 6: offset = getESI() + fetchOpcodeDWord(); break;
        default: offset = getEDI() + fetchOpcodeDWord(); break;
        }
        return { *this, segment, offset, size };
    default: // 0xC0
        switch (size) {
        case DWordSize: return { *this, treg32[rmbyte & 7], size };
        case WordSize: return { *this, treg16[rmbyte & 7], size };
        case ByteSize: return { *this, treg8[rmbyte & 7], size };
        }
    }
    VM_ASSERT(false);
}

DWORD VCpu::evaluateSIB(BYTE rm, BYTE sib, WORD& segment, unsigned sizeOfImmediate)
{
    DWORD scale;
    switch (sib & 0xC0) {
    case 0x00: scale = 1; break;
    case 0x40: scale = 2; break;
    case 0x80: scale = 4; break;
    case 0xC0: scale = 8; break;
    }
    DWORD index;
    switch ((sib >> 3) & 0x07) {
    case 0: index = getEAX(); break;
    case 1: index = getECX(); break;
    case 2: index = getEDX(); break;
    case 3: index = getEBX(); break;
    case 4: index = 0; break;
    case 5: DEFAULT_TO_SS; index = getEBP(); break;
    case 6: index = getESI(); break;
    case 7: index = getEDI(); break;
    }
    DWORD scaledIndex = index * scale;

    DWORD base = 0;
    switch (sib & 0x07) {
    case 0: base = getEAX(); break;
    case 1: base = getECX(); break;
    case 2: base = getEDX(); break;
    case 3: base = getEBX(); break;
    case 4: DEFAULT_TO_SS; base = getESP(); break;
    case 6: base = getESI(); break;
    case 7: base = getEDI(); break;
    default: // 5
        // FIXME: Uh, what to do if the ModR/M byte signalled a different size of immediate?
        sizeOfImmediate = 0;
        switch ((rm >> 6) & 3) {
        case 0: base = fetchOpcodeDWord(); break;
        case 1: DEFAULT_TO_SS; base = vomit_signExtend<DWORD>(fetchOpcodeByte()) + getEBP(); break;
        case 2: DEFAULT_TO_SS; base = fetchOpcodeDWord() + getEBP(); break;
        default: VM_ASSERT(false); break;
        }
        break;
    }

    if (sizeOfImmediate == 8)
        base = vomit_signExtend<DWORD>(fetchOpcodeByte());
    else if (sizeOfImmediate == 32)
        base = fetchOpcodeDWord();
    //dumpAll();
    //vlog(LogCPU, "evaluateSIB(): sib=%02X -> %s+%s*%u, scaledIndex:%08X, base:%08X", sib, registerName(baseRegister), registerName(indexRegister), scale, scaledIndex, base);
    return scaledIndex + base;
}
