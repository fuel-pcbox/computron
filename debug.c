/* debug.c
 * MS Debug-like VM interface
 *
 *
 */

#include "vomit.h"
#include <stdarg.h>
#include <stdio.h>

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
	}

	va_start( ap, format );
	vfprintf( logfile, format, ap );
	va_end( ap );

	fputc( '\n', logfile );

	fclose( logfile );
}

#ifdef VM_DEBUG

#include "vomit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool g_debug_step = false;

void
vm_debug()
{
	bool cont = true;
	char curcmd[256], *curtok;
	word mseg, moff;

	if ( g_debug_step ) {
		dump_all();
	}
	g_debug_step = false;

	while ( cont ) {
		printf( "vomit> " );
		fflush( stdout );
		fgets( curcmd, sizeof( curcmd ), stdin );
		curtok = strtok( curcmd, " \n" );
		if ( curtok ) {
			if ( !strcmp( curtok, "g" ) )
				cont = false;
			else if ( !strcmp( curtok, "c" ) )
				dump_cpu();
			else if ( !strcmp( curtok, "r" ) )
				dump_all();
			else if ( !strcmp( curtok, "i" ) )
				dump_ivt();
			else if ( !strcmp( curtok, "s" ) ) {
				g_debug_step = true;
				return;
			}
			else if ( curtok[0] == 'd' ) {
				curtok = strtok(NULL, ": \n");
				if(curtok!=0) {
					mseg = strtol(curtok,NULL,16);
					curtok = mseg ? strtok(NULL, " \n") : NULL;
					if(curtok!=0) {
						moff = strtol(curtok,NULL,16);
						dump_mem(mseg, moff, 16);
					} else {
						dump_mem(CS, mseg, 16);
					}
				} else {
					dump_mem(CS, (IP&0xFFF0), 16);
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

#endif /* VM_DEBUG */
