/* 8086/bcd.c
 * BCD instruction handlers
 *
 * Based entirely on the Intel IA32 manual.
 *
 */

#include "vomit.h"

void _AAA() {
	if(((*treg8[REG_AL] & 0x0F)>9) || (AF==1)) {
		*treg8[REG_AL] += 6;
		*treg8[REG_AH] += 1;
		AF = 1;
		CF = 1;
	} else {
		AF = 0;
		CF = 0;
	}
	*treg8[REG_AL] = *treg8[REG_AL] & 0x0F;
}

void _AAM() {
	word offend = IP;				/* security bloat */
	byte tempAL = *treg8[REG_AL];
	byte imm = cpu_pfq_getbyte();
    if(imm==0) {
        IP = --offend;              /* Exceptions return to offending IP */
        int_call(0x00);
        return;
    }
	*treg8[REG_AH] = tempAL / imm;
	*treg8[REG_AL] = tempAL % imm;
	cpu_updflags(*treg8[REG_AL], 8);
}

void _AAD() {
	byte tempAL = *treg8[REG_AL];
	byte tempAH = *treg8[REG_AH];
	byte imm = cpu_pfq_getbyte();
	*treg8[REG_AL] = (tempAL + (tempAH * imm)) & 0xFF;
	*treg8[REG_AH] = 0x00;
	cpu_updflags(*treg8[REG_AL], 8);
}

void _AAS() {
	if(((*treg8[REG_AL] & 0x0F) > 9) || (AF == 1) ) {
		*treg8[REG_AL] -= 6;
		*treg8[REG_AH] -= 1;
		AF = 1;
		CF = 1;
	} else {
		AF = 0;
		CF = 0;
	}
}

void _DAS() {
    byte oldcf = CF;
	byte oldal = *treg8[REG_AL];
	CF = 0;
    if(( (*treg8[REG_AL]&0x0F) > 0x09 ) || AF==1) {
		CF = ((*treg8[REG_AL]-6) >> 8) & 1;
        *treg8[REG_AL] -= 0x06;
        CF = oldcf | CF;
        AF = 1;
    } else { AF = 0; }
    if((oldal>0x99) || oldcf==1) {
        *treg8[REG_AL] -= 0x60;
        CF = 1;
    } else { CF = 0; }
}

void _DAA() {
	byte oldcf = CF;
	byte oldal = *treg8[REG_AL];
	CF = 0;
	if(( (*treg8[REG_AL]&0x0F) > 0x09) || AF==1) {
		CF = ((*treg8[REG_AL]+6) >> 8) & 1;
		*treg8[REG_AL] += 6;
		CF = oldcf | CF;
		AF = 1;
	} else {
		AF = 0;
	}
	if((oldal>0x99) || oldcf==1) {
		*treg8[REG_AL] += 0x60;
		CF = 1;
	} else { CF = 0; }
}

