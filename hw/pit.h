#ifndef PIT_H
#define PIT_H

#include "iodevice.h"

class PIT : public IODevice
{
public:
    PIT();
    ~PIT();

    BYTE in8(WORD port);
    void out8(WORD port, BYTE data);

private:
};

#endif
