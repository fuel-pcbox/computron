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

#define DEFAULT_TO_SS if (!m_cpu->hasSegmentPrefix()) { m_segment = SegmentRegisterIndex::SS; }

SegmentRegisterIndex MemoryOrRegisterReference::segment()
{
    ASSERT(!isRegister());
    return m_segment;
}

DWORD MemoryOrRegisterReference::offset()
{
    ASSERT(!isRegister());
    if (m_a32)
        return m_offset32;
    else
        return m_offset16;
}

void* MemoryOrRegisterReference::memoryPointer()
{
    ASSERT(m_cpu);
    ASSERT(!isRegister());
    return m_cpu->memoryPointer(segment(), offset());
}

template<typename T>
T MemoryOrRegisterReference::read()
{
    ASSERT(m_cpu);
    if (isRegister())
        return m_cpu->readRegister<T>(m_registerIndex);
    return m_cpu->readMemory<T>(segment(), offset());
}

template BYTE MemoryOrRegisterReference::read<BYTE>();
template WORD MemoryOrRegisterReference::read<WORD>();
template DWORD MemoryOrRegisterReference::read<DWORD>();

template<typename T>
void MemoryOrRegisterReference::write(T data)
{
    ASSERT(m_cpu);
    if (isRegister()) {
        m_cpu->writeRegister<T>(m_registerIndex, data);
        return;
    }
    m_cpu->writeMemory<T>(segment(), offset(), data);
}

template void MemoryOrRegisterReference::write<BYTE>(BYTE);
template void MemoryOrRegisterReference::write<WORD>(WORD);
template void MemoryOrRegisterReference::write<DWORD>(DWORD);

BYTE MemoryOrRegisterReference::read8() { return read<BYTE>(); }
WORD MemoryOrRegisterReference::read16() { return read<WORD>(); }
DWORD MemoryOrRegisterReference::read32() { ASSERT(m_cpu->o32()); return read<DWORD>(); }
void MemoryOrRegisterReference::write8(BYTE data) { return write(data); }
void MemoryOrRegisterReference::write16(WORD data) { return write(data); }
void MemoryOrRegisterReference::write32(DWORD data) { ASSERT(m_cpu->o32()); return write(data); }

void MemoryOrRegisterReference::writeClearing16(WORD data, bool o32)
{
    if (o32 && isRegister()) {
        m_cpu->writeRegister<DWORD>(m_registerIndex, data);
        return;
    }
    return write(data);
}

FLATTEN void MemoryOrRegisterReference::resolve(CPU& cpu)
{
    m_cpu = &cpu;
    ASSERT(m_cpu->a32() == m_a32);
    if (m_a32)
        return resolve32();
    return resolve16();
}

FarPointer CPU::readModRMFarPointer(MemoryOrRegisterReference& modrm)
{
    ASSERT(!modrm.isRegister());

    FarPointer ptr;
    ptr.segment = readMemory16(modrm.segment(), modrm.offset() + 4);
    ptr.offset = readMemory32(modrm.segment(), modrm.offset());

#ifdef MODRM_DEBUG
    vlog(LogCPU, "Loaded far pointer from %s:%08x [PE=%u, PG=%u], got %04x:%08x", registerName(modrm.segment()), modrm.offset(), getPE(), getPG(), ptr.segment, ptr.offset);
#endif

    return ptr;
}

FLATTEN void MemoryOrRegisterReference::decode(InstructionStream& stream, bool a32)
{
    m_a32 = a32;
    m_rm = stream.readInstruction8();

    if (m_a32) {
        decode32(stream);
        switch (m_displacementBytes) {
        case 0: break;
        case 1: m_displacement32 = signExtend<DWORD>(stream.readInstruction8()); break;
        case 4: m_displacement32 = stream.readInstruction32(); break;
        default: ASSERT_NOT_REACHED(); break;
        }
    } else {
        decode16(stream);
        switch (m_displacementBytes) {
        case 0: break;
        case 1: m_displacement16 = signExtend<WORD>(stream.readInstruction8()); break;
        case 2: m_displacement16 = stream.readInstruction16(); break;
        default: ASSERT_NOT_REACHED(); break;
        }
    }
}

ALWAYS_INLINE void MemoryOrRegisterReference::decode16(InstructionStream&)
{
    ASSERT(!m_a32);

    switch (m_rm & 0xc0) {
    case 0:
        if ((m_rm & 0x07) == 6)
            m_displacementBytes = 2;
        else
            ASSERT(m_displacementBytes == 0);
        break;
    case 0x40:
        m_displacementBytes = 1;
        break;
    case 0x80:
        m_displacementBytes = 2;
        break;
    case 0xc0:
        m_registerIndex = m_rm & 7;
        break;
    }
}

