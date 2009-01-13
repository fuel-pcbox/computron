/*
 * busmouse... based on... guesswork and bochs peeking
 */

#include "vomit.h"
#include "debug.h"

static void busmouse_ident_write( word port, byte data );
static byte busmouse_ident_read( word port );
static void busmouse_signature_write( word port, byte data );
static byte busmouse_signature_read( word port );
static void busmouse_control_write( word port, byte data );
static void busmouse_data_write( word port, byte data );
static byte busmouse_data_read( word port );

static byte mouse_register[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static byte selected_register = 0;
static unsigned int ident_seq = 0;
static byte signature = 0x22;
static bool interrupts = false;

void
busmouse_init()
{
	vm_listen( 0x23c, 0L, busmouse_control_write );
	vm_listen( 0x23d, busmouse_signature_read, busmouse_signature_write );
	vm_listen( 0x23e, busmouse_ident_read, busmouse_ident_write );
	vm_listen( 0x23f, busmouse_data_read, busmouse_data_write );
}

void
busmouse_ident_write( word port, byte data )
{
	vlog( VM_MOUSEMSG, "Ident write %02X?", data );
}

void
busmouse_signature_write( word port, byte data )
{
	vlog( VM_MOUSEMSG, "register[%u] = %02X", selected_register, data );
	mouse_register[selected_register] = data;
}

byte
busmouse_ident_read( word port )
{
	byte retval;

	if( ident_seq++ & 1 )
		retval = 0x22;
	else
		retval = 0xDE;

	vlog( VM_MOUSEMSG, "Bus mouse identification queried (seq: %u, value: %02X)", ident_seq & 1, retval );

	//vm_debug();

	return retval;
}

void
busmouse_control_write( word port, byte data )
{

	if( data & 0x80 )
	{
		vlog( VM_MOUSEMSG, "BusMouse reset" );
		ident_seq = 0;
	}

	selected_register = data & 0x07;
	vlog( VM_MOUSEMSG, "Register %u selected", selected_register );

	switch( data )
	{
		default:
			vlog( VM_MOUSEMSG, "Command %02X received", data );
			break;
	}
}

void
busmouse_data_write( word port, byte data )
{
	switch( data )
	{
		case 0x10:
			vlog( VM_MOUSEMSG, "Bus mouse interrupt disabled" );
			interrupts = false;
			break;
		case 0x11:
			vlog( VM_MOUSEMSG, "Bus mouse interrupt enabled" );
			interrupts = true;
			break;
		default:
			vlog( VM_MOUSEMSG, "Data %02X received (port: %03x)", data, port );
			break;
	}
}

byte
busmouse_signature_read( word port )
{
	vlog( VM_MOUSEMSG, "Read register[%u] (= %02X)", selected_register, mouse_register[selected_register] );
	return mouse_register[selected_register];
}

byte
busmouse_data_read( word port )
{
	vlog( VM_MOUSEMSG, "Bus mouse data requested (port: %03x)", port );
	return 0x00;
}

void
busmouse_pulse()
{
	if( interrupts )
		irq( 5 );
}
