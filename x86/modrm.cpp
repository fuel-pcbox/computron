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

#include "vomit.h"
#include "vcpu.h"
#include "debug.h"

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
        return reinterpret_cast<DWORD*>(resolveModRM32_internal(rmbyte));

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
        offset = vomit_signExtend(fetchOpcodeByte());
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
        offset = vomit_signExtend(fetchOpcodeByte());
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

void* VCpu::resolveModRM32_internal(BYTE rmbyte)
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
        case 4: offset = evaluateSIB(fetchOpcodeByte()); break;
        case 5: offset = fetchOpcodeDWord(); break;
        case 6: offset = getESI(); break;
        default: offset = getEDI(); break;
        }
        m_lastModRMSegment = segment;
        m_lastModRMOffset = offset;
        m_lastModRMPointer = 0;
        break;
    case 0x40:
        offset = vomit_signExtend(fetchOpcodeByte());
        switch (rmbyte & 0x07) {
        case 0: offset += getEAX(); break;
        case 1: offset += getECX(); break;
        case 2: offset += getEDX(); break;
        case 3: offset += getEBX(); break;
        case 4: offset += evaluateSIB(fetchOpcodeByte()); break;
        case 5: DEFAULT_TO_SS; offset += getEBP(); break;
        case 6: offset += getESI(); break;
        default: offset += getEDI(); break;
        }
        m_lastModRMSegment = segment;
        m_lastModRMOffset = offset;
        m_lastModRMPointer = 0;
        break;
    case 0x80:
        offset = fetchOpcodeWord();
        switch (rmbyte & 0x07) {
        case 0: offset += getEAX(); break;
        case 1: offset += getECX(); break;
        case 2: offset += getEDX(); break;
        case 3: offset += getEBX(); break;
        case 4: offset += evaluateSIB(fetchOpcodeByte()); break;
        case 5: DEFAULT_TO_SS; offset += getEBP(); break;
        case 6: offset += getESI(); break;
        default: offset += getEDI(); break;
        }
        m_lastModRMSegment = segment;
        m_lastModRMOffset = offset;
        m_lastModRMPointer = 0;
        break;
    default: // 0xC0
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
        break;
    }
    return m_lastModRMPointer;
}

DWORD VCpu::evaluateSIB(BYTE sib)
{
    vlog(VM_ALERT, "evaluateSIB() called.. this is not properly implemented :(");
    vm_exit(1);

    switch (sib & 0xC0) {
    case 0x00:
        switch (rmbyte & 0x07) {
        case 0: return getEAX();
        case 1: return getECX();
        case 2: return getEDX();
        case 3: return getEBX();
        case 4: return 0;
        case 5: return getEBP();
        case 6: return getESI();
        default: return getEDI();
        }
        break;
    case 0x40:
        switch (rmbyte & 0x07) {
        case 0: return getEAX() * 2;
        case 1: return getECX() * 2;
        case 2: return getEDX() * 2;
        case 3: return getEBX() * 2;
        case 4: return 0;
        case 5: return getEBP() * 2;
        case 6: return getESI() * 2;
        default: return getEDI() * 2;
        }
        break;
    case 0x80:
        switch (rmbyte & 0x07) {
        case 0: return getEAX() * 4;
        case 1: return getECX() * 4;
        case 2: return getEDX() * 4;
        case 3: return getEBX() * 4;
        case 4: return 0;
        case 5: return getEBP() * 4;
        case 6: return getESI() * 4;
        default: return getEDI() * 4;
        }
        break;
    default: // 0xC0
        switch (rmbyte & 0x07) {
        case 0: return getEAX() * 8;
        case 1: return getECX() * 8;
        case 2: return getEDX() * 8;
        case 3: return getEBX() * 8;
        case 4: return 0;
        case 5: return getEBP() * 8;
        case 6: return getESI() * 8;
        default: return getEDI() * 8;
        }
        break;
    }
}
