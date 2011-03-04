#ifndef FDC_H
#define FDC_H

#include "iodevice.h"

class FDC : public IODevice
{
public:
    FDC();
    virtual ~FDC();

    virtual BYTE in8(WORD port);
    virtual void out8(WORD port, BYTE data);

    static FDC* the();

private:
    void executeCommand();
    void raiseIRQ();

    struct Private;
    Private* d;
};

#endif
