#include "vomit.h"
#include "debug.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static byte current_register = 0;

/* there are only 0x12, but let's avoid segfaults. */
static byte io_register[0x20];

static byte *video_memory;
static byte columns;
static byte rows;

static void vga_selreg( word, byte );
static void vga_setreg( word, byte );
static byte vga_getreg( word );
static byte vga_status( word );

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

#ifdef VOMIT_DEBUG_VGA
static void
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
#ifdef VOMIT_DEBUG_VGA
	vga_dump();
#endif
}

void
vga_selreg( word port, byte data )
{
	(void) port;
	/* mask off unused bits */
	current_register = data & 0x1F;
}

void
vga_setreg( word port, byte data )
{
	(void) port;
	io_register[current_register] = data;
}

byte
vga_getreg( word port )
{
	(void) port;
	return io_register[current_register];
}

byte
vga_status( word port )
{
	static bool last_bit0 = 0;
	byte data;

	(void) port;

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

	/* 0000 1100 */
	data = 0x0C;

	/* Microsoft DEFRAG expects bit 0 to "blink", so we'll flip it between reads. */
	last_bit0 = !last_bit0;
	data |= last_bit0;

	return data;
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
vga_read_register( byte number )
{
	assert( number <= 0x12 );
	return io_register[number];
}

void
vga_write_register( byte number, byte value )
{
	assert( number <= 0x12 );
	io_register[number] = value;
}
