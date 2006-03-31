/* debug.c
 * MS Debug-like VM interface
 * + home of vlog()
 *
 */

#include "vomit.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

bool g_debug_step = false;

void
vlog( int category, const char *format, ... )
{
	va_list ap;
	const char *prefix = 0L;
	FILE *logfile = fopen( "log.txt", "a" );

	if( !logfile )
		return;

	switch( category )
	{
		case VM_INITMSG: prefix = "init"; break;
		case VM_DISKLOG: prefix = "disk"; break;
		case VM_KILLMSG: prefix = "kill"; break;
		case VM_IOMSG:   prefix = "i/o"; break;
		case VM_ALERT:   prefix = "alert"; break;
		case VM_PRNLOG:  prefix = "lpt"; break;
		case VM_VIDEOMSG: prefix = "video"; break;
		case VM_CONFIGMSG: prefix = "config"; break;
		case VM_CPUMSG:  prefix = "cpu"; break;
		case VM_MEMORYMSG: prefix = "memory"; break;
	}

	if( prefix )
	{
		fprintf( logfile, "(%8s) ", prefix );
		printf( "(%8s) ", prefix );
	}

	va_start( ap, format );
	vfprintf( logfile, format, ap );
	vprintf( format, ap );
	va_end( ap );

	fputc( '\n', logfile );
	puts( "" );

	fclose( logfile );
}

void
vm_debug()
{
	char curcmd[256], *curtok;
	word mseg, moff;

	if( g_debug_step )
	{
		dump_all();
		g_debug_step = false;
	}

	while( true )
	{
		printf( "vomit> " );
		fflush( stdout );
		fgets( curcmd, sizeof( curcmd ), stdin );
		if( feof( stdin ))
		{
			vlog( VM_KILLMSG, "EOF on stdin, exiting." );
			vm_exit( 0 );
		}
		curtok = strtok( curcmd, " \n" );
		if ( curtok ) {
			if( !strcmp( curtok, "g" ))
				break;
			if( !strcmp( curtok, "s" ))
			{
				g_debug_step = true;
				break;
			}

			if ( !strcmp( curtok, "c" ) )
				dump_cpu();
			else if ( !strcmp( curtok, "r" ) )
				dump_all();
			else if ( !strcmp( curtok, "i" ) )
				dump_ivt();
			else if ( curtok[0] == 'd' ) {
				curtok = strtok(NULL, ": \n");
				if(curtok!=0) {
					mseg = strtol(curtok,NULL,16);
					curtok = mseg ? strtok(NULL, " \n") : NULL;
					if(curtok!=0) {
						moff = strtol(curtok,NULL,16);
						dump_mem(mseg, moff, 16);
					} else {
						dump_mem( cpu.CS, mseg, 16 );
					}
				} else {
					dump_mem( cpu.CS, (cpu.IP&0xFFF0), 16 );
					curtok = &curcmd[0];
				}
			}
			else if ( !strcmp( curtok, "q" ) )
				vm_exit( 0 );
			else if ( !strcmp( curtok, "?" ) ) {
				printf(	"\tavailable commands:\n\n" \
						"\tc\t\tcpu information\n" \
						"\tr\t\tdump registers and stack\n" \
						"\td [SEG OFF]\tdump memory\n" \
						"\ti\t\tdump interrupt vector table\n" \
						"\tg\t\tcontinue execution\n" \
						"\ts\t\tstep execution\n" \
						"\tq\t\tabort execution\n" \
						"\n");
			}
		}
	}
}
