/* ui.c
 * User interface functions
 *
 * This is a mess.
 */

#include "vomit.h"

#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include <signal.h>

#define VGA_ATTR(a) COLOR_PAIR( (((a)>>4)&7)*8 |  ((a)&7)) | ( ((a)&8) ? A_BOLD : 0 )
#define PRINTABLE(c) ((c < 32 || c > 250) ? ' ' : c)

static void ui_output_add( char * );

#ifdef VM_DEBUG
extern word g_last_nonbios_CS;
extern word g_last_nonbios_IP;
#endif

static bool s_show_registers = false;

#define notype   0
#define bytetype 1
#define wordtype 2
#define booltype 3

typedef struct { const char *s; bool *b; } sboolpair_t;
typedef struct { const char *s; void *v; int type; } sanypair_t;

sboolpair_t s_showables[] = {
	{ "regs", &s_show_registers },
	{ 0L,     0L }
};

sanypair_t s_settables[] = {
	{ "cpu_type",  &cpu_type, bytetype },
	{ "ax",        &AX,       wordtype },
	{ "bx",        &BX,       wordtype },
	{ "cx",        &CX,       wordtype },
	{ "dx",        &DX,       wordtype },
	{ "si",        &SI,       wordtype },
	{ "di",        &DI,       wordtype },
	{ "bp",        &BasePointer, wordtype },
	{ "sp",        &StackPointer, wordtype },
	{ "cs",        &CS,       wordtype },
	{ "ds",        &DS,       wordtype },
	{ "es",        &ES,       wordtype },
	{ "ss",        &SS,       wordtype },
	{ "currentsegment", &CurrentSegment, wordtype },
	{ "cf",        &CF,       booltype },
	{ "if",        &IF,       booltype },
	{ "of",        &OF,       booltype },
	{ "zf",        &ZF,       booltype },
	{ "df",        &DF,       booltype },
	{ "af",        &AF,       booltype },
	{ "tf",        &TF,       booltype },
	{ "sf",        &SF,       booltype },
	{ "pf",        &PF,       booltype },
	{ 0L,          0L,        notype }
};

static WINDOW *s_screen;
static WINDOW *s_diskaction;
static WINDOW *s_outputwin;
static WINDOW *s_simulation;
static WINDOW *s_statusbar;
static WINDOW *s_registers;

bool g_command_mode = false;

static char s_command[128] = "";
static char s_output[6][256] = { "", "", "", "", "", "" };

static bool ui_visible = false;

static void __ui_init();

static void
ui_resize() {
	ui_kill();
	__ui_init();
	ui_sync();
}

static word
to_scancode( int c )
{
	switch( c )
	{
		case KEY_BACKSPACE: return 0x0E08;
		case KEY_UP: return 0x4800;
		case KEY_DOWN: return 0x5000;
		case KEY_LEFT: return 0x4B00;
		case KEY_RIGHT: return 0x4D00;
		case KEY_HOME: return 0x4700;
		case KEY_PPAGE: return 0x4900;
		case KEY_NPAGE: return 0x5100;
		case KEY_F(1): return 0x3B00;
		case KEY_F(2): return 0x3C00;
		case KEY_F(3): return 0x3D00;
		case KEY_F(4): return 0x3E00;
		case KEY_F(5): return 0x3F00;
		case KEY_F(6): return 0x4000;
		case KEY_F(7): return 0x4100;
		case KEY_F(8): return 0x4200;
		case KEY_F(9): return 0x4300;
		case KEY_F(10): return 0x4400;
		case KEY_F(11): return 0x4500;
		case KEY_F(12): return 0x4600;
	}
	return c;
}

static int
__getch()
{
	int c = getch();
	if( c == KEY_RESIZE ) {
		ui_resize();
		return ERR;
	}
	return c;
}

word
kbd_getc() {
	int c = __getch();
	return c == ERR ? 0 : to_scancode( c );
}

word
kbd_hit() {
	int c = __getch();
	if( c == ERR )
		return 0;
	ungetch(c);
	return to_scancode( c );
}

static void
__ui_init()
{
	int f, b;
	int doscolors[8] = { COLOR_BLACK, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN, COLOR_RED, COLOR_MAGENTA, COLOR_YELLOW, COLOR_WHITE };

	if( !initscr() )
	{
		vlog( VM_KILLMSG, "initscr() failed, dying." );
		exit( 1 );
	}
	cbreak();
	noecho();
	nonl();
	keypad( stdscr, true );
	nodelay( stdscr, true );
	start_color();
	for ( b = 0; b < 8; ++b ) {
		for ( f = 0; f < 8; ++f ) {
			init_pair( b * 8 + f, doscolors[f], doscolors[b] );
		}
	}

	bkgd( ' ' | COLOR_PAIR( 10 ));

	s_simulation = newwin( 6, 24, 0, COLS - 24 );
	s_diskaction = newwin( 10, 24, 6, COLS - 24 );
	s_outputwin = newwin( 6, COLS, LINES - 7, 0 );

	s_screen = newwin( 27, 82, 0, 0 );

	s_statusbar = newwin( 1, COLS, LINES - 1, 0 );
	wbkgd( s_statusbar, ' ' | COLOR_PAIR( 56 ));
	ui_visible = true;
}

