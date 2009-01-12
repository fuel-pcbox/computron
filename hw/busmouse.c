/*
 * busmouse... based on... guesswork and bochs peeking
 */

#include "vomit.h"
#include "debug.h"

static byte busmouse_ident_read( word port );
static void busmouse_control_write( word port, byte data );
static void busmouse_data_write( word port, byte data );
static byte busmouse_data_read( word port );

void
busmouse_init()
{
	vm_listen( 0x23e, busmouse_ident_read, 0L );
	vm_listen( 0x23c, 0L, busmouse_control_write );
	vm_listen( 0x23d, busmouse_data_read, busmouse_data_write );
}

byte
busmouse_ident_read( word port )
{
	static unsigned int seq = 0;

	vlog( VM_MOUSEMSG, "Bus mouse identification queried (seq: %u), [port=0x%0x]", seq & 1, port );

	if( seq++ & 1 )
		return 0x22;
	else
		return 0xDE;
}

void
busmouse_control_write( word port, byte data )
{

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
			break;
		default:
			vlog( VM_MOUSEMSG, "Data %02X received", data );
			break;
	}
}

byte
busmouse_data_read( word port )
{
	vlog( VM_MOUSEMSG, "Bus mouse data requested" );
	return 0xff;
}
