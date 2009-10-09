#include "vomit.h"
#include "8086.h"
#include "debug.h"
#include <string.h>
#include <stdio.h>

static void set_video_mode();
static void select_active_display_page();
static void get_video_state();
static void set_cursor_type();
static void set_cursor_position();
static void get_cursor_position();
static void write_character_at_cursor();
static void read_character_and_attribute_at_cursor();
static void write_character_and_attribute_at_cursor();
static void write_text_in_teletype_mode();
static void scroll_active_page_up();
static void scroll_active_page_down();
static void video_subsystem_configuration();
static void video_display_combination();
static void character_generator();
static word columns();
static byte rows();
static void store_cursor( word cursor );
static word load_cursor_word();
static void set_dac_color_register();
static void write_graphics_pixel_at_coordinate();
void load_cursor( byte *row, byte *column );

extern void set_video_dirty();

void
video_bios_init()
{
    /* Mode 3 - 80x25 16 color text */
    g_cpu->memory[0x449] = 3;

    /* 80 columns */
    g_cpu->memory[0x44A] = 80;
    g_cpu->memory[0x44B] = 0;

    /* 25 rows */
    g_cpu->memory[0x484] = 24;

    /* Cursor type */
    g_cpu->memory[0x460] = 0x0E;
    g_cpu->memory[0x461] = 0x0D;

    /* Video Display Combination */
    g_cpu->memory[0x48A] = 0x08;

    /* Active 6845 CRT I/O base */
    g_cpu->memory[0x463] = 0xD4;
    g_cpu->memory[0x464] = 0x03;

    vga_write_register( 0x0A, 0x06 );
    vga_write_register( 0x0B, 0x07 );
}

void
bios_interrupt10()
{
    switch( g_cpu->regs.B.AH )
    {
        case 0x00: set_video_mode(); break;
        case 0x01: set_cursor_type(); break;
        case 0x02: set_cursor_position(); break;
        case 0x03: get_cursor_position(); break;
#if 0
        case 0x04: read_light_pen(); break;
#endif
        case 0x05: select_active_display_page(); break;
        case 0x06: scroll_active_page_up(); break;
        case 0x07: scroll_active_page_down(); break;
        case 0x08: read_character_and_attribute_at_cursor(); break;
        case 0x09: write_character_and_attribute_at_cursor(); break;
        case 0x0a: write_character_at_cursor(); break;
#if 0
        case 0x0b: /*set_color_palette();*/ break;
#endif
        case 0x0c: write_graphics_pixel_at_coordinate(); break;
#if 0
        case 0x0d: /*read_graphics_pixel_at_coordinate();*/ break;
#endif
        case 0x0e: write_text_in_teletype_mode(); break;
        case 0x0f: get_video_state(); break;
        case 0x12: video_subsystem_configuration(); break;
        case 0x1a: video_display_combination(); break;
        case 0x11: character_generator(); break;
        case 0x10: set_dac_color_register(); break;
        default:
            vlog( VM_VIDEOMSG, "Interrupt 10, function %02X requested, AL=%02X, BH=%02X", g_cpu->regs.B.AH, g_cpu->regs.B.AL, g_cpu->regs.B.BH );
    }
}

void
set_cursor_position()
{
    /* BH is page# */

    if( g_cpu->regs.B.BH != 0 )
        vlog( VM_VIDEOMSG, "Cursor moved on page %u", g_cpu->regs.B.BH );

    byte row = g_cpu->regs.B.DH;
    byte column = g_cpu->regs.B.DL;
    //vlog( VM_VIDEOMSG, "Move to %u, %u", row, column );

    word cursor = row * columns() + column;

    store_cursor( cursor );
}

void
get_cursor_position()
{
    /* Cursor row and column. */
    load_cursor( &g_cpu->regs.B.DH, &g_cpu->regs.B.DL );

    /* Starting (top) scanline. */
    g_cpu->regs.B.CH = g_cpu->memory[0x461];
    /* Ending (bottom) scanline. */
    g_cpu->regs.B.CL = g_cpu->memory[0x460];
}

