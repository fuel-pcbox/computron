/* ui.c
 * User interface functions
 *
 * This is a mess.
 */

#include "vomit.h"

#ifdef VOMIT_CURSES

#include <string.h>
#include <stdlib.h>
#include <curses.h>

#define VGA_ATTR(a) COLOR_PAIR( (((a)>>4)&7)*8 |  ((a)&7)) | ( ((a)&8) ? A_BOLD : 0 )
#define PRINTABLE(c) ((c < 32 || c > 250) ? ' ' : c)

#ifdef VM_DEBUG
extern word g_last_nonbios_CS;
extern word g_last_nonbios_IP;
#endif

static WINDOW *s_screen;
static bool ui_visible;

static byte *video_memory = 0L;
static byte last_video[4000];

typedef union {
	struct { byte c; byte a; } b;
	word w;
} ca;


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
	if( c == KEY_RESIZE )
	{
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

void
ui_init()
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

	s_screen = newwin( 27, 82, 0, 0 );
	ui_visible = true;

	werase( stdscr );
	wnoutrefresh( stdscr );

	video_memory = mem_space + 0xB8000;
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
	s_surface = SDL_SetVideoMode( 640, 480, 32, SDL_SWSURFACE | SDL_ANYFORMAT );
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

#ifdef poo
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
#endif

dword
to_rgb( byte color )
{
	switch( color )
	{
		case  0: return 0;
		case  1: return 0x000080;
		case  2: return 0x008000;
		case  3: return 0x008080;
		case  4: return 0x800000;
		case  5: return 0x800080;
		case  6: return 0x808000;
		case  7: return 0x808080;

		case  8: return 0xa0a0a0;
		case  9: return 0x0000ff;
		case 10: return 0x00ffff;
		case 11: return 0xff0000;
		case 12: return 0xff00ff;
		case 13: return 0xffff00;
		case 14: return 0xffffff;
	}
	return 0xffffff;
}

#endif /* VOMIT_SDL */

void
ui_sync() {
	int x, y, off, nx = 0, ny = 0;
	byte mode = get_video_mode();
	ca *vcur, *vlast;

	/* TODO: Move this, I guess. */
	if ( !ui_visible )
		return;

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
		vcur = (ca *)video_memory;
		vlast = (ca *)last_video;
		for (y=0; y<25; y++)
			for (x=0; x<80; x++) {
				if( vcur->w != vlast->w )
				{
					mvwaddch( s_screen, y+1, x+1, PRINTABLE(vcur->b.c) | VGA_ATTR(vcur->b.a) );
					vlast->w = vcur->w;
				}
				vcur ++;
				vlast ++;
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

		for( y = 0; y < 200; y += 2 )
		{
			for( x = 0; x < 320; x += 4 )
			{
				word offset = (y*40) + (x>>2);

				byte data = video_memory[offset];

				byte p4 = (data) & 3;
				byte p3 = (data >> 2) & 3;
				byte p2 = (data >> 4) & 3;
				byte p1 = (data >> 6) & 3;

				putpixel( x, y, to_rgb( p1 ));
				putpixel( x+1, y, to_rgb( p2 ));
				putpixel( x+2, y, to_rgb( p3 ));
				putpixel( x+3, y, to_rgb( p4 ));

				data = video_memory[0x2000 + offset];
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
	else if( mode == 0x12 )
	{
		if( !s_surface )
		{
			sdl_init();
		}

		if( SDL_MUSTLOCK( s_surface ))
			SDL_LockSurface( s_surface );

		extern byte vm_p0[];
		extern byte vm_p1[];
		extern byte vm_p2[];
		extern byte vm_p3[];

		word offset = 0;
		for( y = 0; y < 480; y ++ )
		{
			for( x = 0; x < 640; x += 8, ++offset )
			{
				byte data[4];
				data[0] = vm_p0[offset];
				data[1] = vm_p1[offset];
				data[2] = vm_p2[offset];
				data[3] = vm_p3[offset];

#define D(i) ((data[0]>>i) & 1) | (((data[1]>>i) & 1)<<1) | (((data[2]>>i) & 1)<<2) | (((data[3]>>i) & 1)<<3)

				byte p1 = D(0);
				byte p2 = D(1);
				byte p3 = D(2);
				byte p4 = D(3);
				byte p5 = D(4);
				byte p6 = D(5);
				byte p7 = D(6);
				byte p8 = D(7);

				putpixel( x+7, y, to_rgb( p1 ));
				putpixel( x+6, y, to_rgb( p2 ));
				putpixel( x+5, y, to_rgb( p3 ));
				putpixel( x+4, y, to_rgb( p4 ));
				putpixel( x+3, y, to_rgb( p5 ));
				putpixel( x+2, y, to_rgb( p6 ));
				putpixel( x+1, y, to_rgb( p7 ));
				putpixel( x+0, y, to_rgb( p8 ));

			}
		}

		if( SDL_MUSTLOCK( s_surface ))
			SDL_UnlockSurface( s_surface );

		SDL_Flip( s_surface );
	}
#endif /* VOMIT_SDL */
	else
	{
		mvwaddstr( s_screen, 1, 1, "Crazy video mode active..." );
	}

	box( s_screen, ACS_VLINE, ACS_HLINE );

	wmove( s_screen, ny + 1, nx + 1);
	wrefresh( s_screen );
}

void
ui_show()
{
	if ( ui_visible )
		return;
	ui_init();
	ui_sync();
}

void
ui_kill()
{
	delwin( s_screen );
	endwin();
	ui_visible = false;
}

#else

void ui_init() {}
void ui_show() {}
void ui_kill() {}
void ui_sync() {}

#endif
