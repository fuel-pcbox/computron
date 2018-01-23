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

void* VCpu::resolveModRM8(BYTE rmbyte)
{
    BYTE* registerPointer = static_cast<BYTE*>(resolveModRM_internal(rmbyte, ByteSize));
    if (registerPointer)
        return registerPointer;
    return this->memoryPointer(m_lastModRMSegment, m_lastModRMOffset);
}

void* VCpu::resolveModRM16(BYTE rmbyte)
{
    WORD* registerPointer = static_cast<WORD*>(resolveModRM_internal(rmbyte, WordSize));
    if (registerPointer)
        return registerPointer;
    return this->memoryPointer(m_lastModRMSegment, m_lastModRMOffset);
}

void* VCpu::resolveModRM32(BYTE rmbyte)
{
    DWORD* registerPointer = static_cast<DWORD*>(resolveModRM_internal(rmbyte, DWordSize));
    if (registerPointer)
        return registerPointer;
    return this->memoryPointer(m_lastModRMSegment, m_lastModRMOffset);
}

void VCpu::writeModRM32(BYTE rmbyte, DWORD value)
{
    DWORD* registerPointer = reinterpret_cast<DWORD*>(resolveModRM_internal(rmbyte, DWordSize));

    if (registerPointer)
        *registerPointer = value;
    else
        writeMemory32(m_lastModRMSegment, m_lastModRMOffset, value);
}

void VCpu::writeModRM16(BYTE rmbyte, WORD value)
{
    WORD* registerPointer = reinterpret_cast<WORD*>(resolveModRM_internal(rmbyte, WordSize));

    if (registerPointer)
        *registerPointer = value;
    else
        writeMemory16(m_lastModRMSegment, m_lastModRMOffset, value);
}

void VCpu::writeModRM8(BYTE rmbyte, BYTE value)
{
    BYTE* registerPointer = reinterpret_cast<BYTE*>(resolveModRM_internal(rmbyte, ByteSize));

    if (registerPointer)
        *registerPointer = value;
    else
        writeMemory8(m_lastModRMSegment, m_lastModRMOffset, value);
}

WORD VCpu::readModRM16(BYTE rmbyte)
{
    WORD* registerPointer = reinterpret_cast<WORD*>(resolveModRM_internal(rmbyte, WordSize));

    if (registerPointer)
        return *registerPointer;
    return readMemory16(m_lastModRMSegment, m_lastModRMOffset);
}

void* VCpu::resolveModRM_internal(BYTE rmbyte, ValueSize size)
{
    if (a32())
        return reinterpret_cast<DWORD*>(resolveModRM32_internal(rmbyte, size));

    if (size == ByteSize)
        return reinterpret_cast<DWORD*>(resolveModRM8_internal(rmbyte));

    return reinterpret_cast<DWORD*>(resolveModRM16_internal(rmbyte));
}

BYTE VCpu::readModRM8(BYTE rmbyte)
{
    BYTE* registerPointer = reinterpret_cast<BYTE*>(resolveModRM_internal(rmbyte, ByteSize));
    if (registerPointer)
        return *registerPointer;
    return readMemory8(m_lastModRMSegment, m_lastModRMOffset);
}

void VCpu::updateModRM32(DWORD value)
{
    if (m_lastModRMPointer)
        *(reinterpret_cast<DWORD*>(m_lastModRMPointer)) = value;
    else
        writeMemory32(m_lastModRMSegment, m_lastModRMOffset, value);
}

void VCpu::updateModRM16(WORD value)
{
    if (m_lastModRMPointer)
        *(reinterpret_cast<WORD*>(m_lastModRMPointer)) = value;
    else
        writeMemory16(m_lastModRMSegment, m_lastModRMOffset, value);
}

void VCpu::updateModRM8(BYTE value)
{
    if (m_lastModRMPointer)
        *(reinterpret_cast<BYTE*>(m_lastModRMPointer)) = value;
    else
        writeMemory8(m_lastModRMSegment, m_lastModRMOffset, value);
}

DWORD VCpu::readModRM32(BYTE rmbyte)
{
    DWORD* registerPointer = reinterpret_cast<DWORD*>(resolveModRM_internal(rmbyte, DWordSize));

    if (registerPointer)
        return *registerPointer;

    return readMemory32(m_lastModRMSegment, m_lastModRMOffset);
}

