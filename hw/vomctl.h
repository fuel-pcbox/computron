#ifndef VOMCTL_H
#define VOMCTL_H

#include "iodevice.h"

class VCpu;

namespace Vomit
{

class VomCtl : public Vomit::IODevice
{
public:
    VomCtl();
    virtual ~VomCtl();

    virtual void out8(WORD port, BYTE data);
    virtual BYTE in8(WORD port);

private:
    BYTE m_registerIndex;
    VCpu* m_cpu;

    struct Private;
    Private* d;
};

}

#endif
