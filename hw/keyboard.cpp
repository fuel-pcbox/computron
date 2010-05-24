/*
 * Basic AT keyboard controller
 * for VOMIT :^)
 */

#include "vomit.h"
#include "vcpu.h"
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

static BYTE keyboard_read_data(VCpu* cpu, WORD port);
static void keyboard_write_data(VCpu* cpu, WORD, BYTE);
static BYTE keyboard_status(VCpu* cpu, WORD);
static BYTE system_control_read(VCpu* cpu, WORD);
static void system_control_write(VCpu* cpu, WORD, BYTE);
static void keyboard_command(VCpu* cpu, WORD, BYTE);

static BYTE system_control_port_data;

static BYTE keyboard_controller_ram[32];
static BYTE keyboard_controller_command;
static bool keyboard_controller_has_command;

void keyboard_init()
{
	vm_listen(0x60, keyboard_read_data, keyboard_write_data);
	vm_listen(0x61, system_control_read, system_control_write);
	vm_listen(0x64, keyboard_status, keyboard_command);

	system_control_port_data = 0;
    keyboard_controller_command = 0x00;
    keyboard_controller_has_command = false;

    memset(keyboard_controller_ram, 0, sizeof(keyboard_controller_ram));

    keyboard_controller_ram[0] |= CCB_SYSTEM_FLAG;

    // FIXME: The BIOS should do this, no?
    keyboard_controller_ram[0] |= CCB_KEYBOARD_ENABLE;
    keyboard_controller_ram[0] |= CCB_KEYBOARD_INTERRUPT_ENABLE;
}

BYTE keyboard_status(VCpu* cpu, WORD port)
{
	/* Keyboard not locked, POST completed successfully. */
	//vlog(VM_KEYMSG, "Keyboard status queried.");
	return ATKBD_UNLOCKED | (keyboard_controller_ram[0] & ATKBD_SYSTEM_FLAG);
}

/*
 * From http://courses.ece.uiuc.edu/ece390/books/labmanual/io-devices.html
 *
 * ...The only special requirement is that it acknowledges reception of
 * the keyboard event by toggling bit 7 of port 61h to 1 and back to 0.
 * The other bits of port 61h must not be modified, since they control
 * other hardware. This is only required for full original IBM PC
 * compatibility.
 *
 * This is why MS Windows flips the 0x80 bit of I/O port 0x61 after each
 * keypress. Took a while to catch that one... :)
 */

BYTE system_control_read(VCpu* cpu, WORD port)
{
	//vlog( VM_KEYMSG, "%02X <- System control port", system_control_port_data);
	return system_control_port_data;
}

void system_control_write(VCpu* cpu, WORD port, BYTE data)
{
	system_control_port_data = data;
	//vlog(VM_KEYMSG, "System control port <- %02X", data);
}

BYTE keyboard_read_data(VCpu* cpu, WORD port)
{
    if (keyboard_controller_has_command && keyboard_controller_command <= 0x3F) {
        BYTE ramIndex = keyboard_controller_command & 0x3F;
        keyboard_controller_has_command = false;
        vlog(VM_KEYMSG, "Reading 8042 RAM [%02] = %02X", ramIndex, keyboard_controller_ram[ramIndex]);
        return keyboard_controller_ram[ramIndex];
    }

	BYTE key = kbd_pop_raw();
	//vlog(VM_KEYMSG, "keyboard_data = %02X", key);
	return key;
}

void keyboard_write_data(VCpu* cpu, WORD port, BYTE data)
{
    if (!keyboard_controller_has_command) {
        vlog(VM_KEYMSG, "Got data (%02X) without command", data);
        return;
    }

    keyboard_controller_has_command = false;

    if (keyboard_controller_command >= 0x60 && keyboard_controller_command <= 0x7F) {
        BYTE ramIndex = keyboard_controller_command & 0x3F;
        keyboard_controller_ram[ramIndex] = data;

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

    vlog(VM_KEYMSG, "Got data %02X for unknown command %02X", data, keyboard_controller_command);
}

void keyboard_command(VCpu* cpu, WORD port, BYTE data)
{
    keyboard_controller_command = data;
	vlog(VM_KEYMSG, "Keyboard command <- %02X", data);
}

void keyboard_raise_irq_if_enabled()
{
    if (keyboard_controller_ram[0] & CCB_KEYBOARD_INTERRUPT_ENABLE)
        irq(1);
}
