/* 8086/modrm.c
 * ModR/M Address Extraction
 *
 */

#include <stdio.h>
#include <string.h>
#include "vomit.h"
#include "debug.h"

static word calcEA(byte);
static tvptrfunctab cpu_modrmtable[0x100];
static byte cpu_rmbits;

void
*cpu_rmptr (byte b, byte bits) {
	void *p;
	cpu_rmbits = bits;
	p = cpu_modrmtable[b] ();
	return p;
}

void *_rm00() {
	word segment, offset = cpu.regs.W.BX+cpu.regs.W.SI;
	segment = *cpu.CurrentSegment;
	return (void *)mem_space+(segment<<4)+offset;
}
void *_rm01() {
	word segment, offset = cpu.regs.W.BX+cpu.regs.W.DI;
	segment = *cpu.CurrentSegment;
	return (void *)mem_space+(segment<<4)+offset;
}
void *_rm02() {
	word segment, offset = cpu.regs.W.BP+cpu.regs.W.SI;
	if(cpu.CurrentSegment==&cpu.SegmentPrefix) segment = *cpu.CurrentSegment;
	else segment = cpu.SS;
	return (void *)mem_space+(segment<<4)+offset;
}
void *_rm03() {
	word segment, offset = cpu.regs.W.BP+cpu.regs.W.DI;
	if(cpu.CurrentSegment==&cpu.SegmentPrefix) segment = *cpu.CurrentSegment;
	else segment = cpu.SS;
	return (void *)mem_space+(segment<<4)+offset;
}
void *_rm04() {
	word segment, offset = cpu.regs.W.SI;
	segment = *cpu.CurrentSegment;
	return (void *)mem_space+(segment<<4)+offset;
}
void *_rm05() {
	word segment, offset = cpu.regs.W.DI;
	segment = *cpu.CurrentSegment;
	return (void *)mem_space+(segment<<4)+offset;
}
void *_rm06() {
	word segment, offset = cpu_pfq_getword();
	segment = *cpu.CurrentSegment;
	return (void *)mem_space+(segment<<4)+offset;
}
void *_rm07() {
	word segment, offset = cpu.regs.W.BX;
	segment = *cpu.CurrentSegment;
	return (void *)mem_space+(segment<<4)+offset;
}
void *_rm08() {
    word segment, offset = cpu.regs.W.BX+cpu.regs.W.SI+signext(cpu_pfq_getbyte());
    segment = *cpu.CurrentSegment;
    return (void *)mem_space+(segment<<4)+offset;
}
void *_rm09() {
    word segment, offset = cpu.regs.W.BX+cpu.regs.W.DI+signext(cpu_pfq_getbyte());
    segment = *cpu.CurrentSegment;
    return (void *)mem_space+(segment<<4)+offset;
}
void *_rm10() {
    word segment, offset = cpu.regs.W.BP+cpu.regs.W.SI+signext(cpu_pfq_getbyte());
    if(cpu.CurrentSegment==&cpu.SegmentPrefix)
        segment = *cpu.CurrentSegment;
    else
        segment = cpu.SS;
    return (void *)mem_space+(segment<<4)+offset;
}
void *_rm11() {
    word segment, offset = cpu.regs.W.BP+cpu.regs.W.DI+signext(cpu_pfq_getbyte());
    if(cpu.CurrentSegment==&cpu.SegmentPrefix)
        segment = *cpu.CurrentSegment;
    else
        segment = cpu.SS;
    return (void *)mem_space+(segment<<4)+offset;
}
void *_rm12() {
    word segment, offset = cpu.regs.W.SI+signext(cpu_pfq_getbyte());
    segment = *cpu.CurrentSegment;
    return (void *)mem_space+(segment<<4)+offset;
}
void *_rm13() {
    word segment, offset = cpu.regs.W.DI+signext(cpu_pfq_getbyte());
    segment = *cpu.CurrentSegment;
    return (void *)mem_space+(segment<<4)+offset;
}
void *_rm14() {
    word segment, offset = cpu.regs.W.BP+signext(cpu_pfq_getbyte());
    if(cpu.CurrentSegment==&cpu.SegmentPrefix)
		segment = *cpu.CurrentSegment;
	else
		segment = cpu.SS;
    return (void *)mem_space+(segment<<4)+offset;
}
void *_rm15() {
    word segment, offset = cpu.regs.W.BX+signext(cpu_pfq_getbyte());
    segment = *cpu.CurrentSegment;
    return (void *)mem_space+(segment<<4)+offset;
}
void *_rm16() {
    word segment, offset = cpu.regs.W.BX+cpu.regs.W.SI+cpu_pfq_getword();
    segment = *cpu.CurrentSegment;
    return (void *)mem_space+(segment<<4)+offset;
}
void *_rm17() {
    word segment, offset = cpu.regs.W.BX+cpu.regs.W.DI+cpu_pfq_getword();
    segment = *cpu.CurrentSegment;
    return (void *)mem_space+(segment<<4)+offset;
}
void *_rm18() {
    word segment, offset = cpu.regs.W.BP+cpu.regs.W.SI+cpu_pfq_getword();
    if(cpu.CurrentSegment==&cpu.SegmentPrefix)
        segment = *cpu.CurrentSegment;
    else
        segment = cpu.SS;
    return (void *)mem_space+(segment<<4)+offset;
}
void *_rm19() {
    word segment, offset = cpu.regs.W.BP+cpu.regs.W.DI+cpu_pfq_getword();
    if(cpu.CurrentSegment==&cpu.SegmentPrefix)
        segment = *cpu.CurrentSegment;
    else
        segment = cpu.SS;
    return (void *)mem_space+(segment<<4)+offset;
}
void *_rm20() {
    word segment, offset = cpu.regs.W.SI+cpu_pfq_getword();
    segment = *cpu.CurrentSegment;
    return (void *)mem_space+(segment<<4)+offset;
}
void *_rm21() {
    word segment, offset = cpu.regs.W.DI+cpu_pfq_getword();
    segment = *cpu.CurrentSegment;
    return (void *)mem_space+(segment<<4)+offset;
}
void *_rm22() {
    word segment, offset = cpu.regs.W.BP+cpu_pfq_getword();
    if(cpu.CurrentSegment==&cpu.SegmentPrefix)
        segment = *cpu.CurrentSegment;
    else
        segment = cpu.SS;
    return (void *)mem_space+(segment<<4)+offset;
}
void *_rm23() {
    word segment, offset = cpu.regs.W.BX+cpu_pfq_getword();
    segment = *cpu.CurrentSegment;
    return (void *)mem_space+(segment<<4)+offset;
}