FarPointer VCpu::readModRMFarPointerSegmentFirst(BYTE rmbyte)
{
    void* registerPointer = resolveModRM_internal(rmbyte, DWordSize);

    // FIXME: What should I do if it's a register? :|
    assert(!registerPointer);

    FarPointer ptr;
    ptr.segment = readMemory16(m_lastModRMSegment, m_lastModRMOffset);
    ptr.offset = readMemory32(m_lastModRMSegment, m_lastModRMOffset + 2);

    vlog(LogCPU, "Loaded far pointer (segment first) from %04X:%08X [PE=%u], got %04X:%08X", m_lastModRMSegment, m_lastModRMOffset, getPE(), ptr.segment, ptr.offset);

    return ptr;
}

FarPointer VCpu::readModRMFarPointerOffsetFirst(BYTE rmbyte)
{
    void* registerPointer = resolveModRM_internal(rmbyte, DWordSize);

    // FIXME: What should I do if it's a register? :|
    assert(!registerPointer);

    FarPointer ptr;
    ptr.segment = readMemory16(m_lastModRMSegment, m_lastModRMOffset + 4);
    ptr.offset = readMemory32(m_lastModRMSegment, m_lastModRMOffset);

    vlog(LogCPU, "Loaded far pointer (offset first) from %04X:%08X [PE=%u], got %04X:%08X", m_lastModRMSegment, m_lastModRMOffset, getPE(), ptr.segment, ptr.offset);

    return ptr;
}

void *VCpu::resolveModRM8_internal(BYTE rmbyte)
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
        m_lastModRMSegment = segment;
        m_lastModRMOffset = offset;
        m_lastModRMPointer = 0;
        break;
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
        m_lastModRMSegment = segment;
        m_lastModRMOffset = offset;
        m_lastModRMPointer = 0;
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
        m_lastModRMSegment = segment;
        m_lastModRMOffset = offset;
        m_lastModRMPointer = 0;
        break;
    default: // 0xC0
        switch (rmbyte & 0x07) {
        case 0: m_lastModRMPointer = &this->regs.B.AL; break;
        case 1: m_lastModRMPointer = &this->regs.B.CL; break;
        case 2: m_lastModRMPointer = &this->regs.B.DL; break;
        case 3: m_lastModRMPointer = &this->regs.B.BL; break;
        case 4: m_lastModRMPointer = &this->regs.B.AH; break;
        case 5: m_lastModRMPointer = &this->regs.B.CH; break;
        case 6: m_lastModRMPointer = &this->regs.B.DH; break;
        default: m_lastModRMPointer = &this->regs.B.BH; break;
        }
        break;
    }
    return m_lastModRMPointer;
}

void* VCpu::resolveModRM16_internal(BYTE rmbyte)
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
        m_lastModRMSegment = segment;
        m_lastModRMOffset = offset;
        m_lastModRMPointer = 0;
        break;
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
        m_lastModRMSegment = segment;
        m_lastModRMOffset = offset;
        m_lastModRMPointer = 0;
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
        m_lastModRMSegment = segment;
        m_lastModRMOffset = offset;
        m_lastModRMPointer = 0;
        break;
    default: // 0xC0
        switch (rmbyte & 0x07) {
        case 0: m_lastModRMPointer = &this->regs.W.AX; break;
        case 1: m_lastModRMPointer = &this->regs.W.CX; break;
        case 2: m_lastModRMPointer = &this->regs.W.DX; break;
        case 3: m_lastModRMPointer = &this->regs.W.BX; break;
        case 4: m_lastModRMPointer = &this->regs.W.SP; break;
        case 5: m_lastModRMPointer = &this->regs.W.BP; break;
        case 6: m_lastModRMPointer = &this->regs.W.SI; break;
        default: m_lastModRMPointer = &this->regs.W.DI; break;
        }
        break;
    }
    return m_lastModRMPointer;
}