void
write_character_at_cursor()
{
    /* BH is page# */
    /* BL is foreground color (ignored?) */

    byte row, column;
    word cursor;

    /* Check count of characters to write. */
    if( g_cpu->regs.W.CX == 0 )
        return;

    load_cursor( &row, &column );
    cursor = row * columns() + column;

    /* XXX: 0xB800 is hard-coded for now. */
    vomit_cpu_memory_write8(g_cpu, 0xB800, cursor << 1, g_cpu->regs.B.AL );
}

void
read_character_and_attribute_at_cursor()
{
    /* XXX: Works *only* with page 0. */
    /* BH is page# */

    word cursor = load_cursor_word();

    /* XXX: 0xB800 is hard-coded for now. */
    g_cpu->regs.B.AH = vomit_cpu_memory_read8(g_cpu, 0xB800, (cursor << 1) + 1 );
    g_cpu->regs.B.AL = vomit_cpu_memory_read8(g_cpu, 0xB800, cursor << 1 );
}

void
write_character_and_attribute_at_cursor()
{
    /* BH is page# */
    byte row, column;
    word cursor;

    /* Check count of characters to write. */
    if( g_cpu->regs.W.CX == 0 )
    {
        vlog( VM_VIDEOMSG, "Asked to write zero chars!?\n" );
        return;
    }

    load_cursor( &row, &column );
    cursor = row * columns() + column;

    if( g_cpu->memory[0x449] != 0x03 )
    {
        vlog( VM_VIDEOMSG, "Writing text to screen but not in mode 3.." );
    }
    /* XXX: 0xB800 is hard-coded for now. */
    vomit_cpu_memory_write8(g_cpu, 0xB800, cursor << 1, g_cpu->regs.B.AL );
    vomit_cpu_memory_write8(g_cpu, 0xB800, (cursor << 1) + 1, g_cpu->regs.B.BL );
}

void
write_text_in_teletype_mode()
{
    /* BH is page# */

    byte row, column;
    word cursor;

    byte ch = g_cpu->regs.B.AL;
    byte attr = g_cpu->regs.B.BL;

    load_cursor( &row, &column );

    //fprintf( stderr, "<%02u,%02u>%c", row, column, ch );
    //fprintf( stderr, "%c", ch );

    cursor = row * columns() + column;

    switch( ch )
    {
        case '\n':
            row++;
            cursor = row * columns() + column;
            break;
        case '\r':
            column = 0;
            cursor = row * columns() + column;
            break;
        case 0x08:
            if( column > 0)
                --cursor;
            break;
        case '\t':
            {
                word space = attr << 8 | ch;

                /* 1 tab -- 4 spaces ;-) */
                vomit_cpu_memory_write16(g_cpu, 0xB800, cursor++ << 1, space );
                vomit_cpu_memory_write16(g_cpu, 0xB800, cursor++ << 1, space );
                vomit_cpu_memory_write16(g_cpu, 0xB800, cursor++ << 1, space );
                vomit_cpu_memory_write16(g_cpu, 0xB800, cursor++ << 1, space );
            }
            break;
        default:
            vomit_cpu_memory_write8(g_cpu, 0xB800, cursor << 1, ch );
            vomit_cpu_memory_write8(g_cpu, 0xB800, (cursor << 1) + 1, attr );
            cursor++;
    }
    if( columns() == 0 || rows() == 0 )
    {
        /* Something has burninated the BDA screen size data. */
        vlog( VM_VIDEOMSG, "BDA screen size is corrupted." );
    }
    else if( cursor >= (rows() * columns()) )
    {
        int i;
        memmove( g_cpu->memory + 0xB8000, g_cpu->memory + 0xB8000 + columns() * 2, (columns() * (rows() - 1))*2 );
        for( i = 0; i < columns() * 2; i += 2 )
        {
            g_cpu->memory[0xB8000 + 4000 - 160 + i] = 0x20;
            g_cpu->memory[0xB8000 + 4000 - 160 + i + 1] = attr;
        }
        cursor = columns() * (rows() - 1);
    }
    store_cursor( cursor );
}

