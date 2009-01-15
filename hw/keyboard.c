/*
 * Basic AT keyboard controller
 * for VOMIT :^)
 */

#include "vomit.h"
#include "debug.h"

#define ATKBD_PARITY_ERROR  0x80
#define ATKBD_TIMEOUT       0x40
#define ATKBD_BUFFER_FULL   0x20
#define ATKBD_UNLOCKED      0x10
#define ATKBD_CMD_DATA      0x08
#define ATKBD_SYSTEM_FLAG   0x04
#define ATKBD_INPUT_STATUS  0x02
#define ATKBD_OUTPUT_STATUS 0x01

static byte keyboard_data( word port );
static byte keyboard_status( word );
static byte system_control_read( word );
static void system_control_write( word, byte );

static byte system_control_port_data;

void
keyboard_init()
{
	vm_listen( 0x60, keyboard_data, 0L );
	vm_listen( 0x61, system_control_read, system_control_write );
	vm_listen( 0x64, keyboard_status, 0L );

	system_control_port_data = 0;
}

byte
keyboard_status( word port )
{
	/* Keyboard not locked, POST completed successfully. */
	vlog( VM_KEYMSG, "Keyboard status queried." );
	return ATKBD_UNLOCKED | ATKBD_SYSTEM_FLAG;
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

byte
system_control_read( word port )
{
	vlog( VM_KEYMSG, "%02X <- System control port", system_control_port_data );
	return system_control_port_data;
}

void
system_control_write( word port, byte data )
{
	system_control_port_data = data;
	vlog( VM_KEYMSG, "System control port <- %02X", data );
}

byte
keyboard_data( word port )
{
	byte key = kbd_pop_raw();
	vlog( VM_KEYMSG, "keyboard_data = %02X", key );
	return key;
}
