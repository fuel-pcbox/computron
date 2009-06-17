#include "vomit.h"
#include "debug.h"
#include "disasm.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

static byte current_register = 0;
static byte current_register2 = 0;
static byte current_sequencer = 0;

/* there are only 0x12, but let's avoid segfaults. */
static byte io_register[0x20];

/* there are only ???, but let's avoid segfaults. */
static byte io_register2[0x20];

/* there are only 4, but let's avoid segfaults. */
static byte io_sequencer[0x20];

/* there are only ??, but let's avoid segfaults. */
static byte index_palette = 0;

static byte columns;
static byte rows;

static byte latch[4];

static void vga_selreg( word, byte );
static void vga_setreg( word, byte );
static void vga_selreg2( word, byte );
static void vga_setreg2( word, byte );
static byte vga_getreg2( word );
static void vga_selseq( word, byte );
static void vga_setseq( word, byte );
static byte vga_getseq( word );
static byte vga_getreg( word );
static byte vga_status( word );
static byte vga_get_current_register( word );
static void vga_write_3c0( word port, byte data );
static byte vga_read_3c1( word port );
static byte vga_read_miscellaneous_output_register( word port );

static bool next_3c0_is_index = true;

static bool video_dirty = false;
bool palette_dirty = true;

byte vm_p0[0x10000];
byte vm_p1[0x10000];
byte vm_p2[0x10000];
byte vm_p3[0x10000];

byte vga_palette_register[17] =
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0x03
};

rgb_t vga_color_register[256] =
{
	{0x00,0x00,0x00}, {0x00,0x00,0x2a}, {0x00,0x2a,0x00}, {0x00,0x2a,0x2a}, {0x2a,0x00,0x00}, {0x2a,0x00,0x2a}, {0x2a,0x15,0x00}, {0x2a,0x2a,0x2a},
	{0x00,0x00,0x00}, {0x00,0x00,0x2a}, {0x00,0x2a,0x00}, {0x00,0x2a,0x2a}, {0x2a,0x00,0x00}, {0x2a,0x00,0x2a}, {0x2a,0x15,0x00}, {0x2a,0x2a,0x2a},
	{0x15,0x15,0x15}, {0x15,0x15,0x3f}, {0x15,0x3f,0x15}, {0x15,0x3f,0x3f}, {0x3f,0x15,0x15}, {0x3f,0x15,0x3f}, {0x3f,0x3f,0x15}, {0x3f,0x3f,0x3f},
	{0x15,0x15,0x15}, {0x15,0x15,0x3f}, {0x15,0x3f,0x15}, {0x15,0x3f,0x3f}, {0x3f,0x15,0x15}, {0x3f,0x15,0x3f}, {0x3f,0x3f,0x15}, {0x3f,0x3f,0x3f},
	{0x00,0x00,0x00}, {0x00,0x00,0x2a}, {0x00,0x2a,0x00}, {0x00,0x2a,0x2a}, {0x2a,0x00,0x00}, {0x2a,0x00,0x2a}, {0x2a,0x15,0x00}, {0x2a,0x2a,0x2a},
	{0x00,0x00,0x00}, {0x00,0x00,0x2a}, {0x00,0x2a,0x00}, {0x00,0x2a,0x2a}, {0x2a,0x00,0x00}, {0x2a,0x00,0x2a}, {0x2a,0x15,0x00}, {0x2a,0x2a,0x2a},
	{0x15,0x15,0x15}, {0x15,0x15,0x3f}, {0x15,0x3f,0x15}, {0x15,0x3f,0x3f}, {0x3f,0x15,0x15}, {0x3f,0x15,0x3f}, {0x3f,0x3f,0x15}, {0x3f,0x3f,0x3f},
	{0x15,0x15,0x15}, {0x15,0x15,0x3f}, {0x15,0x3f,0x15}, {0x15,0x3f,0x3f}, {0x3f,0x15,0x15}, {0x3f,0x15,0x3f}, {0x3f,0x3f,0x15}, {0x3f,0x3f,0x3f},
};


#if 0
rgb_t vga_color_register[256] =
{
	{ 0, 0, 0 },
	{ 0, 0, 31 },
	{ 0, 31, 0 },
	{ 0, 31, 31 },
	{ 31, 0, 0 },
	{ 31, 0, 31 },
	{ 31, 31, 0 },
	{ 31, 31, 31 },
	{ 47, 47, 47},
	{ 0, 0, 63 },
	{ 0, 63, 0 },
	{ 0, 63, 63 },
	{ 63, 0, 0 },
	{ 63, 0, 63 },
	{ 63, 63, 0 },
	{ 63, 63, 63 },
};
#endif

