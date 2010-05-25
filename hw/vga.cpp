#include "vomit.h"
#include "vcpu.h"
#include "vga.h"
#include "debug.h"
#include "disasm.h"
#include <string.h>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

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

static byte dac_data_index = 0;
static byte dac_data_subindex = 0;

static QMutex s_paletteMutex;

static void vga_selreg(VCpu*, word, byte );
static void vga_setreg(VCpu*, word, byte );
static void vga_selreg2(VCpu*, word, byte );
static void vga_setreg2(VCpu*, word, byte );
static byte vga_getreg2(VCpu*, word );
static void vga_selseq(VCpu*, word, byte );
static void vga_setseq(VCpu*, word, byte );
static byte vga_getseq(VCpu*, word );
static byte vga_getreg(VCpu*, word );
static byte vga_status(VCpu*, word );
static byte vga_get_current_register(VCpu*, word );
static void vga_write_3c0(VCpu*, word port, byte data );
static byte vga_read_3c1(VCpu*, word port );
static byte vga_read_miscellaneous_output_register(VCpu*, word port );
static void vga_dac_write_address(VCpu*, WORD, BYTE);
static BYTE vga_dac_read_data(VCpu*, WORD);
static void vga_dac_write_data(VCpu*, WORD, BYTE);
static BYTE vga_fcr_read(VCpu*, WORD);
static void vga_fcr_write(VCpu*, WORD, BYTE);

static bool next_3c0_is_index = true;

static bool palette_dirty = true;

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

void
vga_init()
{
    vm_listen( 0x3b4, vga_get_current_register, vga_selreg );
    vm_listen( 0x3b5, vga_getreg, vga_setreg );
    vm_listen(0x3ba, vga_status, vga_fcr_write);

    vm_listen( 0x3c0, 0L, vga_write_3c0 );
    vm_listen( 0x3c1, vga_read_3c1, 0L );

    vm_listen( 0x3c4, 0L, vga_selseq );
    vm_listen( 0x3c5, vga_getseq, vga_setseq );
    vm_listen(0x3c8, 0L, vga_dac_write_address);
    vm_listen(0x3c9, vga_dac_read_data, vga_dac_write_data);
    vm_listen(0x3ca, vga_fcr_read, 0L);
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

    dac_data_index = 0;
    dac_data_subindex = 0;
}

