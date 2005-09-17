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

#define VGA_ATTR(a) COLOR_PAIR( (((a)>>4)&7)*8 |  ((a)&7))
#define PRINTABLE(c) ((c < 32) ? ' ' : c)

#ifdef VM_DEBUG
extern word g_last_nonbios_CS;
extern word g_last_nonbios_IP;
#endif

static WINDOW *s_statusbar;

static bool ui_visible = false;

static void __ui_init();

static void
ui_resize() {
	ui_kill();
	__ui_init();
	ui_sync();
}

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
		case VM_ALERT:
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

static int
__getch()
{
	int c = getch();
	if ( c == KEY_RESIZE ) {
		ui_resize();
		return ERR;
	}
	return c;
}

int
kbd_getc() {
	int c = __getch();
	return (c == ERR) ? 0 : c;
}

int
kbd_hit() {
	int c = __getch();
	if (c == ERR)
		return 0;
	ungetch(c);
	return c;
}

static void
__ui_init()
{
	int f, b;

	if ( !initscr() ) {
		vm_out( "initscr() failed, dying.", VM_KILLMSG );
		exit( 1 );
	}
	cbreak();
	noecho();
	nonl();
	nodelay( stdscr, true );
	start_color();
	for ( b = 0; b < 8; ++b ) {
		for ( f = 0; f < 8; ++f ) {
			init_pair( b * 8 + f, f, b );
		}
	}
	s_statusbar = newwin( 1, COLS, LINES - 1, 0 );
	wbkgd( s_statusbar, COLOR_PAIR( 56 ) );
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
	char *p, buf[80];
	static int break_timeout = 0;
	wmove( s_statusbar, 0, 0 );
	p = buf;
	p += sprintf( p, "[80%s86] ", cpu_type == 1 ? "1" : "" );
#ifdef VM_DEBUG
	p += sprintf( p, "%04X:%04X", g_last_nonbios_CS, g_last_nonbios_IP );
#endif
	waddstr( s_statusbar, buf );
	if ( g_break_pressed ) {
		break_timeout = 3;
	}
	if ( break_timeout ) {
		waddstr( s_statusbar, " [BREAK]" );
		--break_timeout;
	}
	wnoutrefresh( s_statusbar );
}

void
ui_statusbar() {
	__ui_statusbar();
	wrefresh( s_statusbar );
	doupdate();
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
	refresh();
	wbkgd( s_statusbar, COLOR_PAIR( 56 ) );
	move(ny, nx);
	wrefresh( s_statusbar );
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
	endwin();
	ui_visible = false;
}

