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

    void out8(WORD port, BYTE data);
    BYTE in8(WORD port);

private:
    BYTE m_statusRegisterB;
    BYTE m_registerIndex;

    bool inBinaryClockMode() const;
    bool in24HourMode() const;
};

}

#endif
