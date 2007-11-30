#include "vomit.h"

static byte icw[4] = { 0, 0, 0, 0 };

static void pic_write_icw( word, byte );
static byte pic_read_icw( word );

void
pic_init()
{
	vm_listen( 0x0020, pic_read_icw, pic_write_icw );
	vm_listen( 0x0021, pic_read_icw, pic_write_icw );
}

void
pic_write_icw( word port, byte data )
{
	icw[port - 0x20] = data;
}

byte
pic_read_icw( word port )
{
	return icw[port - 0x20];
}
