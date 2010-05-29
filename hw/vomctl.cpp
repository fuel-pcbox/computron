#include "vomctl.h"
#include "vcpu.h"
#include "debug.h"

static VomCtl the;

struct VomCtl::Private
{
    QString consoleWriteBuffer;
};

VomCtl::VomCtl()
    : IODevice("VomCtl")
    , d(new Private)
{
    m_registerIndex = 0;
    listen(0xD6, IODevice::ReadWrite);
    listen(0xD7, IODevice::ReadWrite);
}

VomCtl::~VomCtl()
{
    delete d;
    d = 0;
}

uint8_t
VomCtl::in8(WORD port)
{
    switch (port) {
    case 0xD6: // VOMCTL_REGISTER
        vlog(VM_VOMCTL, "Read register %02X", m_registerIndex);
        switch (m_registerIndex) {
        case 0x00: // Always 0
            return 0;
        case 0x01: // Get CPU type
            return VOMIT_CPU_LEVEL;
        case 0x02: // RAM size LSB
            return LSB(g_cpu->baseMemorySize() / 1024);
        case 0x03: // RAM size MSB
            return MSB(g_cpu->baseMemorySize() / 1024);
        }
        vlog(VM_VOMCTL, "Invalid register %02X read", m_registerIndex);
        break;
    case 0xD7: // VOMCTL_CONSOLE_WRITE
        vlog(VM_VOMCTL, "%s", d->consoleWriteBuffer.toLatin1().constData());
        d->consoleWriteBuffer.clear();
        break;
    }

    return 0;
}

void VomCtl::out8(WORD port, BYTE data)
{
    switch (port) {
    case 0xD6: // VOMCTL_REGISTER
        //vlog(VM_VOMCTL, "Select register %02X", data);
        m_registerIndex = data;
        break;
    case 0xD7: // VOMCTL_CONSOLE_WRITE
        d->consoleWriteBuffer += QChar::fromLatin1(data);
        break;
    }
}
