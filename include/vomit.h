#ifndef __vomit_h__
#define __vomit_h__

#include <stdbool.h>
#include <stdarg.h>

#include "types.h"

#define MAX_FILESIZE	524288		/* 512kB is max "loadfile" size */
#define MAX_FN_LENGTH	128

void vm_exit(int);
void config_reload();
void vm_loadconf();
void vm_cbreak(int);

void vm_kill();

word kbd_hit();
word kbd_getc();
byte kbd_pop_raw();
int get_current_x();
int get_current_y();

#ifdef VOMIT_DIRECT_SCREEN
void screen_direct_update( word );
#endif

void busmouse_event();
void busmouse_press( int button );
void busmouse_release( int button );

void dump_try();
void dump_ivt();
int dump_disasm( word, word );
void dump_mem(word,word,byte);

void dma_init();
void fdc_init();
void pic_init();
void pit_init();
void busmouse_init();
void keyboard_init();
void ide_init();
void gameport_init();

void video_bios_init();
void load_cursor( byte *row, byte *column );

void vomit_init();

void irq( byte num );
void keyboard_raise_irq_if_enabled();

extern bool disklog, trapint, iopeek, mempeek, callpeek;

extern bool g_try_run;
extern bool g_debug_step;

extern void vomit_set_vlog_handler( void (*f)(int, const char *, va_list) );

typedef struct {
	bool bda_peek;
	bool trace;
} vomit_options_t;

extern vomit_options_t options;

#include "vcpu.h"

void dump_all(VCpu*);
void pic_service_irq(VCpu*);

void vomit_ignore_io_port(WORD port);
void vm_listen(word, byte (*)(VCpu*, word), void (*)(VCpu*, word, byte));
void vm_call8(VCpu*, WORD port, BYTE value);
void dump_cpu(VCpu*);
void dump_regs(VCpu*);

template <typename T> inline void swap(T &a, T &b)
{
	T c(a);
	a = b;
	b = c;
}

#endif
