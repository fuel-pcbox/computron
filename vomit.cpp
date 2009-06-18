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

vomit_options_t options;

bool disklog, trapint, iopeek, mempeek;

#ifdef VOMIT_TRY
bool g_try_run = false;
#endif

bool g_break_pressed = false;

static bool exiting = 0;

#define FLAGARG( a, b ) else if( !strcmp( argv[1], a )) { b = true; argc--; argv++; }

int
vomit_init( int argc, char **argv )
{
	memset( &options, 0, sizeof(options) );

#ifdef VOMIT_TRY
	const char *try_path = 0L;
#endif

	while( argc > 1 )
	{
		if( 0 ) {}
		FLAGARG( "--disklog",  disklog )
		FLAGARG( "--trapint",  trapint )
		FLAGARG( "--mempeek",  mempeek )
		FLAGARG( "--iopeek",   iopeek )
		FLAGARG( "--bda-peek", options.bda_peek )
		FLAGARG( "--trace", options.trace )

#ifdef VOMIT_TRY
		else if( argc > 2 && !strcmp( argv[1], "--try" ))
		{
			try_path = argv[2];
			g_try_run = true;
			argc -= 2, argv += 2;
		}
#endif

		else
		{
			fprintf( stderr, "Unknown option: %s\n", argv[1] );
			return 1;
		}
	}

#ifndef VOMIT_TRACE
	if( options.trace )
	{
		fprintf( stderr, "Rebuild with #define VOMIT_TRACE if you want --trace to work.\n" );
		exit( 1 );
	}
#endif

	FILE *fplog = fopen( "log.txt", "w" );
	fclose( fplog );

	extern void vomit_disasm_init_tables();
	vomit_disasm_init_tables();

	vm_init();
	vm_loadconf();
	cpu_genmap();

	cpu.state = CPU_ALIVE;

#ifdef VOMIT_TRY
	if( g_try_run )
	{
		FILE *fp = fopen( try_path, "rb" );
		if( !fp )
		{
			perror( try_path );
			return 1;
		}

		printf( "Loading %s into 1000:0000.\n", try_path );

		/* Read up to MAX_FILESIZE bytes into 1000:0000. */
		int x = fread( mem_space + 0x10000, 1, MAX_FILESIZE, fp );
		printf( "%d bytes read.\n", x );

		fclose( fp );

		cpu.IF = 0;
		cpu_jump( 0x1000, 0x0000 );
		cpu.regs.W.SP = 0x1000;
	}
#endif
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
	exiting = true;
	vm_kill();
	exit( ec );
}
