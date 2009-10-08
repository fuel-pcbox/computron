#ifndef __vomctl_h__
#define __vomctl_h__

#include "iodevice.h"

struct __vomit_cpu_t;

namespace Vomit
{

class VomCtl : public Vomit::IODevice
{
public:
    VomCtl();
    virtual ~VomCtl();

    virtual void out8( uint8_t );
    virtual uint8_t in8();

private:
    uint8_t m_registerIndex;
    __vomit_cpu_t *m_cpu;
};

}

#endif
