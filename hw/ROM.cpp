#include "ROM.h"
#include "Common.h"

ROM::ROM(DWORD baseAddress, const QString& fileName)
    : m_file(fileName)
    , m_baseAddress(baseAddress)
{
    vlog(LogConfig, "Build ROM for %08x with file %s", baseAddress, qPrintable(fileName));
    if (m_file.open(QIODevice::ReadOnly)) {
        m_mmap = m_file.map(0, m_file.size());
    }
}

ROM::~ROM()
{
}

bool ROM::isValid() const
{
    return m_mmap;
}

DWORD ROM::length() const
{
    if (!isValid())
        return 0;
    if (m_file.size() > 0xffffffffLL) {
        ASSERT_NOT_REACHED();
    }
    return m_file.size();
}

ALWAYS_INLINE bool ROM::translateToOffset(DWORD address, DWORD& offset)
{
#ifndef NDEBUG
    if (address < m_baseAddress)
        return false;
    if ((address - m_baseAddress) >= length())
        return false;
#endif
    offset = address - m_baseAddress;
    return true;
}

BYTE ROM::read8(DWORD address)
{
    DWORD offset;
    if (!translateToOffset(address, offset))
        return 0;
    return m_mmap[offset];
}

WORD ROM::read16(DWORD address)
{
    DWORD offset;
    if (!translateToOffset(address, offset))
        return 0;
    return *reinterpret_cast<WORD*>(&m_mmap[offset]);
}

DWORD ROM::read32(DWORD address)
{
    DWORD offset;
    if (!translateToOffset(address, offset))
        return 0;
    return *reinterpret_cast<DWORD*>(&m_mmap[offset]);
}

void ROM::write8(DWORD address, BYTE data)
{
    vlog(LogAlert, "Write to ROM{%s} address %08x, data %02x", qPrintable(m_file.fileName()), address, data);
}

BYTE* ROM::memoryPointer(DWORD address)
{
    DWORD offset;
    if (!translateToOffset(address, offset))
        return 0x00;
    return &m_mmap[offset];
}