void *_rm24() { return &cpu.regs.B.AL; }
void *_rm25() { return &cpu.regs.B.CL; }
void *_rm26() { return &cpu.regs.B.DL; }
void *_rm27() { return &cpu.regs.B.BL; }
void *_rm28() { if(cpu_rmbits==8) return &cpu.regs.B.AH; else return &cpu.regs.W.SP; }
void *_rm29() { if(cpu_rmbits==8) return &cpu.regs.B.CH; else return &cpu.regs.W.BP; }
void *_rm30() { if(cpu_rmbits==8) return &cpu.regs.B.DH; else return &cpu.regs.W.SI; }
void *_rm31() { if(cpu_rmbits==8) return &cpu.regs.B.BH; else return &cpu.regs.W.DI; }

inline void
cpu_modrm_addfunc (int fn, void *(*function)()) {
	int i, y;
	int block = ((fn*8)/64) * 64;
	int head = (fn % 8);
	for(i=0;i<8;i++) {
		y = block + head + (i*8);
		cpu_modrmtable[y] = function;
	}
}

void
cpu_modrm_init() {
	cpu_modrm_addfunc( 0, _rm00); cpu_modrm_addfunc( 1, _rm01);
	cpu_modrm_addfunc( 2, _rm02); cpu_modrm_addfunc( 3, _rm03);
	cpu_modrm_addfunc( 4, _rm04); cpu_modrm_addfunc( 5, _rm05);
	cpu_modrm_addfunc( 6, _rm06); cpu_modrm_addfunc( 7, _rm07);
	cpu_modrm_addfunc( 8, _rm08); cpu_modrm_addfunc( 9, _rm09);
	cpu_modrm_addfunc(10, _rm10); cpu_modrm_addfunc(11, _rm11);
	cpu_modrm_addfunc(12, _rm12); cpu_modrm_addfunc(13, _rm13);
	cpu_modrm_addfunc(14, _rm14); cpu_modrm_addfunc(15, _rm15);
	cpu_modrm_addfunc(16, _rm16); cpu_modrm_addfunc(17, _rm17);
	cpu_modrm_addfunc(18, _rm18); cpu_modrm_addfunc(19, _rm19);
	cpu_modrm_addfunc(20, _rm20); cpu_modrm_addfunc(21, _rm21);
	cpu_modrm_addfunc(22, _rm22); cpu_modrm_addfunc(23, _rm23);
	cpu_modrm_addfunc(24, _rm24); cpu_modrm_addfunc(25, _rm25);
	cpu_modrm_addfunc(26, _rm26); cpu_modrm_addfunc(27, _rm27);
	cpu_modrm_addfunc(28, _rm28); cpu_modrm_addfunc(29, _rm29);
	cpu_modrm_addfunc(30, _rm30); cpu_modrm_addfunc(31, _rm31);
}

