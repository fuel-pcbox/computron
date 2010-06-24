#include "vomit.h"
#include "vcpu.h"
#include "debug.h"

#define DEFAULT_TO_SS if (!hasSegmentPrefix()) { segment = getSS(); }

void VCpu::writeModRM32(BYTE rmbyte, DWORD value)
{
    DWORD* registerPointer = reinterpret_cast<DWORD*>(resolveModRM32(rmbyte));
    if (registerPointer)
        *registerPointer = value;
    else
        writeMemory32(m_lastModRMSegment, m_lastModRMOffset, value);
}

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
    DWORD* registerPointer = reinterpret_cast<DWORD*>(resolveModRM32(rmbyte));

    if (registerPointer)
        return *registerPointer;

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

void* VCpu::resolveModRM32(BYTE rmbyte)
{
    WORD segment = currentSegment();
    WORD offset = 0x00000000;

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