ALWAYS_INLINE void MemoryOrRegisterReference::decode32(InstructionStream& stream)
{
    ASSERT(m_a32);

    switch (m_rm & 0xc0) {
    case 0:
        if ((m_rm & 0x07) == 5)
            m_displacementBytes = 4;
        break;
    case 0x40:
        m_displacementBytes = 1;
        break;
    case 0x80:
        m_displacementBytes = 4;
        break;
    case 0xc0:
        m_registerIndex = m_rm & 7;
        return;
    }

    m_hasSIB = (m_rm & 0x07) == 4;
    if (m_hasSIB) {
        m_sib = stream.readInstruction8();
        if ((m_sib & 0x07) == 5) {
            switch ((m_rm >> 6) & 0x03) {
            case 0: ASSERT(!m_displacementBytes || m_displacementBytes == 4); m_displacementBytes = 4; break;
            case 1: ASSERT(!m_displacementBytes || m_displacementBytes == 1); m_displacementBytes = 1; break;
            case 2: ASSERT(!m_displacementBytes || m_displacementBytes == 4); m_displacementBytes = 4; break;
            default: ASSERT_NOT_REACHED(); break;
            }
        }
    }
}

ALWAYS_INLINE void MemoryOrRegisterReference::resolve16()
{
    ASSERT(m_cpu);
    ASSERT(!m_a32);
    ASSERT(m_cpu->a16());

    m_segment = m_cpu->currentSegment();

    switch (m_rm & 7) {
    case 0: m_offset16 = m_cpu->getBX() + m_cpu->getSI() + m_displacement16; break;
    case 1: m_offset16 = m_cpu->getBX() + m_cpu->getDI() + m_displacement16; break;
    case 2: DEFAULT_TO_SS; m_offset16 = m_cpu->getBP() + m_cpu->getSI() + m_displacement16; break;
    case 3: DEFAULT_TO_SS; m_offset16 = m_cpu->getBP() + m_cpu->getDI() + m_displacement16; break;
    case 4: m_offset16 = m_cpu->getSI() + m_displacement16; break;
    case 5: m_offset16 = m_cpu->getDI() + m_displacement16; break;
    case 6:
        if ((m_rm & 0xc0) == 0)
            m_offset16 = m_displacement16;
        else {
            DEFAULT_TO_SS;
            m_offset16 = m_cpu->getBP() + m_displacement16;
        }
        break;
    default: m_offset16 = m_cpu->getBX() + m_displacement16; break;
    }
}

ALWAYS_INLINE void MemoryOrRegisterReference::resolve32()
{
    ASSERT(m_cpu);
    ASSERT(m_a32);
    ASSERT(m_cpu->a32());

    m_segment = m_cpu->currentSegment();

    switch (m_rm & 0x07) {
    case 0: m_offset32 = m_cpu->getEAX() + m_displacement32; break;
    case 1: m_offset32 = m_cpu->getECX() + m_displacement32; break;
    case 2: m_offset32 = m_cpu->getEDX() + m_displacement32; break;
    case 3: m_offset32 = m_cpu->getEBX() + m_displacement32; break;
    case 4: m_offset32 = evaluateSIB(); break;
    case 6: m_offset32 = m_cpu->getESI() + m_displacement32; break;
    case 7: m_offset32 = m_cpu->getEDI() + m_displacement32; break;
    default: // 5
        if ((m_rm & 0xc0) == 0x00) {
            m_offset32 = m_displacement32; break;
        } else {
            DEFAULT_TO_SS;
            m_offset32 = m_cpu->getEBP() + m_displacement32; break;
        }
        break;
    }
}

DWORD MemoryOrRegisterReference::evaluateSIB()
{
    DWORD scale;
    switch (m_sib & 0xC0) {
    case 0x00: scale = 1; break;
    case 0x40: scale = 2; break;
    case 0x80: scale = 4; break;
    case 0xC0: scale = 8; break;
    }
    DWORD index;
    switch ((m_sib >> 3) & 0x07) {
    case 0: index = m_cpu->getEAX(); break;
    case 1: index = m_cpu->getECX(); break;
    case 2: index = m_cpu->getEDX(); break;
    case 3: index = m_cpu->getEBX(); break;
    case 4: index = 0; break;
    case 5: /* FIXME: DEFAULT_TO_SS here? */ index = m_cpu->getEBP(); break;
    case 6: index = m_cpu->getESI(); break;
    case 7: index = m_cpu->getEDI(); break;
    }
    DWORD scaledIndex = index * scale;

    DWORD base = m_displacement32;
    switch (m_sib & 0x07) {
    case 0: base += m_cpu->getEAX(); break;
    case 1: base += m_cpu->getECX(); break;
    case 2: base += m_cpu->getEDX(); break;
    case 3: base += m_cpu->getEBX(); break;
    case 4: DEFAULT_TO_SS; base += m_cpu->getESP(); break;
    case 6: base += m_cpu->getESI(); break;
    case 7: base += m_cpu->getEDI(); break;
    default: // 5
        switch ((m_rm >> 6) & 3) {
        case 0: break;
        case 1:
        case 2: DEFAULT_TO_SS; base += m_cpu->getEBP(); break;
        default: ASSERT_NOT_REACHED(); break;
        }
        break;
    }

    return scaledIndex + base;
}
