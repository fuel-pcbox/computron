#ifndef __vomit_h__
#define __vomit_h__

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#define MAX_FILESIZE	524288		/* 512kB is max "loadfile" size */
#define MAX_FN_LENGTH	128

#define VM_INITMSG      100
#define VM_KILLMSG      101
#define VM_ERRORMSG     102
#define VM_EXITMSG		103
#define VM_LOADMSG		104
#define VM_FPUMSG       105
#define VM_CPUMSG       106
#define VM_BREAKMSG		107
#define VM_LOGMSG		108
#define VM_IOMSG		109
#define VM_ALERT		110
#define VM_OTHER		111
#define VM_OUTPUT		112
#define VM_DISKLOG		113
#define VM_PRNLOG		114
#define VM_VIDEOMSG		115
#define VM_KEYMSG		116
#define VM_CONFIGMSG	117
#define VM_MEMORYMSG    118
#define VM_DMAMSG       119
#define VM_FDCMSG       120
#define VM_DUMPMSG      121
#define VM_MOUSEMSG     122
#define VM_DOSMSG       123
#define VM_PICMSG       124

typedef uint8_t byte;
typedef uint16_t word;
typedef uint32_t dword;
typedef int8_t sigbyte;
typedef int16_t sigword;
typedef int32_t sigdword;

typedef void (*tfunctab) ();
typedef void *(*tvptrfunctab) ();
extern tfunctab cpu_optable[0x100];

void vm_listen(word, byte (*)(word), void (*)(word, byte));
byte vm_ioh_nin( word );
void vm_ioh_nout( word, byte );

void vm_exit(int);
void config_reload();
void vm_loadconf();
void vm_cbreak(int);
void vm_debug();
void vm_call8(word, byte);
void vm_call16(word, word);
void vm_handleE6(word);

void vm_init();
void vm_kill();

void ui_init();
void ui_kill();
void ui_show();
void ui_sync();
word kbd_hit();
word kbd_getc();
byte kbd_pop_raw();
int get_current_x();
int get_current_y();
void busmouse_event();
void busmouse_press( int button );
void busmouse_release( int button );

void dump_cpu();
void dump_all();
void dump_try();
void dump_ivt();
void dump_mem(word,word,byte);

void dma_init();
void fdc_init();

void video_bios_init();
void load_cursor( byte *row, byte *column );

int vomit_init( int, char ** );

void irq( byte num );
void pic_service_irq();

extern bool disklog, trapint, iopeek, mempeek, callpeek;

extern bool g_try_run;
extern bool g_debug_step;
extern bool g_break_pressed;

extern void vomit_set_vlog_handler( void (*f)(int, const char *, va_list) );

typedef struct {
	bool bda_peek;
	bool trace;
} vomit_options_t;

extern vomit_options_t options;

#include "8086.h"
#include "186.h"
#include "floppy.h"
#include "vga.h"

#endif /* __vomit_h__ */
