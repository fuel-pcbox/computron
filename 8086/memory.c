/* 8086/memory.c
 * Memory Functions
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include "vomit.h"

byte *mem_space;
word mem_avail = 640;

void
mem_init() {
    mem_space = malloc(1048576*sizeof(byte));
    if(mem_space==NULL) vm_out("mem: Insufficient memory available.\n", VM_ERRORMSG);
}

void
mem_kill() {
    free(mem_space);
}

inline byte
mem_getbyte( word seg, word off ) {
	#ifdef VM_DEBUG
		if(mempeek)
			printf("mem_peek: %04X:%04X reading   BYTE at %08X\n", BCS, BIP, seg*16+off);
	#endif
	return mem_space[(seg<<4)+off];
}
inline word
mem_getword( word seg, word off ) {
	#ifdef VM_DEBUG
		if(mempeek)
			printf("mem_peek: %04X:%04X reading   WORD at %08X\n", BCS, BIP, seg*16+off);
	#endif
	#ifndef VM_EXPENDABLE
		if(off==0xffff)
			return mem_space[(seg<<4)+off] + (mem_space[seg<<4]<<8);
		else
	#endif
		return mem_space[(seg<<4)+off] + (mem_space[(seg<<4)+off+1]<<8);
}

inline void
mem_setbyte( word seg, word off, byte b ) {
	#ifdef VM_DEBUG
		if(mempeek)
			printf("mem_peek: %04X:%04X writing   BYTE at %08X\n", BCS, BIP, seg*16+off);
	#endif
	mem_space[(seg<<4)+off]=b;
}
inline void
mem_setword( word seg, word off, word w ) {
	#ifdef VM_DEBUG
		if(mempeek)
			printf("mem_peek: %04X:%04X writing   WORD at %08X\n", BCS, BIP, seg*16+off);
	#endif
	#ifndef VM_EXPENDABLE
		if(off==0xFFFF) {
			mem_space[(seg<<4)+off]=(byte)w; mem_space[seg<<4]=(byte)(w>>8);
		} else
	#endif
		mem_space[(seg<<4)+off]=(byte)w; mem_space[(seg<<4)+off+1]=(byte)(w>>8);
}

void
mem_push( word w ) {					/* inline me */
	StackPointer=StackPointer-2; mem_setword(SS,StackPointer,w);
}

word
mem_pop() {								/* me too */
	word w=mem_getword(SS,StackPointer); StackPointer+=2;
	return w;
}

void
_LDS_reg16_mem16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	*treg16[rmreg(rm)] = *p;
	DS = *(p+1);
	return;
}
void
_LES_reg16_mem16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	*treg16[rmreg(rm)] = *p;
	ES = *(p+1);
	return;
}

