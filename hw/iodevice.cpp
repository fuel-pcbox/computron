#include "iodevice.h"
#include "debug.h"
#include <QList>

QList<IODevice*>& IODevice::devices()
{
    static QList<IODevice*> s_devices;
    return s_devices;
}

QHash<uint16_t, IODevice*>& IODevice::readDevices()
{
    static QHash<uint16_t, IODevice*> s_readDevices;
    return s_readDevices;
}

QHash<uint16_t, IODevice*>& IODevice::writeDevices()
{
    static QHash<uint16_t, IODevice*> s_writeDevices;
    return s_writeDevices;
}

IODevice::IODevice(const char* name)
    : m_name(name)
{
    devices().append(this);
}

IODevice::~IODevice()
{
    devices().removeAll(this);
}

void IODevice::listen(uint16_t port, ListenMask mask)
{
    if (mask & Read)
        readDevices()[port] = this;

    if (mask & Write)
        writeDevices()[port] = this;

    m_ports.append(port);
}

QList<uint16_t> IODevice::ports() const
{
    return m_ports;
}

const char* IODevice::name() const
{
    return m_name;
}

void IODevice::out8(WORD port, BYTE data)
{
    vlog(VM_ALERT, "FIXME: IODevice[%s]::out8(%04X, %02X)", m_name, port, data);
}

BYTE IODevice::in8(WORD port)
{
    vlog(VM_ALERT, "FIXME: IODevice[%s]::in8(%04X)", m_name, port);
    return IODevice::JunkValue;
}
