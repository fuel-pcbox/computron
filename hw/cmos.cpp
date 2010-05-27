#include "cmos.h"
#include "debug.h"
#include <QtCore/QDate>
#include <QtCore/QTime>

namespace Vomit
{

static CMOS the;

CMOS::CMOS()
    : IODevice("CMOS")
{
    m_registerIndex = 0;
    listen(0x70, Vomit::IODevice::Write);
    listen(0x71, Vomit::IODevice::ReadWrite);
}

CMOS::~CMOS()
{
}

BYTE CMOS::in8(WORD)
{
    BYTE value = 0;

    switch (m_registerIndex) {
    case 0x00: value = QTime::currentTime().second(); break;
    case 0x02: value = QTime::currentTime().minute(); break;
    case 0x04: value = QTime::currentTime().hour(); break;
    case 0x06: value = QDate::currentDate().dayOfWeek(); break;
    case 0x07: value = QDate::currentDate().day(); break;
    case 0x08: value = QDate::currentDate().month(); break;
    case 0x09: value = QDate::currentDate().year() % 100; break;
    case 0x15: value = LSB(g_cpu->baseMemorySize() / 1024); break;
    case 0x16: value = MSB(g_cpu->baseMemorySize() / 1024); break;
    default: vlog(VLOG_CMOS, "WARNING: Read unsupported register %02X", m_registerIndex);
    }

    vlog(VLOG_CMOS, "Read register %02X (%02X)", m_registerIndex, value);
    return value;
}

void CMOS::out8(WORD, BYTE data)
{
    vlog(VLOG_CMOS, "Select register %02X", data);
    m_registerIndex = data;
}

}
