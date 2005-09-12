/* 8086/bitwise.c
 * Bitwise instructions
 *
 *
 *
 */

#include "vomit.h"

void
_CBW() {
	AX = signext (*treg8[REG_AL]);
}

void
_CWD() {
	if ((*treg8[REG_AH]>>7)==1)
		DX = 0xFFFF;
	else
		DX = 0x0000;
}

void
_SALC() {
	*treg8[REG_AL] = CF << 7;
}

dword cpu_or(word dest, word src, byte bits) {
	dword result = dest|src;
	cpu_updflags(result, bits);
	OF = 0; CF = 0;
	return result;
}

dword cpu_xor(word dest, word src, byte bits) {
	dword result = dest^src;
	cpu_updflags(result, bits);
	OF = 0; CF = 0;
	return result;
}

dword cpu_and(word dest, word src, byte bits) {
	dword result = dest&src;
	cpu_updflags(result, bits);
	OF = 0; CF = 0;
	return result;
}

dword cpu_shl(word data, byte itn, byte bits) {
	register int i;
	dword result = (dword)data;
	if(bits==8) {
		for(i=0;i<itn;i++) {
			CF = (result>>7) & 1;
			result = result << 1;
		}
		if(itn==1) OF = (data>>7)^CF;
		cpu_updflags(result, 8);
	} else {
		for(i=0;i<itn;i++) {
			CF = (result>>15) & 1;
			result = result << 1;
		}
		if(itn==1) OF = (data>>15)^CF;
		cpu_updflags(result, 16);
	}
	return result;
}

dword cpu_shr(word data, byte itn, byte bits) {
	register int i;
	dword result = (dword)data;
	if(bits==8) {
		for(i=0;i<itn;i++) {
			CF = (result&1);
			result = result >> 1;
		}
		if(itn==1) OF = (data>>7) & 1;
		cpu_updflags(result, 8);
	} else {
		for(i=0;i<itn;i++) {
			CF = (result&1);
			result = result >> 1;
		}
		if(itn==1) OF = (data>>15) & 1;
		cpu_updflags(result, 16);
	}
	return result;
}

dword cpu_sar(word data, byte itn, byte bits) {
	register int i;
	dword result = (dword)data;
	word n;
	if(bits==8) {
		for(i=0;i<itn;i++) {
			n = result;
			result = (result>>1) | (n&0x80);
			CF = n&1;
		}
		if(itn==1) OF = 0;
		cpu_updflags(result, 8);
	} else {
		for(i=0;i<itn;i++) {
			n = result;
			result = (result>>1) | (n&0x8000);
			CF = n&1;
		}
		if(itn==1) OF = 0;
		cpu_updflags(result, 16);
	}
	return result;
}

dword cpu_rol(word data, byte itn, byte bits) {
	register int i;
	dword result = (dword)data;
	if(bits==8) {
		for(i=0;i<itn;i++) {
			CF = (result>>7)&1;
			result = (result<<1) | CF;
		}
		if(itn==1) OF = ((result>>7)&1)^CF;
	} else {
		for(i=0;i<itn;i++) {
			CF = (result>>15)&1;
			result = (result<<1) | CF;
		}
		if(itn==1) OF = ((result>>15)&1)^CF;
	}
	return result;
}

dword cpu_ror(word data, byte itn, byte bits) {
	register int i;
	dword result = (dword)data;
	if(bits==8) {
		for(i=0;i<itn;i++) {
			CF = result & 1;
			result = (result>>1) | (CF<<7);
		}
		if(itn==1) OF = (result>>7)^((result>>6)&1);
	} else {
		for(i=0;i<itn;i++) {
			CF = result & 1;
			result = (result>>1) | (CF<<15);
		}
		if(itn==1) OF = (result>>15)^((result>>14)&1);
	}				
	return result;
}

dword cpu_rcl(word data, byte itn, byte bits) {
	register int i;
	dword result = (dword)data;
	word n;
	if(bits==8) {
		for(i=0;i<itn;i++) {
			n = result;
			result = ((result<<1)&0xFF) | CF;
			CF = (n>>7)&1;
		}
		if(itn==1) OF=(result>>7)^CF;
	} else {
		for(i=0;i<itn;i++) {
			n = result;
			result = ((result<<1)&0xFFFF) | CF;
			CF = (n>>15)&1;
		}
		if(itn==1) OF = (result>>15)^CF;
	}
	return result;
}

dword cpu_rcr(word data, byte itn, byte bits) {
	register int i;
	dword result = (dword)data;
	word n;
	if(bits==8) {
		for(i=0;i<itn;i++) {
			n = result;
			result = (result>>1) | (CF<<7);
			CF = n&1;
		}
		if(itn==1) OF = ((result>>7)^(result>>6)) & 1;
	} else {
		for(i=0;i<itn;i++) {
			n = result;
			result = (result>>1) | (CF<<15);
			CF = n&1;
		}
		if(itn==1) OF = ((result>>15)^(result>>14)) & 1;
	}
	return result;
}

void _OR_RM8_reg8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	*p = cpu_or(*p, *treg8[rmreg(rm)], 8);
}
void _OR_RM16_reg16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	*p = cpu_or(*p, *treg16[rmreg(rm)], 16);
}
void _OR_reg8_RM8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	*treg8[rmreg(rm)] = cpu_or(*treg8[rmreg(rm)], *p, 8);
}
void _OR_reg16_RM16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	*treg16[rmreg(rm)] = cpu_or(*treg16[rmreg(rm)], *p, 16);
}
void _OR_RM8_imm8() {			/* From wrapper 0x80	*/
	byte rm = cpu_rmbyte;
	byte *p = cpu_rmptr(rm, 8);
	*p = cpu_or(*p, cpu_pfq_getbyte(), 8);
}
void _OR_RM16_imm16() {			/* From wrapper 0x81	*/
	byte rm = cpu_rmbyte;	
	word *p = cpu_rmptr(rm, 16);
	*p = cpu_or(*p, cpu_pfq_getword(), 16);
}
void _OR_RM16_imm8() {			/* From wrapper 0x83	*/
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	*p = cpu_or(*p, signext(cpu_pfq_getbyte()), 16);
}
void _OR_AL_imm8() { *treg8[REG_AL] = cpu_or(*treg8[REG_AL], cpu_pfq_getbyte(), 8); }
void _OR_AX_imm16() { AX = cpu_or(AX, cpu_pfq_getword(), 16); }

