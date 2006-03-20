#ifndef __VOMIT_H__
#define __VOMIT_H__

#include <stdbool.h>

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

	typedef unsigned char byte;
	typedef unsigned short int word;
	typedef unsigned long int dword;
	typedef signed char sigbyte;
	typedef signed short int sigword;
	typedef signed long int sigdword;

	typedef void (*tfunctab) ();
	typedef void *(*tvptrfunctab) ();
	typedef word (*tintab) (word, byte);
	typedef void (*touttab) (word, word, byte);
	extern tfunctab cpu_optable[0x100];
	extern char *cpu_opmnemonic[0x100];
	extern byte cpu_opgen[0x100];

	#define DISKACTION_NONE 0
	#define DISKACTION_READ 1
	#define DISKACTION_WRITE 2
	#define DISKACTION_VERIFY 3

	typedef struct {
		byte type;
		byte drive;
		word cylinder;
		byte head;
		word sector;
		word count;
	} diskaction_t;

	diskaction_t g_last_diskaction;

	void vm_listen(word, word (*) (word, byte), void (*) (word, word, byte));
	word vm_ioh_nin(word, byte);
	void vm_ioh_nout(word, word, byte);

	void vm_exit(int);
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

	void dump_cpu();
	void dump_all();
	void dump_try();
	void dump_bin();
	void dump_ivt();
	void dump_mem(word,word,byte);

	extern bool verbose, iplog, disklog, debug, trapint, rmpeek, iopeek, mempeek, callpeek;

	extern bool g_try_run;
	extern bool g_debug_step;
	extern bool g_break_pressed;

	#ifdef VM_DEBUG
		extern word BCS, BIP;
		void dump_ops();
	#endif

	#include "8086.h"
	#include "186.h"
	#include "floppy.h"
	#include "vga.h"

#endif