void* VCpu::resolveModRM32_internal(BYTE rmbyte, ValueSize size)
{
    VM_ASSERT(a32());

    WORD segment = currentSegment();
    DWORD offset = 0x00000000;

    switch (rmbyte & 0xC0) {
    case 0x00:
        switch (rmbyte & 0x07) {
        case 0: offset = getEAX(); break;
        case 1: offset = getECX(); break;
        case 2: offset = getEDX(); break;
        case 3: offset = getEBX(); break;
        case 4: offset = evaluateSIB(rmbyte, fetchOpcodeByte()); break;
        case 5: offset = fetchOpcodeDWord(); break;
        case 6: offset = getESI(); break;
        default: offset = getEDI(); break;
        }
        m_lastModRMSegment = segment;
        m_lastModRMOffset = offset;
        m_lastModRMPointer = 0;
        break;
    case 0x40:
        switch (rmbyte & 0x07) {
        case 0: offset = getEAX() + vomit_signExtend<DWORD>(fetchOpcodeByte()); break;
        case 1: offset = getECX() + vomit_signExtend<DWORD>(fetchOpcodeByte()); break;
        case 2: offset = getEDX() + vomit_signExtend<DWORD>(fetchOpcodeByte()); break;
        case 3: offset = getEBX() + vomit_signExtend<DWORD>(fetchOpcodeByte()); break;
        case 4: offset = evaluateSIB(rmbyte, fetchOpcodeByte(), 8); break;
        case 5: DEFAULT_TO_SS; offset = getEBP() + vomit_signExtend<DWORD>(fetchOpcodeByte()); break;
        case 6: offset = getESI() + vomit_signExtend<DWORD>(fetchOpcodeByte()); break;
        default: offset = getEDI() + vomit_signExtend<DWORD>(fetchOpcodeByte()); break;
        }
        m_lastModRMSegment = segment;
        m_lastModRMOffset = offset;
        m_lastModRMPointer = 0;
        break;
    case 0x80:
        switch (rmbyte & 0x07) {
        case 0: offset = getEAX() + fetchOpcodeDWord(); break;
        case 1: offset = getECX() + fetchOpcodeDWord(); break;
        case 2: offset = getEDX() + fetchOpcodeDWord(); break;
        case 3: offset = getEBX() + fetchOpcodeDWord(); break;
        case 4: offset = evaluateSIB(rmbyte, fetchOpcodeByte(), 32); break;
        case 5: DEFAULT_TO_SS; offset = getEBP() + fetchOpcodeDWord(); break;
        case 6: offset = getESI() + fetchOpcodeDWord(); break;
        default: offset = getEDI() + fetchOpcodeDWord(); break;
        }
        m_lastModRMSegment = segment;
        m_lastModRMOffset = offset;
        m_lastModRMPointer = 0;
        break;
    default: // 0xC0
        if (size == DWordSize) {
            switch (rmbyte & 0x07) {
            case 0: m_lastModRMPointer = &this->regs.D.EAX; break;
            case 1: m_lastModRMPointer = &this->regs.D.ECX; break;
            case 2: m_lastModRMPointer = &this->regs.D.EDX; break;
            case 3: m_lastModRMPointer = &this->regs.D.EBX; break;
            case 4: m_lastModRMPointer = &this->regs.D.ESP; break;
            case 5: m_lastModRMPointer = &this->regs.D.EBP; break;
            case 6: m_lastModRMPointer = &this->regs.D.ESI; break;
            default: m_lastModRMPointer = &this->regs.D.EDI; break;
            }
        } else if (size == WordSize) {
            switch (rmbyte & 0x07) {
            case 0: m_lastModRMPointer = &this->regs.W.AX; break;
            case 1: m_lastModRMPointer = &this->regs.W.CX; break;
            case 2: m_lastModRMPointer = &this->regs.W.DX; break;
            case 3: m_lastModRMPointer = &this->regs.W.BX; break;
            case 4: m_lastModRMPointer = &this->regs.W.SP; break;
            case 5: m_lastModRMPointer = &this->regs.W.BP; break;
            case 6: m_lastModRMPointer = &this->regs.W.SI; break;
            default: m_lastModRMPointer = &this->regs.W.DI; break;
            }
        } else if (size == ByteSize) {
            switch (rmbyte & 0x07) {
            case 0: m_lastModRMPointer = &this->regs.B.AL; break;
            case 1: m_lastModRMPointer = &this->regs.B.CL; break;
            case 2: m_lastModRMPointer = &this->regs.B.DL; break;
            case 3: m_lastModRMPointer = &this->regs.B.BL; break;
            case 4: m_lastModRMPointer = &this->regs.B.AH; break;
            case 5: m_lastModRMPointer = &this->regs.B.CH; break;
            case 6: m_lastModRMPointer = &this->regs.B.DH; break;
            default: m_lastModRMPointer = &this->regs.B.BH; break;
            }
        }
        break;
    }
    return m_lastModRMPointer;
}