void _XOR_RM8_reg8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	*p = cpu_xor(*p, *treg8[rmreg(rm)], 8);
}
void _XOR_RM16_reg16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	*p = cpu_xor(*p, *treg16[rmreg(rm)], 16);
}
void _XOR_reg8_RM8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	*treg8[rmreg(rm)] = cpu_xor(*treg8[rmreg(rm)], *p, 8);
}
void _XOR_reg16_RM16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	*treg16[rmreg(rm)] = cpu_xor(*treg16[rmreg(rm)], *p, 16);
}
void _XOR_RM8_imm8() {                /* From wrapper 0x80    */
	byte rm = cpu_rmbyte;
	byte *p = cpu_rmptr(rm, 8);
	*p = cpu_xor(*p, cpu_pfq_getbyte(), 8);
}
void _XOR_RM16_imm16() {              /* From wrapper 0x81    */
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	*p = cpu_xor(*p, cpu_pfq_getword(), 16);
}
void _XOR_RM16_imm8() {               /* From wrapper 0x83    */
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	*p = cpu_xor(*p, signext(cpu_pfq_getbyte()), 16);
}
void _XOR_AL_imm8() { *treg8[REG_AL] = cpu_xor(*treg8[REG_AL], cpu_pfq_getbyte(), 8); }
void _XOR_AX_imm16() { AX = cpu_xor(AX, cpu_pfq_getword(), 16); }

void _AND_RM8_reg8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	*p = cpu_and(*p, *treg8[rmreg(rm)], 8);
}
void _AND_RM16_reg16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	*p = cpu_and(*p, *treg16[rmreg(rm)], 16);
}
void _AND_reg8_RM8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	*treg8[rmreg(rm)] = cpu_and(*treg8[rmreg(rm)], *p, 8);
}
void _AND_reg16_RM16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	*treg16[rmreg(rm)] = cpu_and(*treg16[rmreg(rm)], *p, 16);
}
void _AND_RM8_imm8() {                /* From wrapper 0x80    */
	byte rm = cpu_rmbyte;
	byte *p = cpu_rmptr(rm, 8);
	*p = cpu_and(*p, cpu_pfq_getbyte(), 8);
}
void _AND_RM16_imm16() {              /* From wrapper 0x81    */
	byte rm = cpu_rmbyte; 
 	word *p = cpu_rmptr(rm, 16);
	*p = cpu_and(*p, cpu_pfq_getword(), 16);
}
void _AND_RM16_imm8() {               /* From wrapper 0x83    */
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	*p = cpu_and(*p, signext(cpu_pfq_getbyte()), 16);
}
void _AND_AL_imm8() { *treg8[REG_AL] = cpu_and(*treg8[REG_AL], cpu_pfq_getbyte(), 8); }
void _AND_AX_imm16() { AX = cpu_and(AX, cpu_pfq_getword(), 16); }

void _NOT_RM8() {				/* From wrapper 0xF6	*/
	byte rm = cpu_rmbyte;
	byte *p = cpu_rmptr(rm, 8);
	*p = ~(*p);
}
void _NOT_RM16() {			/* From wrapper 0xF7	*/
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	*p = ~(*p);
}
void _NEG_RM8() {				/* From wrapper 0xF6	*/
	byte rm = cpu_rmbyte;
	byte *p = cpu_rmptr(rm, 8);
	byte old = *p;
	*p = 0 - *p;
	if (old == 0) CF = 0; else CF = 1;
	cpu_updflags(*p, 8);
	OF =	((
			((0)^(old)) &
			((0)^(0-old))
			)>>(7))&1;
	cpu_setAF(0-old, 0, old);
}
void _NEG_RM16() {				/* From wrapper 0xF7	*/
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	word old = *p;
	*p = 0 - *p;
	if (old == 0) CF = 0; else CF = 1;
	cpu_updflags(*p, 16);
	OF =	((
			((0)^(old)) &
			((0)^(0-old))
			)>>(15))&1;
	cpu_setAF(0-old, 0, old);
}

void _TEST_RM8_reg8() {
    byte rm = cpu_pfq_getbyte();
    byte *p = cpu_rmptr(rm, 8);
    cpu_and(*p, *treg8[rmreg(rm)], 8);
}
void _TEST_RM16_reg16() {
    byte rm = cpu_pfq_getbyte();
    word *p = cpu_rmptr(rm, 16);
    cpu_and(*p, *treg16[rmreg(rm)], 16);
}
void _TEST_RM8_imm8() {		/* From wrapper 0xF6    */
	byte rm = cpu_rmbyte;
	byte *p = cpu_rmptr(rm, 8);
	cpu_and(*p, cpu_pfq_getbyte(), 8);
}
void _TEST_RM16_imm16() {		/* From wrapper 0xF7    */
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	cpu_and(*p, cpu_pfq_getword(), 16);
}
void _TEST_AL_imm8() { cpu_and(*treg8[REG_AL], cpu_pfq_getbyte(), 8); }
void _TEST_AX_imm16() { cpu_and(AX, cpu_pfq_getword(), 16); }

