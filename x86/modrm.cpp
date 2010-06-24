#include "vomit.h"
#include "vcpu.h"
#include "debug.h"

#define DEFAULT_TO_SS if (!hasSegmentPrefix()) { segment = getSS(); }

void VCpu::writeModRM16(BYTE rmbyte, WORD value)
{
    WORD* registerPointer = reinterpret_cast<WORD*>(resolveModRM16(rmbyte));
    if (registerPointer)
        *registerPointer = value;
    else
        writeMemory16(m_lastModRMSegment, m_lastModRMOffset, value);
}

void VCpu::writeModRM8(BYTE rmbyte, BYTE value)
{
    BYTE* registerPointer = reinterpret_cast<BYTE*>(resolveModRM8(rmbyte));
    if (registerPointer)
        *registerPointer = value;
    else
        writeMemory8(m_lastModRMSegment, m_lastModRMOffset, value);
}

WORD VCpu::readModRM16(BYTE rmbyte)
{
    WORD* registerPointer = reinterpret_cast<WORD*>(resolveModRM16(rmbyte));
    if (registerPointer)
        return *registerPointer;
    return readMemory16(m_lastModRMSegment, m_lastModRMOffset);
}

BYTE VCpu::readModRM8(BYTE rmbyte)
{
    BYTE* registerPointer = reinterpret_cast<BYTE*>(resolveModRM8(rmbyte));
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
    // NOTE: We don't need resolveModRM32() at the moment.
    BYTE* registerPointer = reinterpret_cast<BYTE*>(resolveModRM8(rmbyte));

    if (registerPointer)
        return *((DWORD*)registerPointer);

    return readMemory32(m_lastModRMSegment, m_lastModRMOffset);
}

void *VCpu::resolveModRM8(BYTE rmbyte)
{
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

void* VCpu::resolveModRM16(BYTE rmbyte)
{
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
