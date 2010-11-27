#ifndef PIC_H
#define PIC_H

#include "iodevice.h"
#include <QtCore/QMutex>

class VCpu;

class PIC : public IODevice
{
public:
    PIC(WORD baseAddress, BYTE isrBase);
    ~PIC();

    void out8(WORD port, BYTE data);
    BYTE in8(WORD port);

    void raise(BYTE num);

    BYTE getIMR() const { return m_imr; }
    BYTE getIRR() const { return m_irr; }
    BYTE getISR() const { return m_isr; }

    static bool hasPendingIRQ();
    static void serviceIRQ(VCpu*);
    static void raiseIRQ(BYTE num);

private:
    QMutex m_mutex;

    WORD m_baseAddress;
    BYTE m_isrBase;

    BYTE m_isr;
    BYTE m_irr;
    BYTE m_imr;

    bool m_icw2Expected;
    bool m_readIRR;

    static WORD s_pendingRequests;
    static void updatePendingRequests();
    static QMutex s_mutex;
};

#endif
