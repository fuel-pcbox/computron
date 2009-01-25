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

void
video_bios_init()
{
	/* Mode 3 - 80x25 16 color text */
	mem_space[0x449] = 3;

	/* 80 columns */
	mem_space[0x44A] = 80;
	mem_space[0x44B] = 0;

	/* 25 rows */
	mem_space[0x484] = 24;

	/* Cursor type */
	mem_space[0x460] = 0x0E;
	mem_space[0x461] = 0x0D;

	/* Video Display Combination */
	mem_space[0x48A] = 0x08;

	/* Active 6845 CRT I/O base */
	mem_space[0x463] = 0xD4;
	mem_space[0x464] = 0x03;

	vga_write_register( 0x0A, 0x06 );
	vga_write_register( 0x0B, 0x07 );
}

void
bios_interrupt10()
{
	switch( cpu.regs.B.AH )
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
			vlog( VM_VIDEOMSG, "Interrupt 10, function %02X requested, AL=%02X, BH=%02X", cpu.regs.B.AH, cpu.regs.B.AL, cpu.regs.B.BH );
	}
}

void
set_cursor_position()
{
	/* BH is page# */

	if( cpu.regs.B.BH != 0 )
		vlog( VM_VIDEOMSG, "Cursor moved on page %u", cpu.regs.B.BH );

	byte row = cpu.regs.B.DH;
	byte column = cpu.regs.B.DL;
	//vlog( VM_VIDEOMSG, "Move to %u, %u", row, column );

	word cursor = row * columns() + column;

	store_cursor( cursor );
}

void
get_cursor_position()
{
	/* Cursor row and column. */
	load_cursor( &cpu.regs.B.DH, &cpu.regs.B.DL );

	/* Starting (top) scanline. */
	cpu.regs.B.CH = mem_space[0x461];
	/* Ending (bottom) scanline. */
	cpu.regs.B.CL = mem_space[0x460];
}

void
write_character_at_cursor()
{
	/* BH is page# */
	/* BL is foreground color (ignored?) */

	byte row, column;
	word cursor;

	/* Check count of characters to write. */
	if( cpu.regs.W.CX == 0 )
		return;

	load_cursor( &row, &column );
	cursor = row * columns() + column;

	/* XXX: 0xB800 is hard-coded for now. */
	mem_setbyte( 0xB800, cursor << 1, cpu.regs.B.AL );
}

void
read_character_and_attribute_at_cursor()
{
	/* XXX: Works *only* with page 0. */
	/* BH is page# */

	word cursor = load_cursor_word();

	/* XXX: 0xB800 is hard-coded for now. */
	cpu.regs.B.AH = mem_getbyte( 0xB800, (cursor << 1) + 1 );
	cpu.regs.B.AL = mem_getbyte( 0xB800, cursor << 1 );
}

void
write_character_and_attribute_at_cursor()
{
	/* BH is page# */
	byte row, column;
	word cursor;

	/* Check count of characters to write. */
	if( cpu.regs.W.CX == 0 )
		return;

	load_cursor( &row, &column );
	cursor = row * columns() + column;

	/* XXX: 0xB800 is hard-coded for now. */
	mem_setbyte( 0xB800, cursor << 1, cpu.regs.B.AL );
	mem_setbyte( 0xB800, (cursor << 1) + 1, cpu.regs.B.BL );
}