void
ui_init() {
#ifdef VM_DEBUG
	FILE *fplog = fopen("log.txt", "w");	/* TRUNCATE FUCKLOG */
	fclose(fplog);
#endif
	__ui_init();
}

static void
__ui_statusbar() {
	char buf[256];
	int i;

	if( g_command_mode )
	{
		snprintf( buf, sizeof(buf), ">%s", s_command );
		mvwaddstr( s_statusbar, 0, 0, buf );
	}
	else
	{
		mvwaddstr( s_statusbar, 0, 0, "^C to enter command mode" );
	}

	wclrtoeol( s_statusbar );
	wnoutrefresh( s_statusbar );

	box( s_simulation, ACS_VLINE, ACS_HLINE );
	mvwaddstr( s_simulation, 0, 1, "Simulation" );
	mvwaddstr( s_simulation, 2, 2, "Type:  " );
	waddstr( s_simulation, cpu_type == 1 ? "80186" : "8086 " );
	mvwaddstr( s_simulation, 3, 2, "State: " );
	waddstr( s_simulation, cpu_state == CPU_ALIVE ? "alive" : "halted" );
	wnoutrefresh( s_simulation );

	box( s_diskaction, ACS_VLINE, ACS_HLINE );
	mvwaddstr( s_diskaction, 0, 1, "Disk activity" );
	if( g_last_diskaction.type != DISKACTION_NONE )
	{
		byte d = g_last_diskaction.drive;
		byte t = g_last_diskaction.type;
		mvwaddstr( s_diskaction, 2, 2, "Drive:    " );
		waddch( s_diskaction, d & 0x80 ? 'C' + (d & 1) : 'A' + (d & 1));
		mvwaddstr( s_diskaction, 3, 2, "Cylinder: " );
		sprintf( buf, "%-5u", g_last_diskaction.cylinder );
		waddstr( s_diskaction, buf );
		mvwaddstr( s_diskaction, 4, 2, "Head:     " );
		sprintf( buf, "%-3u", g_last_diskaction.head );
		waddstr( s_diskaction, buf );
		mvwaddstr( s_diskaction, 5, 2, "Sector:   " );
		sprintf( buf, "%-5u", g_last_diskaction.sector );
		waddstr( s_diskaction, buf );
		mvwaddstr( s_diskaction, 7, 2, t == DISKACTION_READ ? "READ  " : t == DISKACTION_WRITE ? "WRITE " : "VERIFY" );
	}
	wnoutrefresh( s_diskaction );

	if( !s_show_registers )
	{
		if( s_registers )
			delwin( s_registers );
		s_registers = 0L;
	}
	else
	{
		if( !s_registers )
		{
			s_registers = newwin( 18, 24, 16, COLS - 24 );
		}
		box( s_registers, ACS_VLINE, ACS_HLINE );
		mvwaddstr( s_registers, 0, 1, "Registers" );
		i = 2;
		sprintf( buf, "AX=%04X", AX );
		mvwaddstr( s_registers, i++, 2, buf );
		sprintf( buf, "BX=%04X", BX );
		mvwaddstr( s_registers, i++, 2, buf );
		sprintf( buf, "CX=%04X", CX );
		mvwaddstr( s_registers, i++, 2, buf );
		sprintf( buf, "DX=%04X", DX );
		mvwaddstr( s_registers, i++, 2, buf );

		sprintf( buf, "SI=%04X", SI );
		mvwaddstr( s_registers, i++, 2, buf );
		sprintf( buf, "DI=%04X", DI );
		mvwaddstr( s_registers, i++, 2, buf );
		sprintf( buf, "BP=%04X", BasePointer );
		mvwaddstr( s_registers, i++, 2, buf );
		sprintf( buf, "SP=%04X", StackPointer );
		mvwaddstr( s_registers, i++, 2, buf );

		sprintf( buf, "DS=%04X", DS );
		mvwaddstr( s_registers, i++, 2, buf );
		sprintf( buf, "ES=%04X", ES );
		mvwaddstr( s_registers, i++, 2, buf );
		sprintf( buf, "SS=%04X", SS );
		mvwaddstr( s_registers, i++, 2, buf );

		i++;

		sprintf( buf, "CS=%04X", CS );
		mvwaddstr( s_registers, i++, 2, buf );
		sprintf( buf, "IP=%04X", IP );
		mvwaddstr( s_registers, i++, 2, buf );

		wnoutrefresh( s_registers );
	}

	for( i = 0; i < 6; ++i )
	{
		mvwaddstr( s_outputwin, i, 0, s_output[i] );
		wclrtoeol( s_outputwin );
	}
	wnoutrefresh( s_outputwin );
}

