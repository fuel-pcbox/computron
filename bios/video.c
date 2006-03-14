#include "vomit.h"
#include "8086.h"
#include "debug.h"
#include <string.h>

static byte s_current_mode = 3;
static byte s_rows = 25;
static byte s_columns = 80;

byte get_video_mode() { return s_current_mode; }

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
static void video_subsystem_configuration();

static void
store_cursor( word cursor )
{
	vga_reg[0x0E] = cursor >> 8;
	vga_reg[0x0F] = cursor & 0xFF;

	mem_setbyte( 0x0040, 0x0050, cursor / s_columns );
	mem_setbyte( 0x0040, 0x0051, cursor % s_columns );
}

static void
load_cursor( byte *row, byte *column )
{
	word cursor = vga_reg[0x0E] << 8 | vga_reg[0x0F];

	*row = cursor / s_columns;
	*column = cursor % s_columns;

	mem_setbyte( 0x0040, 0x0050, cursor / s_columns );
	mem_setbyte( 0x0040, 0x0051, cursor % s_columns );
}

static word
load_cursor_word()
{
	return vga_reg[0x0E] << 8 | vga_reg[0x0F];
}

void
bios_interrupt10()
{
	switch( *treg8[REG_AH] )
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
#if 0
		case 0x07: scroll_active_page_down(); break;
#endif
		case 0x08: read_character_and_attribute_at_cursor(); break;
		case 0x09: write_character_and_attribute_at_cursor(); break;
		case 0x0a: write_character_at_cursor(); break;
#if 0
		case 0x0b: /*set_color_palette();*/ break;
		case 0x0c: /*write_graphics_pixel_at_coordinate();*/ break;
		case 0x0d: /*read_graphics_pixel_at_coordinate();*/ break;
#endif
		case 0x0e: write_text_in_teletype_mode(); break;
		case 0x0f: get_video_state(); break;
		case 0x12: video_subsystem_configuration(); break;
		default:
			vlog( VM_VIDEOMSG, "Interrupt 10, function %02X requested", *treg8[REG_AH] );
	}
}

void
set_cursor_position()
{
	/* BH is page# */

	byte row = *treg8[REG_DH];
	byte column = *treg8[REG_DL];

	word cursor = row * s_columns + column;

	store_cursor( cursor );
}

void
get_cursor_position()
{
	/* Cursor row and column. */
	load_cursor( treg8[REG_DH], treg8[REG_DL] );

	/* XXX: Cursor scanlines are not actually implemented. */

	/* Starting (top) scanline. */
	*treg8[REG_CH] = mem_getbyte( 0x0040, 0x0061 );
	/* Ending (bottom) scanline. */
	*treg8[REG_CL] = mem_getbyte( 0x0040, 0x0060 );
}

void
write_character_at_cursor()
{
	/* BH is page# */
	/* BL is foreground color (ignored?) */

	byte row, column;
	word cursor;

	/* Check count of characters to write. */
	if( CX == 0 )
		return;

	load_cursor( &row, &column );
	cursor = row * s_columns + column;

	/* XXX: 0xB800 is hard-coded for now. */
	mem_setbyte( 0xB800, cursor << 1, *treg8[REG_AL] );
}

void
read_character_and_attribute_at_cursor()
{
	/* XXX: Works *only* with page 0. */
	/* BH is page# */

	word cursor = load_cursor_word();

	*treg8[REG_AH] = mem_getbyte( 0xB800, (cursor << 1) + 1 );
	*treg8[REG_AL] = mem_getbyte( 0xB800, cursor << 1 );
}

void
write_character_and_attribute_at_cursor()
{
	/* BH is page# */
	byte row, column;
	word cursor;

	/* Check count of characters to write. */
	if( CX == 0 )
		return;

	load_cursor( &row, &column );
	cursor = row * s_columns + column;

	/* XXX: 0xB800 is hard-coded for now. */
	mem_setbyte( 0xB800, cursor << 1, *treg8[REG_AL] );
	mem_setbyte( 0xB800, (cursor << 1) + 1, *treg8[REG_BL] );
}

void
write_text_in_teletype_mode()
{
	/* BH is page# */

	byte row, column;
	word cursor;

	byte ch = *treg8[REG_AL];
	byte attr = *treg8[REG_BL];

	load_cursor( &row, &column );
	cursor = row * s_columns + column;

	switch( ch )
	{
		case 0x0d:
			row++;
			cursor = row * s_columns + column;
			break;
		case 0x0a:
			column = 0;
			cursor = row * s_columns + column;
			break;
		case 0x08:
			cursor--;
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
	if( cursor >= (s_rows * s_columns) )
	{
		int i;
		memmove( mem_space + 0xB8000, mem_space + 0xB8000 + s_columns * 2, (s_columns * (s_rows - 1))*2 );
		for( i = 0; i < s_columns * 2; i += 2 )
		{
			mem_space[0xB8000 + 4000 - 160 + i] = 0x20;
			mem_space[0xB8000 + 4000 - 160 + i + 1] = attr;
		}
		cursor = s_columns * (s_rows - 1);
	}
	store_cursor( cursor );
}

void
scroll_active_page_up()
{
	vga_scrollup(*treg8[REG_CL], *treg8[REG_CH], *treg8[REG_DL], *treg8[REG_DH], *treg8[REG_AL], *treg8[REG_BH]);
}

void
set_video_mode()
{
	byte mode = *treg8[REG_AL];
	byte temp;

	s_current_mode = mode;

	mem_setbyte( 0x0040, 0x0049, mode );

	temp = mem_getbyte( 0x0040, 0x0087 );
	temp &= 0x7F;
	temp |= mode & 0x80;

	vlog( VM_VIDEOMSG, "Mode %d selected", mode );
}

void
get_video_state()
{
	/* Screen columns. */
	*treg8[REG_AH] = s_columns;

	/* Active page. */
	*treg8[REG_BH] = 0;

	/* Current video mode. */
	*treg8[REG_AL] = s_current_mode;
}

void
select_active_display_page()
{
	vlog( VM_VIDEOMSG, "Page %d selected (not handled!)", *treg8[REG_AL] );

	/* XXX: Note that nothing actually happens here ;-) */
}

void
video_subsystem_configuration()
{
	vlog( VM_VIDEOMSG, "INT 10,12 with BL=%02X", *treg8[REG_BL] );

	/* Report "color mode" in effect. */
	*treg8[REG_BH] = 0x00;

#if 0
	/* Report 256k of EGA memory. */
	*treg8[REG_BL] = 0x00;

	CX = 0;
#endif
}

void
set_cursor_type()
{
	vlog( VM_VIDEOMSG, "Cursor set: %u to %u%s", *treg8[REG_CH], *treg8[REG_CL], CX == 0x2000 ? " (disabled)" : "" );
}