void
vga_init()
{
	latch[0] = 0;
	latch[1] = 0;
	latch[2] = 0;
	latch[3] = 0;

	memset( &vm_p0, 0x0, sizeof(vm_p0) );
	memset( &vm_p1, 0x0, sizeof(vm_p1) );
	memset( &vm_p2, 0x0, sizeof(vm_p2) );
	memset( &vm_p3, 0x0, sizeof(vm_p3) );

	vm_listen( 0x3b4, vga_get_current_register, vga_selreg );
	vm_listen( 0x3b5, vga_getreg, vga_setreg );
	vm_listen( 0x3ba, vga_status, 0L );

	vm_listen( 0x3c0, 0L, vga_write_3c0 );
	vm_listen( 0x3c1, vga_read_3c1, 0L );

	vm_listen( 0x3c4, 0L, vga_selseq );
	vm_listen( 0x3c5, vga_getseq, vga_setseq );
	vm_listen( 0x3ce, 0L, vga_selreg2 );
	vm_listen( 0x3cf, vga_getreg2, vga_setreg2 );

	vm_listen( 0x3d4, vga_get_current_register, vga_selreg );
	vm_listen( 0x3d5, vga_getreg, vga_setreg );
	vm_listen( 0x3da, vga_status, 0L );

	vm_listen( 0x3cc, vga_read_miscellaneous_output_register, 0L );

	columns = 80;
	rows = 25;

	memset( io_register, 0, sizeof(io_register) );
	memset( io_register2, 0, sizeof(io_register2) );

	io_sequencer[2] = 0x0F;
}

void
vga_write_3c0( word port, byte data )
{
	(void) port;

	if( next_3c0_is_index )
		index_palette = data;
	else
	{
		vga_palette_register[index_palette] = data;
		vlog( VM_VIDEOMSG, "PALETTE[%u] = %02X", index_palette, data );
	}

	next_3c0_is_index = !next_3c0_is_index;
}

byte
vga_read_3c1( word port )
{
	(void) port;
	vlog( VM_VIDEOMSG, "Read PALETTE[%u] (=%02X)", index_palette, vga_palette_register[index_palette] );
	return vga_palette_register[index_palette];
}

