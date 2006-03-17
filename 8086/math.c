/* 8086/math.c
 * Mathematical instuctions
 *
 */

#include "vomit.h"

void
cpu_mathflags (dword result, word dest, word src, byte bits) {
	if ( bits == 8 ) {
		CF = ( result >> 8 ) & 1;
		SF = ( result >> 7 ) & 1;
		ZF = ( result & 0x00FF ) == 0;
	} else {
		CF = ( result >> 16 ) & 1;
		SF = ( result >> 15 ) & 1;
		ZF = ( result & 0xFFFF ) == 0;
	}
	cpu_setPF( result );
	cpu_setAF( result, dest, src );
}

void
cpu_cmpflags (dword result, word dest, word src, byte bits) {
	cpu_mathflags(result, dest, src, bits);
	OF =	((
			((src)^(dest)) &
			((src)^(src-dest))
			)>>(bits-1))&1;
}

dword
cpu_add (word dest, word src, byte bits) {
	dword result = dest+src;
	cpu_mathflags(result, dest, src, bits);
	OF =	((
			((result)^(dest)) &
			((result)^(src))
			)>>(bits-1))&1;
	return result;
}

dword
cpu_adc (word dest, word src, byte bits) {
	dword result;
	src += CF;
	result = dest+src;

	cpu_mathflags(result, dest, src, bits);
	OF =	((
			((result)^(dest)) &
			((result)^(src))
			)>>(bits-1))&1;
	return result;
}

dword
cpu_sub (word dest, word src, byte bits) {
	dword result = dest-src;
	cpu_cmpflags(result, dest, src, bits);
	return result;
}

dword
cpu_sbb (word dest, word src, byte bits) {
	dword result;
	src+=CF;
	result = dest-src;
	cpu_cmpflags(result, dest, src, bits);
	return result;
}

dword
cpu_mul( word acc, word multi, byte bits )
{
	dword result = acc * multi;
	cpu_mathflags( result, acc, multi, bits );

	OF = CF = ( result & ( bits == 8 ? 0xFF00 : 0xFFFF0000 ) ) != 0;

	/* 8086 CPUs set ZF on zero result */
	if( cpu_type == INTEL_8086 )
	{
		ZF = (result == 0);
	}
	return result;
}

dword
cpu_imul (word acc, word multi, byte bits) {
	sigword result = (sigword)acc * (sigword)multi;
	cpu_mathflags(result, acc, multi, bits);
	return result;
}

/* --------- ADD --------- */

void
_ADD_RM8_reg8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	*p = cpu_add(*p, *treg8[rmreg(rm)], 8);
}
void
_ADD_RM16_reg16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	*p = cpu_add(*p, *treg16[rmreg(rm)], 16);
}
void
_ADD_reg8_RM8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	*treg8[rmreg(rm)] = cpu_add(*treg8[rmreg(rm)], *p, 8);
}
void
_ADD_reg16_RM16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	*treg16[rmreg(rm)] = cpu_add(*treg16[rmreg(rm)], *p, 16);
}
void
_ADD_AL_imm8() {
	byte imm = cpu_pfq_getbyte();
	*treg8[REG_AL] = cpu_add(*treg8[REG_AL], imm, 8);
}
void
_ADD_AX_imm16() {
	word imm = cpu_pfq_getword();
	AX = cpu_add(AX, imm, 16);
}
void
_ADD_RM8_imm8() {
	byte rm = cpu_rmbyte;
	byte *p = cpu_rmptr(rm, 8);
	byte imm = cpu_pfq_getbyte();
	*p = cpu_add(*p, imm, 8);
}
void
_ADD_RM16_imm16() {
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	word imm = cpu_pfq_getword();
	*p = cpu_add(*p, imm, 16);
}
void
_ADD_RM16_imm8() {
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	word imm = signext(cpu_pfq_getbyte());
	*p = cpu_add(*p, imm, 16);
}

/* --------- ADC --------- */

