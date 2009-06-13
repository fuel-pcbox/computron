/*
 * Gameport...
 *
 * Not really an emulation, just setting up some dummy I/O handlers
 * so it won't spam me about reading port 0x201..
 */

#include "vomit.h"

static byte gameport_read( word port );

void
gameport_init()
{
	vm_listen( 0x201, gameport_read, 0L );
}

byte
gameport_read( word port )
{
	return 0x80;
}