static void
ui_output_add( char *str )
{
	int i;
	for( i = 0; i < 5; ++i )
	{
		strcpy( s_output[i], s_output[i+1] );
	}
	strcpy( s_output[5], str );
}

static void
ui_tokenize_command( char **list, int *size )
{
	int max = *size;
	char *tok = s_command;
	*size = 0;
	while( *size < max )
	{
		char *p = strchr( tok, ' ' );
		if( !p )
		{
			list[*size] = tok;
			*size += 1;
			return;
		}
		*p = '\0';
		list[*size] = tok;
		*size += 1;
		tok = p + 1;
	}
}

static void
ui_exec_command()
{
	char buf[256];
	char *list[16];
	int size = 16;

	sprintf( buf, ">%s", s_command );
	ui_output_add( buf );
	ui_tokenize_command( list, &size );

	if( !strcasecmp( list[0], "show" ))
	{
		sboolpair_t *p;
		if( size > 1 )
			for( p = s_showables; p->b; ++p )
				if( !strcasecmp( p->s, list[1] ))
					*(p->b) = true;
	}
	else if( !strcasecmp( list[0], "hide" ))
	{
		sboolpair_t *p;
		if( size > 1 )
			for( p = s_showables; p->b; ++p )
				if( !strcasecmp( p->s, list[1] ))
					*(p->b) = false;
	}
	else if( !strcasecmp( list[0], "set" ) || !strcasecmp( list[0], "print" ))
	{
		sanypair_t *p;
		if( !strcasecmp( list[0], "print" ))
			size = 2;
		for( p = s_settables; p->v; ++p )
			if( !strcasecmp( p->s, list[1] ))
			{
				switch( p->type )
				{
					case bytetype:
						{
							byte *ptr = p->v;
							if( size > 2 )
								*ptr = (byte)strtol( list[2], 0L, 16 );
							else
							{
								sprintf( buf, "%s = %02X", list[1], *ptr );
								ui_output_add( buf );
							}
						}
						break;
					case wordtype:
						{
							word *ptr = p->v;
							if( size > 2 )
								*ptr = (word)strtol( list[2], 0L, 16 );
							else
							{
								sprintf( buf, "%s = %04X", list[1], *ptr );
								ui_output_add( buf );
							}
						}
						break;
					case booltype:
						{
							bool *ptr = p->v;
							if( size > 2 )
								*ptr = !strcasecmp( list[2], "true" );
							else
							{
								sprintf( buf, "%s = %s", list[1], *ptr ? "true" : "false" );
								ui_output_add( buf );
							}
						}
						break;
				}
			}
	}
	else if( !strcasecmp( list[0], "swapfd" ))
	{
		byte tmp_status = drv_status[0];
		byte tmp_type = drv_type[0];
		word tmp_spt = drv_spt[0];
		word tmp_heads = drv_heads[0];
		word tmp_sectors = drv_sectors[0];
		word tmp_sectsize = drv_sectsize[0];

		drv_status[0] = drv_status[1];
		drv_type[0] = drv_type[1];
		drv_spt[0] = drv_spt[1];
		drv_heads[0] = drv_heads[1];
		drv_sectors[0] = drv_sectors[1];
		drv_sectsize[0] = drv_sectors[1];

		char tmp_imgfile[MAX_FN_LENGTH];
		strcpy( tmp_imgfile, drv_imgfile[0] );

		strcpy( drv_imgfile[0], drv_imgfile[1] );
	}
	else if( !strcasecmp( list[0], "reboot" ))
	{
		cpu_jump( 0xF000, 0xFFF0 );
	}
	else if( !strcasecmp( list[0], "break" ))
	{
		int_call( 9 );
	}
	else if( !strcasecmp( list[0], "quit" ))
	{
		vm_exit( 0 );
	}
	else if( list[0][0] != '\0' )
	{
		ui_output_add( "Unknown command." );
	}
}

void
ui_command_mode()
{
	int end;
	int ch = __getch();
	if( ch == ERR )
		return;

	if( ch == '\n' || ch == '\r' )
	{
		ui_exec_command();
		s_command[0] = 0;
	}
	else
	{
		end = strlen( s_command );
		if( ch == 0x08 || ch == 0x7f )
		{
			if( end )
				--end;
		}
		else if( ch == '' )
		{
			end = 0;
		}
		else
		{
			s_command[end++] = (char)ch;
		}
		s_command[end] = 0;
	}
}