void
scroll_active_page_down()
{
    vlog( VM_VIDEOMSG, "Come on (and implement scroll_active_page_down())" );
}

void
scroll_active_page_up()
{
    vga_scrollup( g_cpu->regs.B.CL, g_cpu->regs.B.CH, g_cpu->regs.B.DL, g_cpu->regs.B.DH, g_cpu->regs.B.AL, g_cpu->regs.B.BH );
}

void
set_video_mode()
{
    byte mode = g_cpu->regs.B.AL;
    byte actual_mode = g_cpu->regs.B.AL & ~0x80;

    byte last_mode = g_cpu->memory[0x449];

    g_cpu->memory[0x449] = mode;

    g_cpu->memory[0x487] &= ~0x80;
    g_cpu->memory[0x487] |= mode & 0x80;

    switch( actual_mode )
    {
        case 0x03:
            g_cpu->memory[0x44A] = 80;
            g_cpu->memory[0x44B] = 0;
            g_cpu->memory[0x484] = 24;

            if( (mode & 0x80) == 0 )
            {
                // Blank it!
                word space = (0x07 << 8) | ' ';
                int y, x, i = 0;
                for( y = 0; y < 25; ++y )
                {
                    for( x = 0; x < 80; ++x )
                    {
                        //vomit_cpu_memory_write16(g_cpu, 0xB800, i++ << 1, space );
                    }
                }
            }

            break;
    }

    if( mode != last_mode )
        vlog( VM_VIDEOMSG, "Mode %d (hex %02X) selected", mode, mode );

#if 0
    vlog( VM_VIDEOMSG, "=== Begin CRT register update ===" );

    (void) cpu_in( 0x3DA );
    cpu_out( 0x3C0, 0x10 );
    cpu_out( 0x3C0, actual_mode == 0x03 ? 0x0C : 0x01 );
    cpu_out( 0x3C0, 0x11 );
    cpu_out( 0x3C0, actual_mode == 0x03 ? 0x00 : 0x00 );
    cpu_out( 0x3C0, 0x12 );
    cpu_out( 0x3C0, actual_mode == 0x03 ? 0x0F : 0x0F );
    cpu_out( 0x3C0, 0x13 );
    cpu_out( 0x3C0, actual_mode == 0x03 ? 0x08 : 0x00 );
    cpu_out( 0x3C0, 0x14 );
    cpu_out( 0x3C0, actual_mode == 0x03 ? 0x00 : 0x00 );

    vlog( VM_VIDEOMSG, "=== End CRT register update ===" );
#endif
}

void
get_video_state()
{
    /* Screen columns. */
    g_cpu->regs.B.AH = columns();

    /* Active page. */
    g_cpu->regs.B.BH = 0;

    /* Current video mode. */
    g_cpu->regs.B.AL = g_cpu->memory[0x449];
}

void
select_active_display_page()
{
    vlog( VM_VIDEOMSG, "Page %d selected (not handled!)", g_cpu->regs.B.AL );

    /* XXX: Note that nothing actually happens here ;-) */
}

void
video_subsystem_configuration()
{
    vlog( VM_VIDEOMSG, "INT 10,12 with BL=%02X", g_cpu->regs.B.BL );

    if( g_cpu->regs.B.BL == 0x10 )
    {
        /* Report "color mode" in effect. */
        g_cpu->regs.B.BH = 0x00;

        /* Report 256k of EGA memory. */
        g_cpu->regs.B.BL = 0x00;

        g_cpu->regs.W.CX = 0;
    }
    else if( g_cpu->regs.B.BL == 0x33 )
    {
        vlog( VM_VIDEOMSG, "Gray scale summing mode %02X", g_cpu->regs.B.AL );

        g_cpu->regs.B.AL = 0x12;
    }
}

