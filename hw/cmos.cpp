#include "cmos.h"
#include "debug.h"

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