void
vga_write_3c0(VCpu*, word port, byte data )
{
    QMutexLocker locker(&s_paletteMutex);
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

BYTE vga_read_3c1(VCpu*, WORD)
{
    QMutexLocker locker(&s_paletteMutex);
    //vlog(VM_VIDEOMSG, "Read PALETTE[%u] (=%02X)", index_palette, vga_palette_register[index_palette]);
    return vga_palette_register[index_palette];
}

byte
vga_read_miscellaneous_output_register(VCpu*, word port )
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
vga_get_current_register(VCpu*, word port )
{
    (void) port;
    return current_register;
}

void
vga_selreg(VCpu*, word port, byte data )
{
    (void) port;
    /* mask off unused bits */
    current_register = data & 0x1F;
}

void
vga_setreg(VCpu*, word port, byte data )
{
    (void) port;
    io_register[current_register] = data;
}

byte
vga_getreg(VCpu*, word port )
{
    (void) port;
    return io_register[current_register];
}

void
vga_selseq(VCpu*, word port, byte data )
{
    (void) port;
    /* mask off unused bits */
    current_sequencer = data & 0x1F;
}

BYTE vga_getseq(VCpu*, WORD)
{
    //vlog( VM_VIDEOMSG, "reading seq %d, data is %02X", current_sequencer, io_sequencer[current_sequencer] );
    return io_sequencer[current_sequencer];
}

void vga_setseq(VCpu*, WORD, BYTE value)
{
    //vlog( VM_VIDEOMSG, "writing to seq %d, data is %02X", current_sequencer, data );
    io_sequencer[current_sequencer] = value;
}

BYTE vga_status(VCpu*, WORD)
{
    /*
     * 6845 - Port 3DA Status Register
     *
     * |7|6|5|4|3|2|1|0|  3DA Status Register
     *  | | | | | | | `---- 1 = display enable, RAM access is OK
     *  | | | | | | `----- 1 = light pen trigger set
     *  | | | | | `------ 0 = light pen on, 1 = light pen off
     *  | | | | `------- 1 = vertical retrace, RAM access OK for next 1.25ms
     *  `-------------- unused
     *
     */

    /* 0000 0100 */
    BYTE value = 0x04;

    if (vomit_in_vretrace())
        value |= 0x08;

    next_3c0_is_index = true;

    return value;
}

BYTE vga_read_register(BYTE index)
{
    VM_ASSERT(index <= 0x12);
    return io_register[index];
}

BYTE vga_read_register2(BYTE index)
{
	// TODO: Check if 12 is the right number here...
    VM_ASSERT(index <= 0x12);
    return io_register2[index];
}

BYTE vga_read_sequencer(BYTE index)
{
    VM_ASSERT(index <= 0x4);
    return io_sequencer[index];
}

void vga_write_register(BYTE index, BYTE value)
{
    VM_ASSERT(index <= 0x12);
    io_register[index] = value;
}

void
vga_selreg2(VCpu*, word port, byte data)
{
    (void) port;
    current_register2 = data;
}

void
vga_setreg2(VCpu*, word port, byte data )
{
    (void) port;
    //vlog( VM_VIDEOMSG, "writing to reg2 %d, data is %02X", current_register2, data );
    io_register2[current_register2] = data;
}

byte
vga_getreg2(VCpu*, word port )
{
    (void) port;
    //vlog( VM_VIDEOMSG, "reading reg2 %d, data is %02X", current_register2, io_register2[current_register2] );
    return io_register2[current_register2];
}

void mark_palette_dirty()
{
    QMutexLocker locker(&s_paletteMutex);
    palette_dirty = true;
}

void clear_palette_dirty()
{
    QMutexLocker locker(&s_paletteMutex);
    palette_dirty = false;
}

bool is_palette_dirty()
{
    QMutexLocker locker(&s_paletteMutex);
    return palette_dirty;
}

void vga_dac_write_address(VCpu*, WORD, BYTE data)
{
    // vlog(VM_VIDEOMSG, "DAC register %02X selected", data);
    dac_data_index = data;
    dac_data_subindex = 0;
}

BYTE vga_dac_read_data(VCpu*, WORD)
{
    BYTE data = 0;
    switch (dac_data_subindex) {
    case 0:
        data = vga_color_register[dac_data_index].r;
        break;
    case 1:
        data = vga_color_register[dac_data_index].g;
        break;
    case 2:
        data = vga_color_register[dac_data_index].b;
        break;
    }

    // vlog(VM_VIDEOMSG, "Reading component %u of color %02X (%02X)", dac_data_subindex, dac_data_index, data);

    if (++dac_data_subindex >= 3) {
        dac_data_subindex = 0;
        ++dac_data_index;
    }

    return data;
}

void vga_dac_write_data(VCpu* cpu, WORD, BYTE data)
{
    // vlog(VM_VIDEOMSG, "Setting component %u of color %02X to %02X", dac_data_subindex, dac_data_index, data);
    switch (dac_data_subindex) {
    case 0:
        vga_color_register[dac_data_index].r = data;
        break;
    case 1:
        vga_color_register[dac_data_index].g = data;
        break;
    case 2:
        vga_color_register[dac_data_index].b = data;
        break;
    }

    if (++dac_data_subindex >= 3) {
        dac_data_subindex = 0;
        ++dac_data_index;
    }

    mark_palette_dirty();
    cpu->vgaMemory->syncPalette();
}

BYTE vga_fcr_read(VCpu*, WORD)
{
    return 0x00;
}

void vga_fcr_write(VCpu*, WORD, BYTE)
{
}
