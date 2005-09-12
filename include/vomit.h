#ifndef __VOMIT_H__
#define __VOMIT_H__

#include <stdbool.h>
#include <stdio.h>

	#define MAX_FILESIZE	524288		/* 512kB is max "loadfile" size */
	#define MAX_FN_LENGTH	128

	#define VM_INITMSG      100
	#define VM_KILLMSG      101
	#define VM_ERRORMSG     102
	#define VM_EXITMSG		103
	#define VM_LOADMSG		104
	#define VM_DUMPMSG		105
	#define VM_HELPMSG		106
	#define VM_BREAKMSG		107
	#define VM_LOGMSG		108
	#define VM_IOMSG		109
	#define VM_UNIMP		110
	#define VM_OTHER		111
	#define VM_OUTPUT		112
	#define VM_DISKLOG		113
	#define VM_PRNLOG		114
	#define VM_VIDEOMSG		115

	typedef unsigned char byte;
	typedef unsigned short int word;
	typedef unsigned long int dword;
	typedef signed char sigbyte;
	typedef signed short int sigword;
	typedef signed long int sigdword;

	typedef void (*tfunctab) ();
	typedef void *(*tvptrfunctab) ();
	typedef word (*tintab) (byte);
	typedef void (*touttab) (word, byte);
	extern tfunctab cpu_optable[0x100];
	extern char *cpu_opmnemonic[0x100];
	extern byte cpu_opgen[0x100];

	void vm_listen(word, word (*) (byte), void (*) (word, byte));
	word vm_ioh_nin(byte);
	void vm_ioh_nout(word, byte);

	void vm_out(char *, int);
	void vm_exit(int);
	void vm_loadconf();
	void vm_cbreak(int);
	void vm_debug();
	void vm_call8(word, byte);
	void vm_call16(word, word);
	void vm_handle66(word);

	void vm_init();
	void vm_kill();

	void ui_init();
	void ui_kill();
	void ui_show();
	void ui_sync();
	void ui_statusbar();
	int kbd_hit();
	int kbd_getc();

	int set_tty_raw();
	int set_tty_cooked();
	int kbhit();

	void dump_cpu();
	void dump_all();
	void dump_bin();
	void dump_ivt();
	void dump_mem(word,word,byte);

	extern bool verbose, iplog, disklog, debug, trapint, rmpeek, iopeek, mempeek, callpeek;

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

