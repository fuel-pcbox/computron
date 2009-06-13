/* dump.c
 * Assorted dump functions
 *
 */

#include <stdio.h>
#include "vomit.h"
#include "debug.h"
#include "disasm.h"

void
dump_cpu()
{
	vlog( VM_DUMPMSG, "CPU is an Intel %s", cpu.type == 0 ? "8086" : "80186" );
	vlog( VM_DUMPMSG, "Memory size: %dK", mem_avail );
#ifndef VM_NOPFQ
	vlog( VM_DUMPMSG, "Prefetch queue: %d bytes", CPU_PFQ_SIZE );
#else
	vlog( VM_DUMPMSG, "Prefetch queue: off" );
#endif
}

void
dump_try()
{
	printf( "AX=%04X\nBX=%04X\nCX=%04X\nDX=%04X\n", cpu.regs.W.AX, cpu.regs.W.BX, cpu.regs.W.CX, cpu.regs.W.DX );
	printf( "SP=%04X\nBP=%04X\nSI=%04X\nDI=%04X\n", cpu.regs.W.SP, cpu.regs.W.BP, cpu.regs.W.SI, cpu.regs.W.DI );
	printf( "CS=%04X\nDS=%04X\nES=%04X\nSS=%04X\n", cpu.CS, cpu.DS, cpu.ES, cpu.SS );
	printf( "CF=%x\nPF=%x\nAF=%x\nZF=%x\nSF=%x\nIF=%x\nDF=%x\nOF=%x\nTF=%x\n", cpu.CF, cpu.PF, cpu.AF, cpu.ZF, cpu.SF, cpu.IF, cpu.DF, cpu.OF, cpu.TF );
}

int
dump_disasm( word segment, word offset )
{
	char disasm[64];
	int width, i;
	char buf[512];
	char *p = buf;
	byte *opcode;

	opcode = mem_space + (segment << 4) + offset;
	width = insn_width( opcode );
	disassemble( opcode, offset, disasm, sizeof(disasm) );

	p += sprintf( p, "%04X:%04X ", segment, offset );

	for( i = 0; i < (width ? width : 7); ++i )
	{
		p += sprintf( p, "%02X", opcode[i] );
	}
	for( i = 0; i < (14-((width?width:7)*2)); ++i )
	{
		p += sprintf( p, " " );
	}

	p += sprintf( p, " %s", disasm );

	vlog( VM_DUMPMSG, buf );

	/* Recurse if this is a prefix instruction. */
	if( *opcode == 0x26 || *opcode == 0x2E || *opcode == 0x36 || *opcode == 0x3E || *opcode == 0xF2 || *opcode == 0xF3 )
		width += dump_disasm( segment, offset + width );

	return width;
}

void
dump_regs()
{
	vlog( VM_DUMPMSG,
			"AX=%04X BX=%04X CX=%04X DX=%04X SP=%04X BP=%04X SI=%04X DI=%04X "
			"CS=%04X DS=%04X ES=%04X SS=%04X C=%u P=%u A=%u Z=%u S=%u I=%u D=%u O=%u",
			cpu.regs.W.AX, cpu.regs.W.BX, cpu.regs.W.CX, cpu.regs.W.DX,
			cpu.regs.W.SP, cpu.regs.W.BP, cpu.regs.W.SI, cpu.regs.W.DI,
			cpu.CS, cpu.DS, cpu.ES, cpu.SS,
			cpu.CF, cpu.PF, cpu.AF, cpu.ZF, cpu.SF, cpu.IF, cpu.DF, cpu.OF
		);
}

void dump_all() {
	word *stacky = (void *)mem_space + (cpu.SS<<4)+cpu.regs.W.SP;
	byte *csip = mem_space + (cpu.base_CS<<4) + cpu.base_IP;

#ifndef VM_NOPFQ
	byte x = (byte)cpu_pfq_current;
	byte dpfq[6];
	byte i;

	for(i=0;i<CPU_PFQ_SIZE;i++) {
		dpfq[i] = cpu_pfq[x++];
		if(x==CPU_PFQ_SIZE) x=0;
	}
#endif

	vlog( VM_DUMPMSG, "AX=%04X BX=%04X CX=%04X DX=%04X     SP=> %04X", cpu.regs.W.AX, cpu.regs.W.BX, cpu.regs.W.CX, cpu.regs.W.DX, *(stacky++) );
	vlog( VM_DUMPMSG, "SP=%04X BP=%04X SI=%04X DI=%04X          %04X", cpu.regs.W.SP, cpu.regs.W.BP, cpu.regs.W.SI, cpu.regs.W.DI, *(stacky++) );
	vlog( VM_DUMPMSG, "CS=%04X DS=%04X ES=%04X SS=%04X          %04X", cpu.CS, cpu.DS, cpu.ES, cpu.SS, *(stacky++) );
	vlog( VM_DUMPMSG, "C=%u P=%u A=%u Z=%u S=%u I=%u D=%u O=%u          %04X", cpu.CF, cpu.PF, cpu.AF, cpu.ZF, cpu.SF, cpu.IF, cpu.DF, cpu.OF, *(stacky++) );

	vlog( VM_DUMPMSG, "  -  (%02X %02X%02X%02X%02X%02X)", csip[0], csip[1], csip[2], csip[3], csip[4], csip[5] );
#ifndef VM_NOPFQ
	vlog( VM_DUMPMSG, "  -  [%02X %02X%02X%02X%02X%02X]", dpfq[0], dpfq[1], dpfq[2], dpfq[3], dpfq[4], dpfq[5] );
#endif

	vlog( VM_DUMPMSG, "\n" );
	dump_disasm( cpu.base_CS, cpu.base_IP );
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
		vlog( VM_DUMPMSG,
			"%04X:%04X   %02X %02X %02X %02X %02X %02X %02X %02X - %02X %02X %02X %02X %02X %02X %02X %02X   %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
			seg, (off+i*16),
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
			p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15],
			n(p[0]), n(p[1]), n(p[2]), n(p[3]), n(p[4]), n(p[5]), n(p[6]), n(p[7]),
			n(p[8]), n(p[9]), n(p[10]), n(p[11]), n(p[12]), n(p[13]), n(p[14]), n(p[15])
		);
		p+=16;
	}
}

static word iseg(byte isr) { return mem_getword(0x0000, (isr*4)+2); }
static word ioff(byte isr) { return mem_getword(0x0000, (isr*4)); }

void
dump_ivt()
{
	word i;

	/* XXX: For alignment reasons, we're skipping INT FF */
	for( i = 0; i < 0xFF; i += 4 )
	{
		vlog( VM_DUMPMSG,
			"%02X>  %04X:%04X\t%02X>  %04X:%04X\t%02X>  %04X:%04X\t%02X>  %04X:%04X",
			i, iseg(i), ioff(i),
			i+1, iseg(i+1), ioff(i+1),
			i+2, iseg(i+2), ioff(i+2),
			i+3, iseg(i+3), ioff(i+3)
			//,i+4, iseg(i+4), ioff(i+4)
		);
	}
}