void
_ADC_RM8_reg8() {
    byte rm = cpu_pfq_getbyte();
    byte *p = cpu_rmptr(rm, 8);
    *p = cpu_adc(*p, *treg8[rmreg(rm)], 8);
}
void
_ADC_RM16_reg16() {
    byte rm = cpu_pfq_getbyte();
    word *p = cpu_rmptr(rm, 16);
    *p = cpu_adc(*p, *treg16[rmreg(rm)], 16);
}
void
_ADC_reg8_RM8() {
    byte rm = cpu_pfq_getbyte();
    byte *p = cpu_rmptr(rm, 8);
    *treg8[rmreg(rm)] = cpu_adc(*treg8[rmreg(rm)], *p, 8);
}
void
_ADC_reg16_RM16() {
    byte rm = cpu_pfq_getbyte();
    word *p = cpu_rmptr(rm, 16);
    *treg16[rmreg(rm)] = cpu_adc(*treg16[rmreg(rm)], *p, 16);
}
void
_ADC_AL_imm8() {
	byte imm = cpu_pfq_getbyte();
	*treg8[REG_AL] = cpu_adc(*treg8[REG_AL], imm, 8);
}
void
_ADC_AX_imm16() {
	word imm = cpu_pfq_getword();
	AX = cpu_adc(AX, imm, 16);
}
void
_ADC_RM8_imm8() {
	byte rm = cpu_rmbyte;
	byte *p = cpu_rmptr(rm, 8);
	byte imm = cpu_pfq_getbyte();
	*p = cpu_adc(*p, imm, 8);
}
void
_ADC_RM16_imm16() {
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	word imm = cpu_pfq_getword();
	*p = cpu_adc(*p, imm, 16);
}
void
_ADC_RM16_imm8() {
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	word imm = signext(cpu_pfq_getbyte());
	*p = cpu_adc(*p, imm, 16);
}

/* --------- SUB --------- */

void
_SUB_RM8_reg8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	*p = cpu_sub(*p, *treg8[rmreg(rm)], 8);
}
void
_SUB_RM16_reg16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	*p = cpu_sub(*p, *treg16[rmreg(rm)], 16);
}
void
_SUB_reg8_RM8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	*treg8[rmreg(rm)] = cpu_sub(*treg8[rmreg(rm)], *p, 8);
}
void
_SUB_reg16_RM16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	*treg16[rmreg(rm)] = cpu_sub(*treg16[rmreg(rm)], *p, 16);
}
void
_SUB_AL_imm8() {
	byte imm = cpu_pfq_getbyte();
	*treg8[REG_AL] = cpu_sub(*treg8[REG_AL], imm, 8);
}
void
_SUB_AX_imm16() {
	word imm = cpu_pfq_getword();
	AX = cpu_sub(AX, imm, 16);
}
void
_SUB_RM8_imm8() {
	byte rm = cpu_rmbyte;
	byte *p = cpu_rmptr(rm, 8);
	byte imm = cpu_pfq_getbyte();
	*p = cpu_sub(*p, imm, 8);
}
void
_SUB_RM16_imm16() {
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	word imm = cpu_pfq_getword();
	*p = cpu_sub(*p, imm, 16);
}
void
_SUB_RM16_imm8() {
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	word imm = signext(cpu_pfq_getbyte());
	*p = cpu_sub(*p, imm, 16);
}

/* --------- SBB --------- */

void
_SBB_RM8_reg8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	*p = cpu_sbb(*p, *treg8[rmreg(rm)], 8);
}
void
_SBB_RM16_reg16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	*p = cpu_sbb(*p, *treg16[rmreg(rm)], 16);
}
void
_SBB_reg8_RM8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	*treg8[rmreg(rm)] = cpu_sbb(*treg8[rmreg(rm)], *p, 8);
}
void
_SBB_reg16_RM16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	*treg16[rmreg(rm)] = cpu_sbb(*treg16[rmreg(rm)], *p, 16);
}
void
_SBB_AL_imm8() {
	byte imm = cpu_pfq_getbyte();
	*treg8[REG_AL] = cpu_sbb(*treg8[REG_AL], imm, 8);
}
void
_SBB_AX_imm16() {
	word imm = cpu_pfq_getword();
	AX = cpu_sbb(AX, imm, 16);
}
void
_SBB_RM8_imm8() {
	byte rm = cpu_rmbyte;
	byte *p = cpu_rmptr(rm, 8);
	byte imm = cpu_pfq_getbyte();
	*p = cpu_sbb(*p, imm, 8);
}
void
_SBB_RM16_imm16() {
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	word imm = cpu_pfq_getword();
	*p = cpu_sbb(*p, imm, 16);
}
void
_SBB_RM16_imm8() {
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	word imm = signext(cpu_pfq_getbyte());
	*p = cpu_sbb(*p, imm, 16);
}

/* --------- CMP --------- */