void
set_cursor_type()
{
    byte start, end;

    start = g_cpu->regs.B.CH & 0x1F;
    end = g_cpu->regs.B.CL & 0x1F;

    vlog( VM_VIDEOMSG, "Cursor set: %u to %u%s", start, end, g_cpu->regs.W.CX == 0x2000 ? " (disabled)" : "" );

    vga_write_register( 0x0A, start );
    vga_write_register( 0x0B, end );

    g_cpu->memory[0x461] = start;
    g_cpu->memory[0x460] = end;
}

BYTE get_video_mode()
{
    return g_cpu->memory[0x449];
}

WORD columns()
{
    return vomit_cpu_memory_read16(g_cpu, 0x0040, 0x004A);
}

BYTE rows()
{
    return g_cpu->memory[0x484] + 1;
}

void
store_cursor( word cursor )
{
    vga_write_register(0x0E, cursor >> 8 );
    vga_write_register(0x0F, cursor & 0xFF );

    g_cpu->memory[0x450] = cursor / columns();
    g_cpu->memory[0x451] = cursor % columns();

    set_video_dirty();
}

void
load_cursor( byte *row, byte *column )
{
    word cursor = vga_read_register(0x0E) << 8 | vga_read_register(0x0F);

    *row = cursor / columns();
    *column = cursor % columns();

    g_cpu->memory[0x450] = cursor / columns();
    g_cpu->memory[0x451] = cursor % columns();
}

word
load_cursor_word()
{
    return vga_read_register(0x0E) << 8 | vga_read_register(0x0F);
}

void
video_display_combination()
{
    if( g_cpu->regs.B.AL == 0 )
    {
        g_cpu->regs.B.BL = 0x08;
        g_cpu->regs.B.BH = 0x00;
        g_cpu->regs.B.AL = 0x1A;
        vlog( VM_VIDEOMSG, "Video display combination requested." );
    }
    else
    {
        vlog( VM_VIDEOMSG, "Video display combination overwrite requested." );
    }
}

void
character_generator()
{
    if( g_cpu->regs.B.AL == 0x30 )
    {
        if( g_cpu->regs.B.BH == 0 )
        {
            g_cpu->regs.W.BP = (g_cpu->memory[0x1F * 4 + 1] << 8) | g_cpu->memory[0x1F * 4];
            g_cpu->ES = (g_cpu->memory[0x1F * 4 + 3] << 8) | g_cpu->memory[0x1F * 4 + 2];
            vlog( VM_VIDEOMSG, "Character generator returned pointer to Interrupt 1F" );
            return;
        }
    }

    vlog( VM_VIDEOMSG, "Unknown character generator request: AL=%02X, BH=%02X\n", g_cpu->regs.B.AL, g_cpu->regs.B.BH );
}

void
set_dac_color_register()
{
    if( g_cpu->regs.B.AL == 0x00 )
    {
        vlog( VM_VIDEOMSG, "Set palette register %u to color %u", g_cpu->regs.B.BL, g_cpu->regs.B.BH );
        vga_palette_register[g_cpu->regs.B.BL] = g_cpu->regs.B.BH;
    }
    if( g_cpu->regs.B.AL == 0x10 )
    {
        vlog( VM_VIDEOMSG, "Set color %d to #%02x%02x%02x", g_cpu->regs.W.BX, g_cpu->regs.B.DH<<2, g_cpu->regs.B.CH<<2, g_cpu->regs.B.CL<<2 );

        vga_color_register[g_cpu->regs.W.BX & 0xFF].r = g_cpu->regs.B.DH;
        vga_color_register[g_cpu->regs.W.BX & 0xFF].g = g_cpu->regs.B.CH;
        vga_color_register[g_cpu->regs.W.BX & 0xFF].b = g_cpu->regs.B.CL;

    }
    else if( g_cpu->regs.B.AL == 0x02 )
    {
        int i;
        vlog( VM_VIDEOMSG, "Loading palette from %04X:%04X", g_cpu->ES, g_cpu->regs.W.DX );
        for( i = 0; i < 17; ++i )
        {
            vga_palette_register[i] = vomit_cpu_memory_read8(g_cpu, g_cpu->ES, g_cpu->regs.W.DX + i );
            //vlog( VM_VIDEOMSG, "Palette(%u): %02X", i, vga_palette_register[i] );
        }
    }
    else
    {
        vlog( VM_VIDEOMSG, "Unsupported palette operation %02X", g_cpu->regs.B.AL );
    }
    extern bool palette_dirty;
    palette_dirty = true;
}


