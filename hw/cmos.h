#ifndef __cmos_h__
#define __cmos_h__

#include "iodevice.h"
#include "vomit.h"

namespace Vomit
{

class CMOS : public Vomit::IODevice
{
public:
    CMOS();
    ~CMOS();

    void out8(BYTE);
    BYTE in8();

private:
    BYTE m_register[256];
    BYTE m_registerIndex;
};

}

#endif