void
write_text_in_teletype_mode()
{
	/* BH is page# */

	byte row, column;
	word cursor;

	byte ch = cpu.regs.B.AL;
	byte attr = cpu.regs.B.BL;

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
				mem_setword( 0xB800, cursor++ << 1, space );
				mem_setword( 0xB800, cursor++ << 1, space );
				mem_setword( 0xB800, cursor++ << 1, space );
				mem_setword( 0xB800, cursor++ << 1, space );
			}
			break;
		default:
			mem_setbyte( 0xB800, cursor << 1, ch );
			mem_setbyte( 0xB800, (cursor << 1) + 1, attr );
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
		memmove( mem_space + 0xB8000, mem_space + 0xB8000 + columns() * 2, (columns() * (rows() - 1))*2 );
		for( i = 0; i < columns() * 2; i += 2 )
		{
			mem_space[0xB8000 + 4000 - 160 + i] = 0x20;
			mem_space[0xB8000 + 4000 - 160 + i + 1] = attr;
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
	vga_scrollup( cpu.regs.B.CL, cpu.regs.B.CH, cpu.regs.B.DL, cpu.regs.B.DH, cpu.regs.B.AL, cpu.regs.B.BH );
}

void
set_video_mode()
{
	byte mode = cpu.regs.B.AL;
	byte actual_mode = cpu.regs.B.AL & ~0x80;

	byte last_mode = mem_space[0x449];

	mem_space[0x449] = mode;

	mem_space[0x487] &= ~0x80;
	mem_space[0x487] |= mode & 0x80;

	switch( actual_mode )
	{
		case 0x03:
			mem_space[0x44A] = 80;
			mem_space[0x44B] = 0;
			mem_space[0x484] = 24;

			if( (mode & 0x80) == 0 )
			{
				// Blank it!
				word space = (0x07 << 8) | ' ';
				int i = 0;
				for( int y = 0; y < 25; ++y )
				{
					for( int x = 0; x < 80; ++x )
					{
						//mem_setword( 0xB800, i++ << 1, space );
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
	cpu.regs.B.AH = columns();

	/* Active page. */
	cpu.regs.B.BH = 0;

	/* Current video mode. */
	cpu.regs.B.AL = mem_space[0x449];
}

void
select_active_display_page()
{
	//vlog( VM_VIDEOMSG, "Page %d selected (not handled!)", cpu.regs.B.AL );

	/* XXX: Note that nothing actually happens here ;-) */
}

void
video_subsystem_configuration()
{
	vlog( VM_VIDEOMSG, "INT 10,12 with BL=%02X", cpu.regs.B.BL );

	if( cpu.regs.B.BL == 0x10 )
	{
		/* Report "color mode" in effect. */
		cpu.regs.B.BH = 0x00;

		/* Report 256k of EGA memory. */
		cpu.regs.B.BL = 0x00;

		cpu.regs.W.CX = 0;
	}
	else if( cpu.regs.B.BL == 0x33 )
	{
		vlog( VM_VIDEOMSG, "Gray scale summing mode %02X", cpu.regs.B.AL );

		cpu.regs.B.AL = 0x12;
	}
}

void
set_cursor_type()
{
	byte start, end;

	start = cpu.regs.B.CH & 0x1F;
	end = cpu.regs.B.CL & 0x1F;

	vlog( VM_VIDEOMSG, "Cursor set: %u to %u%s", start, end, cpu.regs.W.CX == 0x2000 ? " (disabled)" : "" );

	vga_write_register( 0x0A, start );
	vga_write_register( 0x0B, end );

	mem_space[0x461] = start;
	mem_space[0x460] = end;
}

byte
get_video_mode()
{
	return mem_space[0x449];
}

word
columns()
{
	return mem_getword( 0x0040, 0x004A );
}

byte
rows()
{
	return mem_space[0x484] + 1;
}

void
store_cursor( word cursor )
{
	vga_write_register(0x0E, cursor >> 8 );
	vga_write_register(0x0F, cursor & 0xFF );

	mem_space[0x450] = cursor / columns();
	mem_space[0x451] = cursor % columns();

	set_video_dirty();
}

void
load_cursor( byte *row, byte *column )
{
	word cursor = vga_read_register(0x0E) << 8 | vga_read_register(0x0F);

	*row = cursor / columns();
	*column = cursor % columns();

	mem_space[0x450] = cursor / columns();
	mem_space[0x451] = cursor % columns();
}

word
load_cursor_word()
{
	return vga_read_register(0x0E) << 8 | vga_read_register(0x0F);
}

void
video_display_combination()
{
	if( cpu.regs.B.AL == 0 )
	{
		cpu.regs.B.BL = 0x08;
		cpu.regs.B.BH = 0x00;
		cpu.regs.B.AL = 0x1A;
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
	if( cpu.regs.B.AL == 0x30 )
	{
		if( cpu.regs.B.BH == 0 )
		{
			cpu.regs.W.BP = (mem_space[0x1F * 4 + 1] << 8) | mem_space[0x1F * 4];
			cpu.ES = (mem_space[0x1F * 4 + 3] << 8) | mem_space[0x1F * 4 + 2];
			vlog( VM_VIDEOMSG, "Character generator returned pointer to Interrupt 1F" );
			return;
		}
	}

	vlog( VM_VIDEOMSG, "Unknown character generator request: AL=%02X, BH=%02X\n", cpu.regs.B.AL, cpu.regs.B.BH );
}

void
set_dac_color_register()
{
	if( cpu.regs.B.AL == 0x00 )
	{
		vlog( VM_VIDEOMSG, "Set palette register %u to color %u", cpu.regs.B.BL, cpu.regs.B.BH );
		vga_palette_register[cpu.regs.B.BL] = cpu.regs.B.BH;
	}
	if( cpu.regs.B.AL == 0x10 )
	{
		vlog( VM_VIDEOMSG, "Set color %d to #%02x%02x%02x", cpu.regs.W.BX, cpu.regs.B.DH<<2, cpu.regs.B.CH<<2, cpu.regs.B.CL<<2 );

		vga_color_register[cpu.regs.W.BX & 0xFF].r = cpu.regs.B.DH;
		vga_color_register[cpu.regs.W.BX & 0xFF].g = cpu.regs.B.CH;
		vga_color_register[cpu.regs.W.BX & 0xFF].b = cpu.regs.B.CL;

	}
	else if( cpu.regs.B.AL == 0x02 )
	{
		vlog( VM_VIDEOMSG, "Loading palette from %04X:%04X", cpu.ES, cpu.regs.W.DX );
		for( int i = 0; i < 17; ++i )
		{
			vga_palette_register[i] = mem_getbyte( cpu.ES, cpu.regs.W.DX + i );
			//vlog( VM_VIDEOMSG, "Palette(%u): %02X", i, vga_palette_register[i] );
		}
	}
	else
	{
		vlog( VM_VIDEOMSG, "Unsupported palette operation %02X", cpu.regs.B.AL );
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

	if( mem_space[0x449] != 0x12 )
	{
		vlog( VM_ALERT, "Poking pixels in mode %02X, dunno how to handle that\n", mem_space[0x449] );
		vm_kill( 1 );
	}

	//vlog( VM_ALERT, "y = %03u, x = %03u, color = %02X\n", cpu.regs.W.DX, cpu.regs.W.CX, cpu.regs.B.AL );

	extern byte vm_p0[];
	extern byte vm_p1[];
	extern byte vm_p2[];
	extern byte vm_p3[];

	word offset = (cpu.regs.W.DX * 80) + (cpu.regs.W.CX / 8);
	byte bit = cpu.regs.W.CX % 8;

	vm_p0[offset] &= ~(0x80 >> bit);
	vm_p1[offset] &= ~(0x80 >> bit);
	vm_p2[offset] &= ~(0x80 >> bit);
	vm_p3[offset] &= ~(0x80 >> bit);

	if( cpu.regs.B.AL & 0x01 )
		vm_p0[offset] |= (0x80 >> bit);

	if( cpu.regs.B.AL & 0x02 )
		vm_p1[offset] |= (0x80 >> bit);

	if( cpu.regs.B.AL & 0x04 )
		vm_p2[offset] |= (0x80 >> bit);

	if( cpu.regs.B.AL & 0x08 )
		vm_p3[offset] |= (0x80 >> bit);

	set_video_dirty();
}
