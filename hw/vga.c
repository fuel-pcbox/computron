#include "vomit.h"
#include "debug.h"
#include "disasm.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static byte current_register = 0;
static byte current_register2 = 0;
static byte current_sequencer = 0;

/* there are only 0x12, but let's avoid segfaults. */
static byte io_register[0x20];

/* there are only ???, but let's avoid segfaults. */
static byte io_register2[0x20];

/* there are only 4, but let's avoid segfaults. */
static byte io_sequencer[0x20];

static byte *video_memory;
static byte columns;
static byte rows;

static byte latch[4];

static byte *vm_p0 = 0;

static void vga_selreg( word, byte );
static void vga_setreg( word, byte );
static void vga_selreg2( word, byte );
static void vga_setreg2( word, byte );
static void vga_selseq( word, byte );
static void vga_setseq( word, byte );
static byte vga_getreg( word );
static byte vga_status( word );
static byte vga_get_current_register( word );

static byte video_dirty = 0;

byte vm_p1[0x9600];
byte vm_p2[0x9600];
byte vm_p3[0x9600];

void
vga_init()
{
	vm_p0 = mem_space + 0xA0000;

	memset( &vm_p1, 0x0, sizeof(vm_p1) );
	memset( &vm_p2, 0x0, sizeof(vm_p2) );
	memset( &vm_p3, 0x0, sizeof(vm_p3) );

	vm_listen( 0x3b4, vga_get_current_register, vga_selreg );
	vm_listen( 0x3b5, vga_getreg, vga_setreg );
	vm_listen( 0x3ba, vga_status, 0L );

	vm_listen( 0x3c4, 0L, vga_selseq );
	vm_listen( 0x3c5, 0L, vga_setseq );
	vm_listen( 0x3ce, 0L, vga_selreg2 );
	vm_listen( 0x3cf, 0L, vga_setreg2 );

	vm_listen( 0x3d4, vga_get_current_register, vga_selreg );
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

byte
vga_get_current_register( word port )
{
	return current_register;
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

void
vga_selseq( word port, byte data )
{
	(void) port;
	/* mask off unused bits */
	current_sequencer = data & 0x1F;
}

void
vga_setseq( word port, byte data )
{
	(void) port;
	vlog( VM_VIDEOMSG, "writing to seq %d, data is %02X", current_sequencer, data );
	io_sequencer[current_sequencer] = data;
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

void
vga_selreg2( word port, byte data )
{
	(void) port;
	current_register2 = data;
}

#define WRITE_MODE (io_register2[5] & 0x03)
#define READ_MODE ((io_register2[5] >> 3) & 1)
#define ODD_EVEN ((io_register2[5] >> 4) & 1)
#define SHIFT_REG ((io_register2[5] >> 5) & 0x03)
#define ROTATE ((io_register2[3]) & 0x07)
#define DRAWOP ((io_register2[3] >> 3) & 3)
#define MAP_MASK_BIT(i) ((io_sequencer[2] >> i)&1)
#define SET_RESET_BIT(i) ((io_register2[0] >> i)&1)
#define SET_RESET_ENABLE_BIT(i) ((io_register2[1] >> i)&1)
#define BIT_MASK (io_register2[8])

void
vga_setreg2( word port, byte data )
{
	(void) port;
	vlog( VM_VIDEOMSG, "writing to reg2 %d, data is %02X", current_register2, data );
	io_register2[current_register2] = data;
}

#define MODE12 (mem_space[0x449]==0x12)

void
vga_setbyte( dword a, byte d )
{
	dword aa = a-0xA0000;
#if 0
	latch[0] = vm_p0[aa];
	latch[1] = vm_p1[aa];
	latch[2] = vm_p2[aa];
	latch[3] = vm_p3[aa];
#endif

	if( aa >= 0x9600 )
	{
		vlog( VM_VIDEOMSG, "OOB write %lx", a + 0xA0000 );
		mem_space[a] = d;
	}

	if( MODE12 )
	{
		byte *dest = 0;
		a -= 0xA0000;
		if( WRITE_MODE != 0 )
		{
			vlog( VM_VIDEOMSG, "write mode %d, no idea what to do ;(", WRITE_MODE );
		}
		switch( io_register2[4] )
		{
			case 0: dest = &vm_p0[a]; break;
			case 1: dest = &vm_p1[a]; break;
			case 2: dest = &vm_p2[a]; break;
			case 3: dest = &vm_p3[a]; break;
		}

		if( WRITE_MODE == 0 )
		{
			byte new_val[4] = { 0, 0, 0, 0 };
			int j, i;
			byte bit_mask = BIT_MASK;
			byte and_mask = 0x01;
			byte value = (d >> ROTATE) | (d << (8-ROTATE));
			static byte lastb = 0;
			if( bit_mask != lastb )
			{
				vlog( VM_VIDEOMSG, "bitmask is %02X", bit_mask );
				lastb = bit_mask;
			}
			for( j = 0; j < 8; ++j )
			{
				if( bit_mask & 0x01 )
				{
					for( i = 0; i < 4; ++i )
					{
						byte new_bit;
						if( SET_RESET_ENABLE_BIT(i))
							new_bit = SET_RESET_BIT(i) << j;
						else
							new_bit = (value & and_mask);

						switch( DRAWOP )
						{
							case 0: new_val[i] |= new_bit; break;
							case 1: new_val[i] |= new_bit & (latch[i] & and_mask); break;
							case 2: new_val[i] |= new_bit | (latch[i] & and_mask); break;
							case 3: new_val[i] |= new_bit ^ (latch[i] & and_mask); break;
							default:
								vlog( VM_VIDEOMSG, "LOL WUT?" );
						}
					}
				}
				else
				{
					for( i = 0; i < 4; ++i )
						new_val[i] |= (latch[i] & and_mask);
				}
				bit_mask >>= 1;
				and_mask <<= 1;
			}
			if( io_sequencer[2] & 0x0F )
			{
				if( MAP_MASK_BIT(0) ) vm_p0[a] = new_val[0];
				if( MAP_MASK_BIT(1) ) vm_p1[a] = new_val[1];
				if( MAP_MASK_BIT(2) ) vm_p2[a] = new_val[2];
				if( MAP_MASK_BIT(3) ) vm_p3[a] = new_val[3];
			}
			else
			{
				*dest = d;
			}
		}
		else
		{
			*dest = d;
		}
		video_dirty = 1;
		return;
	}
	video_dirty = 1;
	mem_space[a] = d;
}

byte
vga_getbyte( dword a )
{
	if( READ_MODE == 1 )
	{
		vlog( VM_VIDEOMSG, "ZOMG!" );
	}
	else if( READ_MODE == 0 )
	{
	}
	if( MODE12 )
	{
		a -= 0xA0000;
		if( a < 0x9600 )
		{
			latch[0] = vm_p0[a];
			latch[1] = vm_p1[a];
			latch[2] = vm_p2[a];
			latch[3] = vm_p3[a];
			return latch[io_register2[4]];
		}
		else
		{
		vlog( VM_VIDEOMSG, "OOB read %lx", a + 0xA0000 );
		return mem_space[a];
		}
	}
	else
	{
		return mem_space[a];
	}
}

void
vga_setword( dword a, word w )
{
	vga_setbyte( a, LSB(w) );
	vga_setbyte( a + 1, MSB(w) );
}

word
vga_getword( dword a )
{
	return MAKEWORD( vga_getbyte(a), vga_getbyte(a+1) );
}

void
clear_video_dirty()
{
	video_dirty = 0;
}

int
is_video_dirty()
{
	return video_dirty;
}