byte
vga_read_miscellaneous_output_register( word port )
{
	(void) port;

	vlog( VM_VIDEOMSG, "Read MOR" );

	/*
	 * 0x01: I/O at 0x3Dx (otherwise at 0x3Bx)
	 * 0x02: RAM access enabled?
	 */

	return 0x03;

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
	(void) port;
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

byte
vga_getseq( word port )
{
	(void) port;
	//vlog( VM_VIDEOMSG, "reading seq %d, data is %02X", current_sequencer, io_sequencer[current_sequencer] );
	return io_sequencer[current_sequencer];
}

void
vga_setseq( word port, byte data )
{
	(void) port;
	//vlog( VM_VIDEOMSG, "writing to seq %d, data is %02X", current_sequencer, data );
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
	/*last_bit0 = !last_bit0;
	data |= last_bit0;*/

	next_3c0_is_index = true;

	return data;
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
	//vlog( VM_VIDEOMSG, "writing to reg2 %d, data is %02X", current_register2, data );
	io_register2[current_register2] = data;
}

byte
vga_getreg2( word port )
{
	(void) port;
	//vlog( VM_VIDEOMSG, "reading reg2 %d, data is %02X", current_register2, io_register2[current_register2] );
	return io_register2[current_register2];
}

void
vga_setbyte( dword a, byte d )
{
	/*
	 * fprintf(stderr,"mem_write: %02X:%04X = %02X <%d>, BM=%02X, ESR=%02X, SR=%02X\n", io_sequencer[2] & 0x0F, a-0xA0000, d, DRAWOP, BIT_MASK, io_register2[1], io_register2[0]);
	 */

	if( a >= 0xAFFFF )
	{
		vlog( VM_VIDEOMSG, "OOB write 0x%lx", a );
		mem_space[a] = d;
		return;
	}

	byte new_val[4];

	a -= 0xA0000;

	if( WRITE_MODE == 2 )
	{
		byte bitmask = BIT_MASK;

        new_val[0] = latch[0] & ~bitmask;
        new_val[1] = latch[1] & ~bitmask;
        new_val[2] = latch[2] & ~bitmask;
        new_val[3] = latch[3] & ~bitmask;

		switch( DRAWOP )
		{
			case 0:
				new_val[0] |= (d & 1) ? bitmask : 0;
				new_val[1] |= (d & 2) ? bitmask : 0;
				new_val[2] |= (d & 4) ? bitmask : 0;
				new_val[3] |= (d & 8) ? bitmask : 0;
				break;
			default:
				ui_kill();
				vlog( VM_VIDEOMSG, "Gaah, unsupported raster op %d in mode 2 :(\n", DRAWOP );
				vm_exit( 0 );
		}
	}
	else if( WRITE_MODE == 0 )
	{
		byte bitmask = BIT_MASK;
		byte set_reset = io_register2[0];
		byte enable_set_reset = io_register2[1];
		byte value = d;

		if( ROTATE )
		{
			vlog( VM_VIDEOMSG, "Rotate used!" );
			value = (value >> ROTATE) | (value << ( 8 - ROTATE ));
		}

		new_val[0] = latch[0] & ~bitmask;
		new_val[1] = latch[1] & ~bitmask;
		new_val[2] = latch[2] & ~bitmask;
		new_val[3] = latch[3] & ~bitmask;

		//fprintf( stderr, "new_val[] = {%02X, %02X, %02X, %02X}\n", new_val[0], new_val[1], new_val[2], new_val[3] );

		switch( DRAWOP )
		{
			case 0:
				new_val[0] |= ((enable_set_reset & 1)
					? ((set_reset & 1) ? bitmask : 0)
					: (value & bitmask));
				new_val[1] |= ((enable_set_reset & 2)
					? ((set_reset & 2) ? bitmask : 0)
					: (value & bitmask));
				new_val[2] |= ((enable_set_reset & 4)
					? ((set_reset & 4) ? bitmask : 0)
					: (value & bitmask));
				new_val[3] |= ((enable_set_reset & 8)
					? ((set_reset & 8) ? bitmask : 0)
					: (value & bitmask));
				break;
			case 1:
				new_val[0] |= ((enable_set_reset & 1)
					? ((set_reset & 1)
						? (~latch[0] & bitmask)
						: (latch[0] & bitmask))
					: (value & latch[0]) & bitmask);

				new_val[1] |= ((enable_set_reset & 2)
					? ((set_reset & 2)
						? (~latch[1] & bitmask)
						: (latch[1] & bitmask))
					: (value & latch[1]) & bitmask);

				new_val[2] |= ((enable_set_reset & 4)
					? ((set_reset & 4)
						? (~latch[2] & bitmask)
						: (latch[2] & bitmask))
					: (value & latch[2]) & bitmask);

				new_val[3] |= ((enable_set_reset & 8)
					? ((set_reset & 8)
						? (~latch[3] & bitmask)
						: (latch[3] & bitmask))
					: (value & latch[3]) & bitmask);
				break;
			case 3:
				new_val[0] |= ((enable_set_reset & 1)
					? ((set_reset & 1)
						? (~latch[0] & bitmask)
						: (latch[0] & bitmask))
					: (value ^ latch[0]) & bitmask);

				new_val[1] |= ((enable_set_reset & 2)
					? ((set_reset & 2)
						? (~latch[1] & bitmask)
						: (latch[1] & bitmask))
					: (value ^ latch[1]) & bitmask);

				new_val[2] |= ((enable_set_reset & 4)
					? ((set_reset & 4)
						? (~latch[2] & bitmask)
						: (latch[2] & bitmask))
					: (value ^ latch[2]) & bitmask);

				new_val[3] |= ((enable_set_reset & 8)
					? ((set_reset & 8)
						? (~latch[3] & bitmask)
						: (latch[3] & bitmask))
					: (value ^ latch[3]) & bitmask);
				break;
			default:
				vlog( VM_VIDEOMSG, "Unsupported raster operation %d", DRAWOP );
				ui_kill();
				vm_exit( 0 );
		}
	}
	else if( WRITE_MODE == 1 )
	{
		new_val[0] = latch[0];
		new_val[1] = latch[1];
		new_val[2] = latch[2];
		new_val[3] = latch[3];
	}
	else
	{
		vlog( VM_VIDEOMSG, "Unsupported 6845 write mode %d", WRITE_MODE );
		vm_exit( 0 );

		/* This is just here to make GCC stop worrying about accessing new_val[] uninitialized. */
		return;
	}

	/*
	 * Check first if any planes should be written.
	 */
	if( io_sequencer[2] & 0x0F )
	{
		if( io_sequencer[2] & 0x01 )
			vm_p0[a] = new_val[0];
		if( io_sequencer[2] & 0x02 )
			vm_p1[a] = new_val[1];
		if( io_sequencer[2] & 0x04 )
			vm_p2[a] = new_val[2];
		if( io_sequencer[2] & 0x08 )
			vm_p3[a] = new_val[3];

#ifdef VOMIT_DIRECT_SCREEN
		if( a < (640 * 480) )
			screen_direct_update( a );
#endif

		video_dirty = true;
	}
}

byte
vga_getbyte( dword a )
{
	if( READ_MODE != 0 )
	{
		vlog( VM_VIDEOMSG, "ZOMG! READ_MODE = %u", READ_MODE );
		ui_kill();
		vm_exit( 1 );
	}

	/* We're assuming READ_MODE == 0 now... */

	if( a < 0xB0000 )
	{
		a -= 0xA0000;
		latch[0] = vm_p0[a];
		latch[1] = vm_p1[a];
		latch[2] = vm_p2[a];
		latch[3] = vm_p3[a];
		/*
			fprintf(stderr, "mem_read: %02X {%02X, %02X, %02X, %02X}\n", latch[io_register2[4]], latch[0], latch[1], latch[2], latch[3]);
		*/
		return latch[io_register2[4]];
	}
	else
	{
		vlog( VM_VIDEOMSG, "OOB read 0x%lx", a );
#if 0
			g_debug_step = true;
			vm_debug();
#endif
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
	video_dirty = false;
}

void
set_video_dirty()
{
	video_dirty = true;
}

bool
is_video_dirty()
{
	return video_dirty;
}

void
clear_palette_dirty()
{
	palette_dirty = false;
}

bool
is_palette_dirty()
{
	return palette_dirty;
}
