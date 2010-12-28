/*
 * Basic AT keyboard controller
 * for VOMIT :^)
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

static Keyboard theKeyboard;

Keyboard::Keyboard()
    : IODevice("Keyboard")
{
    memset(m_ram, 0, sizeof(m_ram));

    m_systemControlPortData = 0;
    m_command = 0x00;
    m_hasCommand = false;
    m_lastWasCommand = false;

    m_ram[0] |= CCB_SYSTEM_FLAG;

    // FIXME: The BIOS should do this, no?
    m_ram[0] |= CCB_KEYBOARD_ENABLE;
    m_ram[0] |= CCB_KEYBOARD_INTERRUPT_ENABLE;

    listen(0x60, IODevice::ReadWrite);
    listen(0x61, IODevice::ReadWrite);
    listen(0x64, IODevice::ReadWrite);
}

Keyboard::~Keyboard()
{
}

BYTE Keyboard::in8(WORD port)
{
    if (port == 0x60) {
        if (m_hasCommand && m_command <= 0x3F) {
            BYTE ramIndex = m_command & 0x3F;
            m_hasCommand = false;
            vlog(VM_KEYMSG, "Reading 8042 RAM [%02] = %02X", ramIndex, m_ram[ramIndex]);
            return m_ram[ramIndex];
        }

        BYTE key = kbd_pop_raw();
        //vlog(VM_KEYMSG, "keyboard_data = %02X", key);
        return key;
    }

    if (port == 0x64) {
        // Keyboard not locked, POST completed successfully.
        BYTE status = ATKBD_UNLOCKED | (m_ram[0] & ATKBD_SYSTEM_FLAG);
        status |= m_lastWasCommand ? ATKBD_CMD_DATA : 0;
        // vlog(VM_KEYMSG, "Keyboard status queried (%02X)", status);
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

    return 0xFF;
}

void Keyboard::out8(WORD port, BYTE data)
{
    if (port == 0x61) {
        //vlog(VM_KEYMSG, "System control port <- %02X", data);
        m_systemControlPortData = data;
        return;
    }

    if (port == 0x64) {
        vlog(VM_KEYMSG, "Keyboard command <- %02X", data);
        m_command = data;
        m_hasCommand = true;
        m_lastWasCommand = true;
        return;
    }

    if (port == 0x60) {
        m_lastWasCommand = false;
        if (!m_hasCommand) {
            vlog(VM_KEYMSG, "Got data (%02X) without command", data);
            return;
        }

        m_hasCommand = false;

        if (m_command == 0xD1) {
            vlog(VM_KEYMSG, "Write output port: A20=%s", (data & 0x02) ? "on" : "off");
            return;
        }

        if (m_command >= 0x60 && m_command <= 0x7F) {
            BYTE ramIndex = m_command & 0x3F;
            m_ram[ramIndex] = data;

            switch (ramIndex) {
            case 0:
                vlog(VM_KEYMSG, "Controller Command Byte set:", data);
                vlog(VM_KEYMSG, "  Keyboard interrupt: %s", data & CCB_KEYBOARD_INTERRUPT_ENABLE ? "enabled" : "disabled");
                vlog(VM_KEYMSG, "  Mouse interrupt:    %s", data & CCB_MOUSE_INTERRUPT_ENABLE ? "enabled" : "disabled");
                vlog(VM_KEYMSG, "  System flag:        %s", data & CCB_SYSTEM_FLAG ? "enabled" : "disabled");
                vlog(VM_KEYMSG, "  Keyboard enable:    %s", data & CCB_KEYBOARD_ENABLE ? "enabled" : "disabled");
                vlog(VM_KEYMSG, "  Mouse enable:       %s", data & CCB_MOUSE_ENABLE ? "enabled" : "disabled");
                vlog(VM_KEYMSG, "  Translation:        %s", data & CCB_TRANSLATE ? "enabled" : "disabled");
                break;
            default:
                vlog(VM_KEYMSG, "Writing 8042 RAM [%02] = %02X", ramIndex, data);
            }
            return;
        }
        vlog(VM_KEYMSG, "Got data %02X for unknown command %02X", data, m_command);
        return;
    }
}

void Keyboard::raiseIRQ()
{
    if (theKeyboard.m_ram[0] & CCB_KEYBOARD_INTERRUPT_ENABLE)
        PIC::raiseIRQ(1);
}
