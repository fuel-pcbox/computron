/* vomit.c
 * Main initialization procedures
 *
 */

#include "vomit.h"
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

bool	verbose,
		disklog, trapint, rmpeek,
		iopeek, mempeek,
		callpeek, iplog;

bool g_break_pressed = false;

static bool exiting = 0;

#ifdef VM_DEBUG
	word BCS, BIP;
#endif

int
main( int argc, char **argv )
{
	int i;
	if( argc > 1 )
		for( i = 1; i < argc; ++i ) {
			#ifdef VM_DEBUG
				if ( !strcmp( argv[i], "--verbose" ) ) verbose = true;
				if ( !strcmp( argv[i], "--iplog" ) ) iplog = true;
				if ( !strcmp( argv[i], "--disklog" ) ) disklog = true;
				if ( !strcmp( argv[i], "--trapint" ) ) trapint = true;
				if ( !strcmp( argv[i], "--rmpeek" ) ) rmpeek = true;
				if ( !strcmp( argv[i], "--iopeek" ) ) iopeek = true;
				if ( !strcmp( argv[i], "--mempeek" ) ) mempeek = true;
				if ( !strcmp( argv[i], "--callpeek" ) ) callpeek = true;
			#endif
        }

	vm_init();
	vm_loadconf();
	cpu_genmap();

	signal( SIGINT, vm_cbreak );
#if defined( VM_DEBUG ) && defined( VM_BREAK )
	if ( verbose ) vm_out( "Ctrl-C mapped to INT 3\n", VM_INITMSG );
#endif

	cpu_state = CPU_ALIVE;

	cpu_main();

	vm_exit( 0 );

	return 0;
}

void vm_init() {
	dword i;
	#ifdef VM_DEBUG
		if(verbose) vm_out("Initializing memory.\n", VM_INITMSG);
	#endif
    mem_init();
	#ifdef VM_DEBUG
		if(verbose) vm_out("Initializing cpu.\n", VM_INITMSG);
	#endif
    cpu_init();
	int_init();
	#ifdef VM_DEBUG
		if(verbose) vm_out("vomit: Initializing ui.\n", VM_INITMSG);
	#endif
	ui_init();

	for ( i = 0; i < 0xffff; ++i )
		vm_listen( i, &vm_ioh_nin, &vm_ioh_nout );

	vga_init();
}

void
vm_kill()
{
	vga_kill();
	cpu_kill();
	mem_kill();
	ui_kill();
}

void
vm_exit( int ec )
{
	exiting = true;
#ifdef VM_DEBUG
	if( verbose ) {
		dump_all();
		vm_out( "Killing VM.\n", VM_KILLMSG );
		/* No "--verbose" messages while exiting. */
		verbose = false;
	}
#endif
	vm_kill();
	exit( ec );
}

void
vm_cbreak( int sig )
{
	(void) sig;
#ifdef VM_BREAK
	g_break_pressed = true;
#endif
#ifndef VM_DEBUG
	vm_exit(0x66);
#endif
}
