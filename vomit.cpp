/* vomit.c
 * Main initialization procedures
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "vomit.h"
#include "debug.h"
#include "iodevice.h"
#include <QDebug>

vomit_options_t options;

int
vomit_init( int argc, char **argv )
{
	vm_init();
	vm_loadconf();
	return 0;
}

void vm_init() {
	dword i;
	vlog( VM_INITMSG, "Initializing memory" );
    mem_init();
	vlog( VM_INITMSG, "Initializing CPU" );
    cpu_init();
	vlog( VM_INITMSG, "Initializing video BIOS" );
	video_bios_init();

	for ( i = 0; i <= 0xFFFF; ++i )
		vm_listen( i, 0L, 0L );

	vlog( VM_INITMSG, "Registering I/O devices" );
	foreach( Vomit::IODevice *device, Vomit::IODevice::devices() )
	{
		vlog( VM_INITMSG, "%s at 0x%p", device->name(), device );
	}

	pic_init();
	dma_init();
	vga_init();
	fdc_init();
	ide_init();
	pit_init();
	busmouse_init();
	keyboard_init();
	gameport_init();
}

void
vm_kill()
{
	vlog( VM_KILLMSG, "Killing VM" );
	vga_kill();
	cpu_kill();
	mem_kill();
}

void
vm_exit( int ec )
{
	vm_kill();
	exit( ec );
}
