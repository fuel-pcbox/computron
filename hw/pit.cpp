#include "vomit.h"
#include "vcpu.h"
#include "pit.h"

static PIT thePIT;

PIT::PIT()
    : IODevice("PIT")
{
    listen(0x40, IODevice::ReadOnly);
    listen(0x42, IODevice::WriteOnly);
    listen(0x43, IODevice::WriteOnly);
}

PIT::~PIT()
{
}

BYTE PIT::in8(WORD)
{
    return rand() % 0xff;
}

void PIT::out8(WORD port, BYTE data)
{
    switch (port) {
    case 0x43:
        // yeah
        break;
    default:
        break;
    }
}
