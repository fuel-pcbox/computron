#ifndef __iodevice_h__
#define __iodevice_h__

#include "types.h"
#include <QList>
#include <QHash>

class IODevice
{
public:
    IODevice(const char* name);
    virtual ~IODevice();

    const char* name() const;

    virtual BYTE in8(WORD port);
    virtual void out8(WORD port, BYTE data);

    static QList<IODevice*>& devices();
    static QHash<WORD, IODevice*>& readDevices();
    static QHash<WORD, IODevice*>& writeDevices();

    QList<WORD> ports() const;

    enum { JunkValue = 0xAA };

protected:
    enum ListenMask {
        ReadOnly = 1,
        WriteOnly = 2,
        ReadWrite = 3
    };
    virtual void listen(WORD port, ListenMask mask);

private:
    const char* m_name;
    QList<WORD> m_ports;
};

#endif