DWORD VCpu::evaluateSIB(BYTE rm, BYTE sib, unsigned sizeOfImmediate)
{
    unsigned scale = 1 << ((sib >> 6) & 3);
    RegisterIndex32 indexRegister = (RegisterIndex32)((sib >> 3) & 7);
    RegisterIndex32 baseRegister = (RegisterIndex32)(sib & 7);

    DWORD result = getRegister32(baseRegister) + getRegister32(indexRegister) * scale;

    DWORD scaledIndex;
    switch (sib & 0xC0) {
    case 0x00:
        switch ((sib >> 3) & 0x07) {
        case 0: scaledIndex = getEAX(); break;
        case 1: scaledIndex = getECX(); break;
        case 2: scaledIndex = getEDX(); break;
        case 3: scaledIndex = getEBX(); break;
        case 4: scaledIndex = 0; break;
        case 5: scaledIndex = getEBP(); break;
        case 6: scaledIndex = getESI(); break;
        default: scaledIndex = getEDI(); break;
        }
        break;
    case 0x40:
        switch ((sib >> 3) & 0x07) {
        case 0: scaledIndex = getEAX() * 2; break;
        case 1: scaledIndex = getECX() * 2; break;
        case 2: scaledIndex = getEDX() * 2; break;
        case 3: scaledIndex = getEBX() * 2; break;
        case 4: scaledIndex = 0; break;
        case 5: scaledIndex = getEBP() * 2; break;
        case 6: scaledIndex = getESI() * 2; break;
        default: scaledIndex = getEDI() * 2; break;
        }
        break;
    case 0x80:
        switch ((sib >> 3) & 0x07) {
        case 0: scaledIndex = getEAX() * 4; break;
        case 1: scaledIndex = getECX() * 4; break;
        case 2: scaledIndex = getEDX() * 4; break;
        case 3: scaledIndex = getEBX() * 4; break;
        case 4: scaledIndex = 0; break;
        case 5: scaledIndex = getEBP() * 4; break;
        case 6: scaledIndex = getESI() * 4; break;
        default: scaledIndex = getEDI() * 4; break;
        }
        break;
    default: // 0xC0
        switch ((sib >> 3) & 0x07) {
        case 0: scaledIndex = getEAX() * 8; break;
        case 1: scaledIndex = getECX() * 8; break;
        case 2: scaledIndex = getEDX() * 8; break;
        case 3: scaledIndex = getEBX() * 8; break;
        case 4: scaledIndex = 0; break;
        case 5: scaledIndex = getEBP() * 8; break;
        case 6: scaledIndex = getESI() * 8; break;
        default: scaledIndex = getEDI() * 8; break;
        }
        break;
    }

    DWORD base = 0;
    switch (sib & 0x07) {
    case 0: base = getEAX(); break;
    case 1: base = getECX(); break;
    case 2: base = getEDX(); break;
    case 3: base = getEBX(); break;
    case 4: base = getESP(); break;
    case 6: base = getESI(); break;
    case 7: base = getEDI(); break;
    default: // 5
        // FIXME: Uh, what to do if the ModR/M byte signalled a different size of immediate?
        sizeOfImmediate = 0;
        switch ((rm >> 6) & 3) {
        case 0: base = fetchOpcodeDWord(); break;
        case 1: base = vomit_signExtend<DWORD>(fetchOpcodeByte()) + getEBP(); break;
        case 2: base = fetchOpcodeDWord() + getEBP(); break;
        default: VM_ASSERT(false); break;
        }
        break;
    }

    if (sizeOfImmediate == 8)
        base = vomit_signExtend<DWORD>(fetchOpcodeByte());
    else if (sizeOfImmediate == 32)
        base = fetchOpcodeDWord();
    //dumpAll();
    //vlog(LogCPU, "%04X:%08X: evaluateSIB(): sib=%02X -> %s+%s*%u, scaledIndex:%08X, base:%08X", getBaseCS(), getBaseEIP(), sib, registerName(baseRegister), registerName(indexRegister), scale, scaledIndex, base);
    return scaledIndex + base;
}
