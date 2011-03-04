#ifndef VGA_H
#define VGA_H

#include "iodevice.h"
#include <QtGui/QColor>

class VGA : public IODevice
{
public:
    VGA();
    virtual ~VGA();

    virtual BYTE in8(WORD port);
    virtual void out8(WORD port, BYTE data);

    void setPaletteDirty(bool);
    bool isPaletteDirty();

    BYTE readRegister(BYTE index);
    BYTE readRegister2(BYTE index);
    BYTE readSequencer(BYTE index);

    void writeRegister(BYTE index, BYTE value);

    QColor color(int index) const;
    QColor paletteColor(int paletteIndex) const;

    static VGA* the();

private:
    struct Private;
    Private* d;

    static VGA* s_the;
};

#endif
