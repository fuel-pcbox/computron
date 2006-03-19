#include "vomit.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static byte current_register = 0;

/* there are only 0x12, but let's avoid segfaults. */
static byte io_register[0x20];

static byte *video_memory;
static byte columns;
static byte rows;

static void vga_selreg( word, word, byte );
static void vga_setreg( word, word, byte );
static word vga_getreg( word, byte );
static word vga_status( word, byte );

void
vga_init()
{
	vm_listen( 0x3d4, 0L, vga_selreg );
	vm_listen( 0x3d5, vga_getreg, vga_setreg );
	vm_listen( 0x3da, vga_status, 0L );

	columns = 80;
	rows = 25;

	video_memory = mem_space + 0xB8000;
	memset( video_memory, 0, columns * rows * 2 );
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
vga_selreg( word port, word data, byte bits )
{
	(void) port;
	(void) bits;
	/* mask off unused bits */
	current_register = (byte)( data & 0x1F );
}

void
vga_setreg( word port, word data, byte bits )
{
	(void) port;
	(void) bits;
	io_register[current_register] = (byte) data;
}

word
vga_getreg( word port, byte bits )
{
	(void) port;
	(void) bits;
	return (word)io_register[current_register];
}

/*
 * 6845 - Port 3DA Status Register
 *
 *  |7|6|5|4|3|2|1|0|  3DA Status Register
 *  | | | | | | | `---- 1 = display enable, RAM access is OK
 *  | | | | | | `----- 1 = light pen trigger set
 *  | | | | | `------ 0 = light pen on, 1 = light pen off
 *  | | | | `------- 1 = vertical retrace, RAM access OK for next 1.25ms
 *  `-------------- unused
 *
 */

word
vga_status( word port, byte bits )
{
	(void) port;
	(void) bits;
	/* 0000 1101 */
	return 0x0D;
}

/* TODO: move this to bios/video.c */
void
vga_scrollup (byte x1, byte y1, byte x2, byte y2, byte num, byte attr) {
	byte x, y, i;
	if ( (num == 0 ) || ( num > rows ) ) {
		for( y = y1; y <= y2; ++y ) {
			for( x = x1; x < x2; ++x) {
				video_memory[( y * 160 + x * 2 ) + 0] = 0x20;
				video_memory[( y * 160 + x * 2 ) + 1] = attr;
			}
		}
		return;
	}
	for ( i = 0; i < num; ++i ) {
		for ( y = y1; y < y2; ++y ) {
			for ( x = x1; x < x2; ++x ) {
				video_memory[( y * 160 + x * 2 ) + 0] = video_memory[(((y+1)*160)+x*2)+0];
				video_memory[( y * 160 + x * 2 ) + 1] = video_memory[(((y+1)*160)+x*2)+1];
			}
		}
		for ( x = x1; x < x2; ++x ) {
			video_memory[( y2 * 160 + x * 2 ) + 0] = 0x20;
			video_memory[( y2 * 160 + x * 2 ) + 1] = attr;
		}
		y2--;
	}
}

byte
vga_read_register( byte index )
{
	assert( index <= 0x12 );
	return io_register[index];
}

void
vga_write_register( byte index, byte value )
{
	assert( index <= 0x12 );
	io_register[index] = value;
}
