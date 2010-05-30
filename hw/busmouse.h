#ifndef BUSMOUSE_H
#define BUSMOUSE_H

#include "iodevice.h"
#include <QtCore/QMutex>

class BusMouse : public IODevice
{
public:
    BusMouse();
    virtual ~BusMouse();

    virtual void out8(WORD port, BYTE data);
    virtual BYTE in8(WORD port);

    enum Button { LeftButton, RightButton };

    void moveEvent(WORD x, WORD y);
    void buttonPressEvent(WORD x, WORD y, Button button);
    void buttonReleaseEvent(WORD x, WORD y, Button button);

    static BusMouse* the();

private:
    bool m_interrupts;
    BYTE m_command;
    BYTE m_buttons;

    WORD m_currentX;
    WORD m_currentY;
    WORD m_lastX;
    WORD m_lastY;
    WORD m_deltaX;
    WORD m_deltaY;

    QMutex m_mutex;
};

#endif