void
ui_statusbar() {
	__ui_statusbar();
	doupdate();
}

#ifdef VOMIT_SDL
#include <SDL.h>

SDL_Surface *s_surface = 0L;

void
sdl_init()
{
	if( SDL_Init( SDL_INIT_VIDEO ) == - 1)
	{
		vm_exit( 1 );
	}
	s_surface = SDL_SetVideoMode( 320, 200, 32, SDL_SWSURFACE | SDL_ANYFORMAT );
	if( !s_surface )
	{
		vm_exit( 1 );
	}
	SDL_WM_SetCaption( "VOMIT", 0L );
}

void
sdl_kill()
{
	SDL_Quit();
	s_surface = 0L;
}

void
putpixel( int x, int y, dword rgb )
{
	const int bpp = 4;
	byte *p = (byte *)s_surface->pixels + y * s_surface->pitch + x * bpp;
	*(dword *)p = rgb;
}

dword
to_rgb( byte color )
{
	switch( color )
	{
		case 0: return 0;
		case 1: return 0xff00ff;
		case 2: return 0x00ffff;
	}
	return 0xffffff;
}
#endif /* VOMIT_SDL */

void
ui_sync() {
	int x, y, off, nx = 0, ny = 0;
	if ( !ui_visible )
		return;

	byte mode = get_video_mode();

	if( (mode&0x7F) == 0x03 )
	{
#ifdef VOMIT_SDL
		if( s_surface )
		{
			sdl_kill();
		}
#endif /* VOMIT_SDL */
		off = ((vga_read_register(0x0E) << 8) + vga_read_register(0x0F));
		ny = off / 80;
		nx = off - (ny * 80);
		for (y=0; y<25; y++)
			for (x=0; x<80; x++) {
				mvwaddch( s_screen, y+1, x+1, PRINTABLE(mem_getbyte(0xB800, (y*160)+(x<<1)))
				                            | VGA_ATTR(mem_getbyte(0xB800, (y*160)+(x<<1)+1)));
			}
	}
#ifdef VOMIT_SDL
	else if( mode == 0x4 )
	{
		if( !s_surface )
		{
			sdl_init();
		}

		if( SDL_MUSTLOCK( s_surface ))
			SDL_LockSurface( s_surface );

		byte *video = mem_space + 0xB8000;

		for( y = 0; y < 200; y += 2 )
		{
			for( x = 0; x < 320; x += 4 )
			{
				word offset = (y*40) + (x>>2);

				byte data = video[offset];

				byte p4 = (data) & 3;
				byte p3 = (data >> 2) & 3;
				byte p2 = (data >> 4) & 3;
				byte p1 = (data >> 6) & 3;

				putpixel( x, y, to_rgb( p1 ));
				putpixel( x+1, y, to_rgb( p2 ));
				putpixel( x+2, y, to_rgb( p3 ));
				putpixel( x+3, y, to_rgb( p4 ));

				data = video[0x2000 + offset];
				p4 = (data) & 3;
				p3 = (data >> 2) & 3;
				p2 = (data >> 4) & 3;
				p1 = (data >> 6) & 3;

				putpixel( x, y+1, to_rgb( p1 ));
				putpixel( x+1, y+1, to_rgb( p2 ));
				putpixel( x+2, y+1, to_rgb( p3 ));
				putpixel( x+3, y+1, to_rgb( p4 ));
			}
		}

		if( SDL_MUSTLOCK( s_surface ))
			SDL_UnlockSurface( s_surface );

		SDL_Flip( s_surface );
	}
#endif /* VOMIT_SDL */
	else
	{
		mvwaddstr( s_screen, 2, 2, "Crazy video mode active..." );
	}

	box( s_screen, ACS_VLINE, ACS_HLINE );
	mvwaddstr( s_screen, 0, 1, "Screen" );

	werase( stdscr );
	wnoutrefresh( stdscr );
	__ui_statusbar();
	if( !g_command_mode )
		wmove( s_screen, ny + 1, nx + 1);
	wnoutrefresh( s_screen );
	if( g_command_mode )
		wnoutrefresh( s_statusbar );
	doupdate();
}

void
ui_show()
{
	if ( ui_visible )
		return;
	ui_visible = true;
	ui_sync();
}

void
ui_kill()
{
	delwin( s_statusbar );
	if( s_registers )
	{
		delwin( s_registers );
		s_registers = 0L;
	}
	delwin( s_outputwin );
	delwin( s_simulation );
	delwin( s_screen );
	endwin();
	ui_visible = false;
}

