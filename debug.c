/* debug.c
 * MS Debug-like VM interface
 *
 *
 */

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
