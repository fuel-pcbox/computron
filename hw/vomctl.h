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

    virtual void out8(BYTE);
    virtual BYTE in8();

private:
    BYTE m_registerIndex;
    VCpu* m_cpu;
};

}

#endif
