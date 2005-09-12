#include "vomit.h"
#include <stdio.h>

byte vga_curreg = 0;

byte vga_reg[0x1F];		/* there are only 0x12, but let's avoid segfaults. */

byte *vga_mem;
byte vga_cols, vga_rows;

void
vga_init()
{
	int i;
	for( i = 0; i < ( 80 * 25 * 2 ); i += 2 ) {
		mem_setbyte( 0xB800, i, ' ' );
	}
	vm_listen( 0x3d4, vm_ioh_nin, vga_selreg );
	vm_listen( 0x3d5, vga_getreg, vga_setreg );
	vga_mem = mem_space + 0xB8000;
	vga_cols = 80;
	vga_rows = 25;
}

#ifdef VM_DEBUG
void
vga_dump()
{
	int i, j;
	FILE *scrot = fopen( "page0.txt", "w" );
	for ( i = 0; i < 25 ; ++i ) {
		for ( j = 0; j < 160; j += 2) {
			fputc( mem_getbyte( 0xB800, i * 160 + j ), scrot );
		}
		fprintf( scrot, "\n" );
	}
	fclose( scrot );
}
#endif

void
vga_kill()
{
#ifdef VM_DEBUG
	vga_dump();
#endif
}

void
vga_selreg( word data, byte bits )
{
	(void) bits;
	vga_curreg = (byte) ( data & 0x1F );	/* mask off unused bits */
}

void
vga_setreg( word data, byte bits )
{
	(void) bits;
	vga_reg[vga_curreg] = (byte) data;
}

word
vga_getreg( byte bits )
{
	(void) bits;
	return (word) vga_reg[vga_curreg];
}

void
vga_scrollup (byte x1, byte y1, byte x2, byte y2, byte num, byte attr) {
	byte x, y, i;
/*	byte width = x2-x1;
	byte height = y2-y1; */
	if ( (num == 0 ) || ( num > vga_rows ) ) {
		for( y = y1; y <= y2; ++y ) {
			for( x = x1; x < x2; ++x) {
				vga_mem[( y * 160 + x * 2 ) + 0] = 0x20;
				vga_mem[( y * 160 + x * 2 ) + 1] = attr;
			}
		}
		return;
	}
	for ( i = 0; i < num; ++i ) {
		for ( y = y1; y < y2; ++y ) {
			for ( x = x1; x < x2; ++x ) {
				vga_mem[( y * 160 + x * 2 ) + 0] = vga_mem[(((y+1)*160)+x*2)+0];
				vga_mem[( y * 160 + x * 2 ) + 1] = vga_mem[(((y+1)*160)+x*2)+1];
			}
		}
		for ( x = x1; x < x2; ++x ) {
			vga_mem[( y2 * 160 + x * 2 ) + 0] = 0x20;
			vga_mem[( y2 * 160 + x * 2 ) + 1] = attr;
		}
		y2--;

	}
	return;
}

