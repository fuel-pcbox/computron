/* ui.c
 * User interface functions
 */

#include "vomit.h"

#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include <signal.h>

#define VGA_ATTR(a) COLOR_PAIR( (((a)>>4)&7)*8 |  ((a)&7))
#define PRINTABLE(c) ((c < 32) ? ' ' : c)

#ifdef VM_DEBUG
extern word g_last_nonbios_CS;
extern word g_last_nonbios_IP;
#endif

WINDOW *mywin;

static bool ui_visible = false;
static char * statusbar;

void
vm_out( char *msg_text, int msg_type ) {
	#ifndef VM_DEBUG
		(void) msg_text;
		(void) msg_type;
	#else
		FILE *fplog;
		fplog = fopen("log.txt", "a+");
		if (fplog == NULL)
			return;

		if (iplog && msg_type != 0)
			fprintf(fplog, "%04X:%04X ", BCS, BIP);

		switch(msg_type) {
		case VM_INITMSG:
			fputs("[  INIT]  ", fplog);
			break;
		case VM_DISKLOG:
			fputs("[  DISK]  ", fplog);
			break;
		case VM_KILLMSG:
			fputs("[  KILL]  ", fplog);
			break;
		case VM_IOMSG:
			fputs("[   I/O]  ", fplog);
			break;
		case VM_UNIMP:
			fputs("[ ALERT]  ", fplog);
			break;
		case VM_PRNLOG:
			fputs("[   LPT]  ", fplog);
			break;
		case VM_VIDEOMSG:
			fputs("[ VIDEO]  ", fplog);
			break;
		}

		fputs(msg_text, fplog);
		fclose(fplog);
	#endif
	return;
}

int
kbd_getc() {
	int c = getch();
	return (c == ERR) ? 0 : c;
}

int
kbd_hit() {
	int c = getch();
	if (c == ERR)
		return 0;
	ungetch(c);
	return c;
}

static void
handle_winch( int signal ) {
	(void) signal;
	endwin();
	statusbar = realloc( statusbar, COLS + 1);
	memset( statusbar, ' ', COLS );
	ui_sync();
}

void
ui_init() {
	int f, b;
#ifdef VM_DEBUG
	FILE *fplog = fopen("log.txt", "w");	/* TRUNCATE FUCKLOG */
	fclose(fplog);
#endif
	mywin = initscr();
	cbreak();
	noecho();
	nonl();
	nodelay(mywin, TRUE);
	start_color();
	for ( b = 0; b < 8; ++b ) {
		for ( f = 0; f < 8; ++f ) {
			init_pair( b * 8 + f, f, b );
		}
	}
	signal( SIGWINCH, handle_winch );
	statusbar = malloc( COLS + 1 );
	memset( statusbar, ' ', COLS );
	ui_visible = true;
}

static void
__ui_statusbar() {
	char *p, buf[64];
	static int break_timeout = 0;
	attron( COLOR_PAIR( 56 ) );
	mvaddstr( LINES - 1, 0, statusbar );
	move( LINES - 1, 0 );
	p = buf;
	p += sprintf( p, "[80%s86] ", cpu_type == 1 ? "1" : "" );
#ifdef VM_DEBUG
	p += sprintf( p, "%04X:%04X", g_last_nonbios_CS, g_last_nonbios_IP );
#endif
	addstr( buf );
	if ( g_break_pressed ) {
		break_timeout = 3;
	}
	if ( break_timeout ) {
		addstr( " [BREAK]" );
		--break_timeout;
	}
	attrset( 0 );
}

void
ui_statusbar() {
	int y, x;
	getyx( stdscr, y, x );
	__ui_statusbar();
	move( y, x );
	refresh();
}

void
ui_sync() {
	int x, y, off, nx, ny;
	if ( !ui_visible )
		return;
	off = ((vga_reg[0x0E] << 8) + vga_reg[0x0F]);
	ny = off / 80;
	nx = off - (ny * 80);
	erase();
	for (y=0; y<25; y++)
		for (x=0; x<80; x++) {
			mvaddch(y, x, PRINTABLE(mem_getbyte(0xB800, (y*160)+(x<<1)))
			             | VGA_ATTR(mem_getbyte(0xB800, (y*160)+(x<<1)+1)));
		}
	__ui_statusbar();
	move(ny, nx);
	refresh();
}

void
ui_show()
{
	if ( ui_visible )
		return;
	statusbar = malloc( COLS + 1 );
	memset( statusbar, ' ', COLS );
	refresh();
	ui_visible = true;
}

void
ui_kill()
{
	if ( !ui_visible )
		return;
	free( statusbar );
	endwin();
	ui_visible = false;
}

