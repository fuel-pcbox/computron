/* dump.c
 * Assorted dump functions
 *
 */

#ifdef VM_DEBUG
	#include <stdio.h>
	#include "vomit.h"

void
dump_cpu()
{
	printf( "CPU is an Intel %s\n", cpu_type == 0 ? "8086" : "80186" );
	printf( "Memory size: %dK\n", mem_avail );
	printf( "Prefetch queue: " );
#ifndef VM_NOPFQ
	printf( "%d bytes\n", CPU_PFQ_SIZE );
#else
	printf( "off\n" );
#endif
	printf( "Trap Flag: " );
#ifdef VM_TRAPFLAG
	printf( "on\n" );
#else
	printf( "off\n" );
#endif
}

void
dump_try()
{
	printf("AX=%04X\nBX=%04X\nCX=%04X\nDX=%04X\n", AX, BX, CX, DX );
	printf("SP=%04X\nBP=%04X\nSI=%04X\nDI=%04X\n", StackPointer, BasePointer, SI, DI);
	printf("CS=%04X\nDS=%04X\nES=%04X\nSS=%04X\n", CS, DS, ES, SS);
	printf("CF=%x\nPF=%x\nAF=%x\nZF=%x\nSF=%x\nIF=%x\nDF=%x\nOF=%x\nTF=%x\n", CF, PF, AF, ZF, SF, IF, DF, OF, TF);
}

void dump_all() {
	word *stacky = (void *)mem_space + (SS*16)+StackPointer;
#ifndef VM_NOPFQ
	byte x = (byte)cpu_pfq_current;
	byte dpfq[6];
	byte i;

	for(i=0;i<CPU_PFQ_SIZE;i++) {
		dpfq[i] = cpu_pfq[x++];
		if(x==CPU_PFQ_SIZE) x=0;
	}
#endif

	printf("\nAX=%04X BX=%04X CX=%04X DX=%04X  -  CS:IP=%04X:%04X", AX, BX, CX, DX, CS, IP);
	printf("\nSP=%04X BP=%04X SI=%04X DI=%04X  -  CurrentSeg=%04X", StackPointer, BasePointer, SI, DI, *CurrentSegment);
	printf("\nCS=%04X DS=%04X ES=%04X SS=%04X", CS, DS, ES, SS);
#ifndef VM_NOPFQ
	printf("  -  [%02X %02X%02X%02X%02X%02X]\n", dpfq[0], dpfq[1], dpfq[2], dpfq[3], dpfq[4], dpfq[5]);
#else
	printf("\n");
#endif
	printf("\nC=%x P=%x A=%x Z=%x S=%x I=%x D=%x O=%x  -  T=%x DISASM", CF, PF, AF, ZF, SF, IF, DF, OF, TF);
	printf("\nStack: %04X %04X %04X %04X %04X\n", *stacky, *(stacky+1), *(stacky+2), *(stacky+3), *(stacky+4));
	stacky += 5;
	printf("       %04X %04X %04X %04X %04X\n",   *stacky, *(stacky+1), *(stacky+2), *(stacky+3), *(stacky+4));
	return;
}

byte n(byte b) {					/* Nice it up for printing.		*/
	if(b<0x20) b='.';				/* MS Debug style ;-)			*/
	if((b>127)&&(b<160)) b='.';		/* No control characters		*/
	return b;
}

void dump_mem(word seg, word off, byte rows) {
	int i; byte *p = mem_space + (seg*16) + off;
	if(rows==0) rows=5;
	for(i=0;i<rows;i++) {
		printf(
			"%04X:%04X   %02X %02X %02X %02X %02X %02X %02X %02X - %02X %02X %02X %02X %02X %02X %02X %02X   %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
			seg, (off+i*16),
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
			p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15],
			n(p[0]), n(p[1]), n(p[2]), n(p[3]), n(p[4]), n(p[5]), n(p[6]), n(p[7]),
			n(p[8]), n(p[9]), n(p[10]), n(p[11]), n(p[12]), n(p[13]), n(p[14]), n(p[15])
		);
		p+=16;
	}
}

word _iseg(byte isr) { return mem_getword(0x0000, (isr*4)+2); }
word _ioff(byte isr) { return mem_getword(0x0000, (isr*4)); }

void dump_ivt() {
	register int i;
	for(i=0;i<0xFF;i+=5) {
		printf(
			"%02X>  %04X:%04X\t%02X>  %04X:%04X\t%02X>  %04X:%04X\t%02X>  %04X:%04X\t%02X>  %04X:%04X\n",
			i, _iseg(i), _ioff(i),
			i+1, _iseg(i+1), _ioff(i+1),
			i+2, _iseg(i+2), _ioff(i+2),
			i+3, _iseg(i+3), _ioff(i+3),
			i+4, _iseg(i+4), _ioff(i+4));
	}
}

void w2b(word w, char *rs) {
	rs[17] = 0;
	rs[16] = '0' + ((w&1)>0);
	rs[15] = '0' + ((w&2)>0);
	rs[14] = '0' + ((w&4)>0);
	rs[13] = '0' + ((w&8)>0);
	rs[12] = '0' + ((w&16)>0);
	rs[11] = '0' + ((w&32)>0);
	rs[10] = '0' + ((w&64)>0);
	rs[9] = '0' + ((w&128)>0);
	rs[8] = '\'';
	rs[7] = '0' + ((w&256)>0);
	rs[6] = '0' + ((w&512)>0);
	rs[5] = '0' + ((w&1024)>0);
	rs[4] = '0' + ((w&2048)>0);
	rs[3] = '0' + ((w&4096)>0);
	rs[2] = '0' + ((w&8192)>0);
	rs[1] = '0' + ((w&16384)>0);
	rs[0] = '0' + ((w&32768)>0);
}

void dump_bin() {
	char sa[18]; char sb[18]; char sc[18]; char sd[18];
	w2b(AX, sa); w2b(BX, sb);
	w2b(CX, sc); w2b(DX, sd);
	printf("\nAX=%s BX=%s    CF=%1X\n", sa, sb, CF);
	printf("CX=%s DX=%s\n", sc, sd);
}

#endif