void
_CMP_RM8_reg8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	cpu_sub(*p, *treg8[rmreg(rm)], 8);
}
void
_CMP_RM16_reg16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	cpu_sub(*p, *treg16[rmreg(rm)], 16);
}
void
_CMP_reg8_RM8() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	cpu_sub(*treg8[rmreg(rm)], *p, 8);
}
void
_CMP_reg16_RM16() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	cpu_sub(*treg16[rmreg(rm)], *p, 16);
}
void
_CMP_AL_imm8() {
	byte imm = cpu_pfq_getbyte();
	cpu_sub(*treg8[REG_AL], imm, 8);
}
void
_CMP_AX_imm16() {
	word imm = cpu_pfq_getword();
	cpu_sub(AX, imm, 16);
}
void
_CMP_RM8_imm8() {
	byte rm = cpu_rmbyte;
	byte *p = cpu_rmptr(rm, 8);
	byte imm = cpu_pfq_getbyte();
	cpu_sub(*p, imm, 8);
}
void
_CMP_RM16_imm16() {
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	word imm = cpu_pfq_getword();
	cpu_sub(*p, imm, 16);
}
void
_CMP_RM16_imm8() {
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	word imm = signext(cpu_pfq_getbyte());
	cpu_sub(*p, imm, 16);
}

/* -------- MUL --------- */

void
_MUL_RM8()
{
	byte rm = cpu_rmbyte;
	byte *p = cpu_rmptr( rm, 8 );
	AX = cpu_mul( *treg8[REG_AL], *p, 8 );
}

void
_MUL_RM16()
{
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr( rm, 16 );
	dword result = cpu_mul( AX, *p, 16 );
	AX = result & 0xFFFF;
	DX = (result >> 16) & 0xFFFF;
}

void
_IMUL_RM8()
{
	byte rm = cpu_rmbyte;
	sigbyte *p = cpu_rmptr( rm, 8 );
	AX = (sigword) cpu_imul( *treg8[REG_AL], *p, 8 );
	OF = CF = ( (sigbyte)*treg8[REG_AL] != AX );
}

void
_IMUL_RM16()
{
	byte rm = cpu_rmbyte;
	sigword *p = cpu_rmptr(rm, 16);
	sigdword result = (sigdword) cpu_imul(AX, *p, 16);
	AX = result;
	DX = result >> 16;
	OF = CF = ( signext32( AX ) != (dword)result );
}

void
_DIV_RM8() {
	byte rm = cpu_rmbyte;
	word offend = IP;
	byte *p = cpu_rmptr(rm, 8);
	word tAX = AX;
	if( (*p==0) ) {
		IP = offend-2;		/* Exceptions return to offending IP */
		int_call(0x00);
		return;
	}
	*treg8[REG_AL] = (byte) (tAX / *p); /* Quote        */
	*treg8[REG_AH] = (byte) (tAX % *p); /* Remainder    */
	return;
}

void
_DIV_RM16() {
	byte rm = cpu_rmbyte;
	word offend = IP;
	word *p = cpu_rmptr(rm, 16);
	dword tDXAX = AX + (DX << 16);
	if( (*p==0) ) {
		IP = offend-2;				/* See above. */
		int_call(0x00);
		return;
	}
    AX = (word) (tDXAX / *p); /* Quote      */
    DX = (word) (tDXAX % *p); /* Remainder  */
	return;
}

void
_IDIV_RM8() {
	byte rm = cpu_rmbyte;
	word offend = IP;
	sigbyte *p = cpu_rmptr( rm, 8 );
	sigword tAX = (sigword)AX;
	if( (*p==0) ) {
		IP = offend-2;		/* Exceptions return to offending IP */
		int_call(0x00);
		return;
	}
	*treg8[REG_AL] = (sigbyte) (tAX / *p); /* Quote        */
	*treg8[REG_AH] = (sigbyte) (tAX % *p); /* Remainder    */
	return;
}

void
_IDIV_RM16()
{
	word offend = IP;
	sigword *p = cpu_rmptr( cpu_rmbyte, 16 );
	sigdword tDXAX = (sigword)(AX + (DX << 16));
	if( (*p==0) ) {
		IP = offend-2;				/* See above. */
		int_call(0x00);
		return;
	}
	AX = (sigword) (tDXAX / *p); /* Quote      */
	DX = (sigword) (tDXAX % *p); /* Remainder  */
	return;
}

