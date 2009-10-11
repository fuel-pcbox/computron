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

static void vga_selreg(vomit_cpu_t *, word, byte );
static void vga_setreg(vomit_cpu_t *, word, byte );
static void vga_selreg2(vomit_cpu_t *, word, byte );
static void vga_setreg2(vomit_cpu_t *, word, byte );
static byte vga_getreg2(vomit_cpu_t *, word );
static void vga_selseq(vomit_cpu_t *, word, byte );
static void vga_setseq(vomit_cpu_t *, word, byte );
static byte vga_getseq(vomit_cpu_t *, word );
static byte vga_getreg(vomit_cpu_t *, word );
static byte vga_status(vomit_cpu_t *, word );
static byte vga_get_current_register(vomit_cpu_t *, word );
static void vga_write_3c0(vomit_cpu_t *, word port, byte data );
static byte vga_read_3c1(vomit_cpu_t *, word port );
static byte vga_read_miscellaneous_output_register(vomit_cpu_t *, word port );

static bool next_3c0_is_index = true;

static bool video_dirty = false;
bool palette_dirty = true;

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
vga_write_3c0(vomit_cpu_t *, word port, byte data )
{
    (void) port;

    if( next_3c0_is_index )
        index_palette = data;
    else
    {
        vga_palette_register[index_palette] = data;
        //vlog( VM_VIDEOMSG, "PALETTE[%u] = %02X", index_palette, data );
    }

    next_3c0_is_index = !next_3c0_is_index;
}

byte
vga_read_3c1(vomit_cpu_t *, word port )
{
    (void) port;
    //vlog( VM_VIDEOMSG, "Read PALETTE[%u] (=%02X)", index_palette, vga_palette_register[index_palette] );
    return vga_palette_register[index_palette];
}

byte
vga_read_miscellaneous_output_register(vomit_cpu_t *, word port )
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
vga_get_current_register(vomit_cpu_t *, word port )
{
    (void) port;
    return current_register;
}

void
vga_selreg(vomit_cpu_t *, word port, byte data )
{
    (void) port;
    /* mask off unused bits */
    current_register = data & 0x1F;
}

void
vga_setreg(vomit_cpu_t *, word port, byte data )
{
    (void) port;
    io_register[current_register] = data;
}

byte
vga_getreg(vomit_cpu_t *, word port )
{
    (void) port;
    return io_register[current_register];
}

void
vga_selseq(vomit_cpu_t *, word port, byte data )
{
    (void) port;
    /* mask off unused bits */
    current_sequencer = data & 0x1F;
}

BYTE vga_getseq(vomit_cpu_t *, WORD)
{
    //vlog( VM_VIDEOMSG, "reading seq %d, data is %02X", current_sequencer, io_sequencer[current_sequencer] );
    return io_sequencer[current_sequencer];
}

void vga_setseq(vomit_cpu_t *, WORD, BYTE value)
{
    //vlog( VM_VIDEOMSG, "writing to seq %d, data is %02X", current_sequencer, data );
    io_sequencer[current_sequencer] = value;
}

BYTE
vga_status(vomit_cpu_t *, word)
{
    static bool last_bit0 = 0;
    BYTE value;

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
    value = 0x0C;

    /* Microsoft DEFRAG expects bit 0 to "blink", so we'll flip it between reads. */
    /*last_bit0 = !last_bit0;
    data |= last_bit0;*/

    next_3c0_is_index = true;

    return value;
}

BYTE vga_read_register(BYTE index)
{
    assert(index <= 0x12);
    return io_register[index];
}

BYTE vga_read_register2(BYTE index)
{
	// TODO: Check if 12 is the right number here...
    assert(index <= 0x12);
    return io_register2[index];
}

BYTE vga_read_sequencer(BYTE index)
{
    assert(index <= 0x4);
    return io_sequencer[index];
}

void vga_write_register(BYTE index, BYTE value)
{
    assert(index <= 0x12);
    io_register[index] = value;
}

void
vga_selreg2(vomit_cpu_t *, word port, byte data)
{
    (void) port;
    current_register2 = data;
}

void
vga_setreg2(vomit_cpu_t *, word port, byte data )
{
    (void) port;
    //vlog( VM_VIDEOMSG, "writing to reg2 %d, data is %02X", current_register2, data );
    io_register2[current_register2] = data;
}

byte
vga_getreg2(vomit_cpu_t *, word port )
{
    (void) port;
    //vlog( VM_VIDEOMSG, "reading reg2 %d, data is %02X", current_register2, io_register2[current_register2] );
    return io_register2[current_register2];
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
