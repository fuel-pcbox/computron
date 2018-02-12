/*
 * Copyright (C) 2003-2018 Andreas Kling <awesomekling@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ANDREAS KLING ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANDREAS KLING OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "vomit.h"
#include "vcpu.h"
#include "keyboard.h"
#include "pic.h"
#include "debug.h"

#define ATKBD_PARITY_ERROR  0x80
#define ATKBD_TIMEOUT       0x40
#define ATKBD_BUFFER_FULL   0x20
#define ATKBD_UNLOCKED      0x10
#define ATKBD_CMD_DATA      0x08
#define ATKBD_SYSTEM_FLAG   0x04
#define ATKBD_INPUT_STATUS  0x02
#define ATKBD_OUTPUT_STATUS 0x01

#define CCB_KEYBOARD_INTERRUPT_ENABLE 0x01
#define CCB_MOUSE_INTERRUPT_ENABLE    0x02
#define CCB_SYSTEM_FLAG               0x04
#define CCB_IGNORE_KEYBOARD_LOCK      0x08
#define CCB_KEYBOARD_ENABLE           0x10
#define CCB_MOUSE_ENABLE              0x20
#define CCB_TRANSLATE                 0x40

#define CMD_NO_COMMAND                0x00
#define CMD_READ_CCB                  0x20
#define CMD_WRITE_CCB                 0x60
#define CMD_SET_LEDS                  0xED

Keyboard::Keyboard(Machine& machine)
    : IODevice("Keyboard", machine)
{
    listen(0x60, IODevice::ReadWrite);
    listen(0x61, IODevice::ReadWrite);
    listen(0x64, IODevice::ReadWrite);

    reset();
}

Keyboard::~Keyboard()
{
}

void Keyboard::reset()
{
    memset(m_ram, 0, sizeof(m_ram));

    m_systemControlPortData = 0;
    m_command = 0x00;
    m_hasCommand = false;
    m_lastWasCommand = false;

    m_leds = 0;

    m_ram[0] |= CCB_SYSTEM_FLAG;

    // FIXME: The BIOS should do this, no?
    m_ram[0] |= CCB_KEYBOARD_ENABLE;
    m_ram[0] |= CCB_KEYBOARD_INTERRUPT_ENABLE;
}

BYTE Keyboard::in8(WORD port)
{
    extern BYTE kbd_pop_raw();

    if (port == 0x60) {
        if (m_hasCommand && m_command <= 0x3F) {
            BYTE ramIndex = m_command & 0x3F;
            m_hasCommand = false;
            vlog(LogKeyboard, "Reading 8042 RAM [%02] = %02X", ramIndex, m_ram[ramIndex]);
            return m_ram[ramIndex];
        }
        if (m_lastWasCommand && m_command == CMD_SET_LEDS) {
            return 0xFA; // ACK
        }

        BYTE key = kbd_pop_raw();
        //vlog(LogKeyboard, "keyboard_data = %02X", key);
        return key;
    }

    if (port == 0x64) {
        // Keyboard not locked, POST completed successfully.
        BYTE status = ATKBD_UNLOCKED | (m_ram[0] & ATKBD_SYSTEM_FLAG);
        status |= m_lastWasCommand ? ATKBD_CMD_DATA : 0;
        //status |= m_hasCommand ? ATKBD_OUTPUT_STATUS : 0;
        // vlog(LogKeyboard, "Keyboard status queried (%02X)", status);
        return status;
    }

    if (port == 0x61) {
        // HACK: This should be implemented properly in the 8254 emulation.
        if (m_systemControlPortData & 0x10)
            m_systemControlPortData &= ~0x10;
        else
            m_systemControlPortData |= 0x10;
        return m_systemControlPortData;
    }

    return IODevice::in8(port);
}

void Keyboard::out8(WORD port, BYTE data)
{
    if (port == 0x61) {
        //vlog(LogKeyboard, "System control port <- %02X", data);
        m_systemControlPortData = data;
        return;
    }

    if (port == 0x64) {
        vlog(LogKeyboard, "Keyboard command <- %02X", data);
        m_command = data;
        m_hasCommand = true;
        m_lastWasCommand = true;
        return;
    }

    if (port == 0x60) {
        m_lastWasCommand = false;
        if (!m_hasCommand) {
            if (data == CMD_SET_LEDS) {
                m_command = data;
                m_hasCommand = true;
                m_lastWasCommand = true;
                return;
            }
            vlog(LogKeyboard, "Got data (%02X) without command", data);
            return;
        }

        m_hasCommand = false;

        if (m_command == CMD_SET_LEDS) {
            vlog(LogKeyboard, "LEDs set to %02X\n", data);
            if (m_leds != data) {
                m_leds = data;
                emit ledsChanged(m_leds);
            }
            return;
        }

        if (m_command == 0xD1) {
            vlog(LogKeyboard, "Write output port: A20=%s", (data & 0x02) ? "on" : "off");
            g_cpu->setA20Enabled(data & 0x02);
            return;
        }

        if (m_command >= 0x60 && m_command <= 0x7F) {
            BYTE ramIndex = m_command & 0x3F;
            m_ram[ramIndex] = data;

            switch (ramIndex) {
            case 0:
                vlog(LogKeyboard, "Controller Command Byte set:", data);
                vlog(LogKeyboard, "  Keyboard interrupt: %s", data & CCB_KEYBOARD_INTERRUPT_ENABLE ? "enabled" : "disabled");
                vlog(LogKeyboard, "  Mouse interrupt:    %s", data & CCB_MOUSE_INTERRUPT_ENABLE ? "enabled" : "disabled");
                vlog(LogKeyboard, "  System flag:        %s", data & CCB_SYSTEM_FLAG ? "enabled" : "disabled");
                vlog(LogKeyboard, "  Keyboard enable:    %s", data & CCB_KEYBOARD_ENABLE ? "enabled" : "disabled");
                vlog(LogKeyboard, "  Mouse enable:       %s", data & CCB_MOUSE_ENABLE ? "enabled" : "disabled");
                vlog(LogKeyboard, "  Translation:        %s", data & CCB_TRANSLATE ? "enabled" : "disabled");
                break;
            default:
                vlog(LogKeyboard, "Writing 8042 RAM [%02] = %02X", ramIndex, data);
            }
            return;
        }
        vlog(LogKeyboard, "Got data %02X for unknown command %02X", data, m_command);
        return;
    }

    IODevice::out8(port, data);
}

void Keyboard::raiseIRQ()
{
    if (m_ram[0] & CCB_KEYBOARD_INTERRUPT_ENABLE)
        PIC::raiseIRQ(machine(), 1);
}