void
_LEA_reg16_mem16()
{
	byte rm = cpu_pfq_getbyte();
	*treg16[rmreg( rm )] = calcEA( rm );
}

word
calcEA (byte b) {	/* LEA address fetcher*/
	word retv = 0;
	switch (b & 0xC0) {
        case 0:
            switch(b & 0x07)
            {
                case 0: retv = cpu.regs.W.BX+cpu.regs.W.SI; break;
                case 1: retv = cpu.regs.W.BX+cpu.regs.W.DI; break;
                case 2: retv = cpu.regs.W.BP+cpu.regs.W.SI; break;
                case 3: retv = cpu.regs.W.BP+cpu.regs.W.DI; break;
                case 4: retv = cpu.regs.W.SI; break;
                case 5: retv = cpu.regs.W.DI; break;
                case 6: retv = cpu_pfq_getword(); break;
                case 7: retv = cpu.regs.W.BX; break;
            }
            break;
        case 64:
            switch(b & 0x07)
            {
                case 0: retv = cpu.regs.W.BX+cpu.regs.W.SI + signext(cpu_pfq_getbyte()); break;
                case 1: retv = cpu.regs.W.BX+cpu.regs.W.DI + signext(cpu_pfq_getbyte()); break;
                case 2: retv = cpu.regs.W.BP+cpu.regs.W.SI + signext(cpu_pfq_getbyte()); break;
                case 3: retv = cpu.regs.W.BP+cpu.regs.W.DI + signext(cpu_pfq_getbyte()); break;
                case 4: retv = cpu.regs.W.SI + signext(cpu_pfq_getbyte()); break;
                case 5: retv = cpu.regs.W.DI + signext(cpu_pfq_getbyte()); break;
                case 6: retv = cpu.regs.W.BP + signext(cpu_pfq_getbyte()); break;
                case 7: retv = cpu.regs.W.BX + signext(cpu_pfq_getbyte()); break;
            }
            break;
        case 128:
            switch(b & 0x07)
            {
                case 0: retv = cpu.regs.W.BX+cpu.regs.W.SI+cpu_pfq_getword(); break;
                case 1: retv = cpu.regs.W.BX+cpu.regs.W.DI+cpu_pfq_getword(); break;
                case 2: retv = cpu.regs.W.BP+cpu.regs.W.SI+cpu_pfq_getword(); break;
                case 3: retv = cpu.regs.W.BP+cpu.regs.W.DI+cpu_pfq_getword(); break;
                case 4: retv = cpu.regs.W.SI + cpu_pfq_getword(); break;
                case 5: retv = cpu.regs.W.DI + cpu_pfq_getword(); break;
                case 6: retv = cpu.regs.W.BP + cpu_pfq_getword(); break;
                case 7: retv = cpu.regs.W.BX + cpu_pfq_getword(); break;
            }
            break;
		case 192:
			vlog( VM_ALERT, "LEA with register source!" );
			/* LEA with register source, an invalid instruction.
			 * Call INT6 (invalid opcode exception) */
			int_call( 6 );
			break;
	}
    return retv;
}
