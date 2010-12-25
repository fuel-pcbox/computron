#ifndef __vomit_h__
#define __vomit_h__

#include <qglobal.h>
#include <stdarg.h>

#include "types.h"

#define MAX_FILESIZE	524288		/* 512kB is max "loadfile" size */
#define MAX_FN_LENGTH	128

void vm_exit(int);
void config_reload();
void vm_loadconf();
void vm_cbreak(int);

void vm_kill();

WORD kbd_hit();
WORD kbd_getc();
BYTE kbd_pop_raw();

#ifdef VOMIT_DIRECT_SCREEN
void screen_direct_update(WORD);
#endif

void dma_init();
void fdc_init();
void ide_init();

#ifdef VOMIT_C_VGA_BIOS
void video_bios_init();
void load_cursor(BYTE* row, BYTE* column);
#endif

void vomit_init();

extern void vomit_set_vlog_handler( void (*f)(int, const char *, va_list) );

typedef struct {
    bool bda_peek;
    bool trace;
    bool disklog;
    bool trapint;
    bool iopeek;
    bool mempeek;
} vomit_options_t;

extern vomit_options_t options;

#include "vcpu.h"

void vomit_ignore_io_port(WORD port);
void vm_listen(WORD, BYTE (*)(VCpu*, WORD), void (*)(VCpu*, WORD, BYTE));
void vm_call8(VCpu*, WORD port, BYTE value);

bool vomit_in_vretrace();

#endif
