/* 8086/misc.c
 * Miscellaneous instructions
 *
 */

#include "vomit.h"

#ifdef VM_UNIX
	#include <unistd.h>
#endif

void
_NOP() {						/* Do nothing. Yay. */
	return;
}

void
_HLT() {		/* Put the CPU in halt state. Await interrupt. */
	cpu_state = CPU_HALTED;
#ifdef VM_DEBUG
		if(verbose) printf("cpu: CPU halted. Awaiting interrupt...\n");
#endif
	while ( cpu_state == CPU_HALTED ) {
#ifdef VM_UNIX
			usleep(100);                /* Sleep for 100ms when halted. Prevents resource sucking. */
#endif
	}
}

void
cpu_updflags (word data, byte bits) {
	if(bits==8) data &= 0xFF; else data &= 0xFFFF;
	cpu_setPF((dword)data);
	cpu_setZF((dword)data);
	cpu_setSF((dword)data, bits);
}

void _STC() { CF = 1; } void _STD() { DF = 1; } void _STI() { IF = 1; }
void _CLC() { CF = 0; } void _CLD() { DF = 0; } void _CLI() { IF = 0; }
void _CMC() { CF = !CF; }

void
_XLAT() {
	*treg8[REG_AL] = mem_getbyte(*CurrentSegment, BX+*treg8[REG_AL]);
	return;
}

void
_CS() {
	byte opcode;
	SegmentPrefix = CS;
	CurrentSegment = &SegmentPrefix;
	opcode = cpu_pfq_getbyte();
	cpu_optable[opcode]();
	CurrentSegment = &DS;
	return;
}
void
_DS() {
	byte opcode;
	SegmentPrefix = DS;
	CurrentSegment = &SegmentPrefix;
	opcode = cpu_pfq_getbyte();
	cpu_optable[opcode]();
	CurrentSegment = &DS;
	return;
}
void
_ES() {
	byte opcode;
	SegmentPrefix = ES;
	CurrentSegment = &SegmentPrefix;
	opcode = cpu_pfq_getbyte();
	cpu_optable[opcode]();
	CurrentSegment = &DS;
	return;
}
void
_SS() {
	byte opcode;
	SegmentPrefix = SS;
	CurrentSegment = &SegmentPrefix;
	opcode = cpu_pfq_getbyte();
	cpu_optable[opcode]();
	CurrentSegment = &DS;
	return;
}

void
_LAHF() {
	*treg8[REG_AH] = CF*1+PF*4+AF*16+ZF*64+SF*128+2;
}

void
_SAHF() {
	CF = ((*treg8[REG_AH] & 1)>0);
	PF = ((*treg8[REG_AH] & 4)>0);
	AF = ((*treg8[REG_AH] & 16)>0);
	ZF = ((*treg8[REG_AH] & 64)>0);
	SF = ((*treg8[REG_AH] & 128)>0);
}

void
_XCHG_AX_reg16() {
	word tmpax = AX;
	AX = *treg16[cpu_opcode & 7];
	*treg16[cpu_opcode & 7] = tmpax;
}

void
_XCHG_reg8_RM8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	byte tmpreg = *treg8[rmreg(rm)];
	*treg8[rmreg(rm)] = *p;
	*p = tmpreg;
}

void
_XCHG_reg16_RM16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	word tmpreg = *treg16[rmreg(rm)];
	*treg16[rmreg(rm)] = *p;
	*p = tmpreg;
}

void
_DEC_reg16() {
	dword i = *treg16[cpu_opcode & 7];
	OF = i != 0 ? 0 : 1; 
	i--;
	cpu_setAF(i, *treg16[cpu_opcode & 7], 1);
	cpu_updflags(i, 16);
	--*treg16[cpu_opcode & 7];
	return;
}
void
_INC_reg16() {
	dword i = *treg16[cpu_opcode & 7];
	OF = i != 32767 ? 0 : 1;
	i++;
	cpu_setAF(i,*treg16[cpu_opcode & 7],1);
	cpu_updflags(i, 16);
	++*treg16[cpu_opcode & 7];
	return;
}

void
_INC_RM16() {
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	dword i = *p;
	OF = i != 32767 ? 0 : 1;
	i++;
	cpu_setAF(i,*p,1);
	cpu_updflags(i, 16);
	++*p;
	return;
}

void
_DEC_RM16() {
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	dword i = *p;
	OF = i != 0 ? 0 : 1;
	i--;
	cpu_setAF(i,*p,1);
	cpu_updflags(i, 16);
	--*p;
	return;
}

word
signext (byte b) {
	word w = 0x0000 + b;
	if ((w&0x80)>0)
		return (w | 0xff00);
	else
		return (w & 0x00ff);
}

dword
signext32 (word w) {
	dword d = 0x00000000 + w;
	if ((d&0x8000)>0)
		return (d | 0xffff0000);
	else
		return (d & 0x0000ffff);
}

