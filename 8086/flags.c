/* 8086/flags.c
 * Handler of the hell that is flags
 *
 * 031102:	Fixed that AddOF fucker.
 *			DOS 3 & 4 are now functioning :-)
 *
 */

#include "vomit.h"

byte parity_table[256];

void cpu_flags_init() {
	register int i;
	for(i=0;i<256;i++)
		parity_table[i]=cpu_parity(i);

	CF=0; DF=0; TF=0; PF=0;
	AF=0; ZF=0; SF=0; IF=0; OF=0;
    
	return;
}

void cpu_setZF(dword ans) {
	if(ans==0) ZF=1; else ZF=0;
}

void cpu_setSF(dword ans, byte bits) {
	if(bits==16)
		SF=(ans>>15)&1;
	else
		SF=(ans>>7)&1;
}

void cpu_setPF(dword ans) {
    PF=parity_table[ans&0xFF];
}

void cpu_setAF(dword ans, word src, word dest) {
	AF=((((ans)^((src)^(dest)))&0x10)>>4)&1;
}

byte cpu_parity(byte b) {
	byte y, z=0, x=1;
	for(y=0;y<8;y++) {
		if(b&x) z++;
		x=x<<1;
	}
	return !(z&1);
}

