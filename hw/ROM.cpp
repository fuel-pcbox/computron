#include "ROM.h"
#include "Common.h"
#include <QFile>

ROM::ROM(PhysicalAddress baseAddress, const QString& fileName)
    : MemoryProvider(baseAddress)
{
    QFile file(fileName);
    vlog(LogConfig, "Build ROM for %08x with file %s", baseAddress, qPrintable(fileName));
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    m_data = file.readAll();
    setSize(m_data.size());
    m_pointerForDirectReadAccess = reinterpret_cast<const BYTE*>(m_data.data());
}

ROM::~ROM()
{
}

bool ROM::isValid() const
{
    return !m_data.isNull();
}

BYTE ROM::read8(DWORD address)
{
    return m_data.data()[address - baseAddress().get()];
}

void ROM::write8(DWORD address, BYTE data)
{
    vlog(LogAlert, "Write to ROM address %08x, data %02x", address, data);
}

// FIXME: This mutable pointer is obviously a ROM violation. Don't vend this.
BYTE* ROM::memoryPointer(DWORD address)
{
    return reinterpret_cast<BYTE*>(&m_data.data()[address - baseAddress().get()]);
}