void
write_graphics_pixel_at_coordinate()
{
    // AL = color value
    // BH = page#
    // CX = column
    // DX = row

    if( g_cpu->memory[0x449] != 0x12 )
    {
        vlog( VM_ALERT, "Poking pixels in mode %02X, dunno how to handle that\n", g_cpu->memory[0x449] );
        vm_exit( 1 );
    }

    //vlog( VM_ALERT, "y = %03u, x = %03u, color = %02X\n", g_cpu->regs.W.DX, g_cpu->regs.W.CX, g_cpu->regs.B.AL );
    extern byte vm_p0[];
    extern byte vm_p1[];
    extern byte vm_p2[];
    extern byte vm_p3[];

    word offset = (g_cpu->regs.W.DX * 80) + (g_cpu->regs.W.CX / 8);
    byte bit = g_cpu->regs.W.CX % 8;

    vm_p0[offset] &= ~(0x80 >> bit);
    vm_p1[offset] &= ~(0x80 >> bit);
    vm_p2[offset] &= ~(0x80 >> bit);
    vm_p3[offset] &= ~(0x80 >> bit);

    if( g_cpu->regs.B.AL & 0x01 )
        vm_p0[offset] |= (0x80 >> bit);

    if( g_cpu->regs.B.AL & 0x02 )
        vm_p1[offset] |= (0x80 >> bit);

    if( g_cpu->regs.B.AL & 0x04 )
        vm_p2[offset] |= (0x80 >> bit);

    if( g_cpu->regs.B.AL & 0x08 )
        vm_p3[offset] |= (0x80 >> bit);

    set_video_dirty();
}

void
putpixel0D( word row, word column, byte pixel )
{
    // AL = color value
    // BH = page#
    // CX = column
    // DX = row

    //printf( "putpixel at %03u,%03u: %02X\n", row, column, pixel );

    extern byte vm_p0[];
    extern byte vm_p1[];
    extern byte vm_p2[];
    extern byte vm_p3[];

    word offset = (row * 40) + (g_cpu->regs.W.CX / 8);
    byte bit = g_cpu->regs.W.CX % 8;

    vm_p0[offset] &= ~(0x80 >> bit);
    vm_p1[offset] &= ~(0x80 >> bit);
    vm_p2[offset] &= ~(0x80 >> bit);
    vm_p3[offset] &= ~(0x80 >> bit);

    if( pixel & 0x01 )
        vm_p0[offset] |= (0x80 >> bit);

    if( pixel & 0x02 )
        vm_p1[offset] |= (0x80 >> bit);

    if( pixel & 0x04 )
        vm_p2[offset] |= (0x80 >> bit);

    if( pixel & 0x08 )
        vm_p3[offset] |= (0x80 >> bit);

    set_video_dirty();
}

void
vga_scrollup (byte x1, byte y1, byte x2, byte y2, byte num, byte attr) {
    byte x, y, i;

    byte *video_memory;

    if( g_cpu->memory[0x449] == 0x0D || g_cpu->memory[0x449] == 0x12 )
        video_memory = g_cpu->memory + 0xB8000;
    else
        video_memory = g_cpu->memory + 0xB8000;

    // TODO: Scroll graphics when in graphics mode (using text coordinates)

    //vlog( VM_VIDEOMSG, "vga_scrollup( %d, %d, %d, %d, %d )", x1, y1, x2, y2, num);
    if ( (num == 0 ) || ( num > rows() ) ) {
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

