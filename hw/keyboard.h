#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "iodevice.h"

class Keyboard : public IODevice
{
public:
    Keyboard();
    ~Keyboard();

    BYTE in8(WORD port);
    void out8(WORD port, BYTE data);

    static void raiseIRQ();

private:
    BYTE m_systemControlPortData;
    BYTE m_ram[32];
    BYTE m_command;
    bool m_hasCommand;
};

#endif
