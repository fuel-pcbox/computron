/* 8086/math.c
 * Mathematical instuctions
 *
 */

#include "vomit.h"

void
cpu_mathflags( dword result, word dest, word src, byte bits )
{
	if( bits == 8 )
	{
		cpu.CF = ( result >> 8 ) & 1;
		cpu.SF = ( result >> 7 ) & 1;
		cpu.ZF = ( result & 0x00FF ) == 0;
	}
	else
	{
		cpu.CF = ( result >> 16 ) & 1;
		cpu.SF = ( result >> 15 ) & 1;
		cpu.ZF = ( result & 0xFFFF ) == 0;
	}
	cpu_setPF( result );
	cpu_setAF( result, dest, src );
}

void
cpu_cmpflags( dword result, word dest, word src, byte bits )
{
	cpu_mathflags( result, dest, src, bits );
	cpu.OF = ((
	         ((src)^(dest)) &
	         ((src)^(src-dest))
	         )>>(bits-1))&1;
}

dword
cpu_add( word dest, word src, byte bits )
{
	dword result = dest + src;
	cpu_mathflags( result, dest, src, bits );
	cpu.OF = ((
	         ((result)^(dest)) &
	         ((result)^(src))
	         )>>(bits-1))&1;
	return result;
}

dword
cpu_adc( word dest, word src, byte bits )
{
	dword result;
	src += cpu.CF;
	result = dest + src;

	cpu_mathflags( result, dest, src, bits );
	cpu.OF = ((
	         ((result)^(dest)) &
	         ((result)^(src))
	         )>>(bits-1))&1;
	return result;
}

dword
cpu_sub( word dest, word src, byte bits )
{
	dword result = dest - src;
	cpu_cmpflags( result, dest, src, bits );
	return result;
}

dword
cpu_sbb( word dest, word src, byte bits )
{
	dword result;
	src += cpu.CF;
	result = dest - src;
	cpu_cmpflags( result, dest, src, bits );
	return result;
}

dword
cpu_mul( word acc, word multi, byte bits )
{
	dword result = acc * multi;
	cpu_mathflags( result, acc, multi, bits );

	cpu.OF = cpu.CF = ( result & ( bits == 8 ? 0xFF00 : 0xFFFF0000 ) ) != 0;

	/* 8086 CPUs set ZF on zero result */
	if( cpu_type == INTEL_8086 )
	{
		cpu.ZF = (result == 0);
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
	cpu.regs.B.AL = cpu_add( cpu.regs.B.AL, imm, 8 );
}
void
_ADD_AX_imm16() {
	word imm = cpu_pfq_getword();
	cpu.regs.W.AX = cpu_add( cpu.regs.W.AX, imm, 16 );
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
	cpu.regs.B.AL = cpu_adc( cpu.regs.B.AL, imm, 8 );
}
void
_ADC_AX_imm16() {
	word imm = cpu_pfq_getword();
	cpu.regs.W.AX = cpu_adc( cpu.regs.W.AX, imm, 16 );
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
	cpu.regs.B.AL = cpu_sub( cpu.regs.B.AL, imm, 8 );
}
void
_SUB_AX_imm16() {
	word imm = cpu_pfq_getword();
	cpu.regs.W.AX = cpu_sub( cpu.regs.W.AX, imm, 16 );
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
	cpu.regs.B.AL = cpu_sbb( cpu.regs.B.AL, imm, 8 );
}
void
_SBB_AX_imm16() {
	word imm = cpu_pfq_getword();
	cpu.regs.W.AX = cpu_sbb( cpu.regs.W.AX, imm, 16 );
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
	cpu_sub( cpu.regs.B.AL, imm, 8 );
}
void
_CMP_AX_imm16() {
	word imm = cpu_pfq_getword();
	cpu_sub( cpu.regs.W.AX, imm, 16 );
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
	cpu.regs.W.AX = cpu_mul( cpu.regs.B.AL, *p, 8 );
}

void
_MUL_RM16()
{
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr( rm, 16 );
	dword result = cpu_mul( cpu.regs.W.AX, *p, 16 );
	cpu.regs.W.AX = result & 0xFFFF;
	cpu.regs.W.DX = (result >> 16) & 0xFFFF;
}

void
_IMUL_RM8()
{
	byte rm = cpu_rmbyte;
	sigbyte *p = cpu_rmptr( rm, 8 );
	cpu.regs.W.AX = (sigword) cpu_imul( cpu.regs.B.AL, *p, 8 );
	cpu.OF = cpu.CF = ( (sigbyte)cpu.regs.B.AL != cpu.regs.W.AX );
}

void
_IMUL_RM16()
{
	byte rm = cpu_rmbyte;
	sigword *p = cpu_rmptr(rm, 16);
	sigdword result = (sigdword) cpu_imul( cpu.regs.W.AX, *p, 16 );
	cpu.regs.W.AX = result;
	cpu.regs.W.DX = result >> 16;
	cpu.OF = cpu.CF = ( signext32( cpu.regs.W.AX ) != (dword)result );
}

void
_DIV_RM8()
{
	byte rm = cpu_rmbyte;
	word offend = cpu.IP;
	byte *p = cpu_rmptr( rm, 8 );
	word tAX = cpu.regs.W.AX;

	if( *p == 0 )
	{
		/* Exceptions return to offending IP */
		cpu.IP = offend - 2;
		int_call( 0 );
		return;
	}
	cpu.regs.B.AL = (byte)(tAX / *p); /* Quote        */
	cpu.regs.B.AH = (byte)(tAX % *p); /* Remainder    */
}

void
_DIV_RM16()
{
	byte rm = cpu_rmbyte;
	word offend = cpu.IP;
	word *p = cpu_rmptr( rm, 16 );
	dword tDXAX = cpu.regs.W.AX + (cpu.regs.W.DX << 16);

	if( *p == 0 )
	{
		/* See above. */
		cpu.IP = offend - 2;
		int_call(0x00);
		return;
	}
	cpu.regs.W.AX = (word)(tDXAX / *p); /* Quote      */
	cpu.regs.W.DX = (word)(tDXAX % *p); /* Remainder  */
}

void
_IDIV_RM8()
{
	byte rm = cpu_rmbyte;
	word offend = cpu.IP;
	sigbyte *p = cpu_rmptr( rm, 8 );
	sigword tAX = (sigword)cpu.regs.W.AX;
	if( *p == 0 )
	{
		/* Exceptions return to offending IP */
		cpu.IP = offend - 2;
		int_call( 0 );
		return;
	}
	cpu.regs.B.AL = (sigbyte)(tAX / *p); /* Quote        */
	cpu.regs.B.AH = (sigbyte)(tAX % *p); /* Remainder    */
}

void
_IDIV_RM16()
{
	word offend = cpu.IP;
	sigword *p = cpu_rmptr( cpu_rmbyte, 16 );
	sigdword tDXAX = (sigword)(cpu.regs.W.AX + (cpu.regs.W.DX << 16));

	if( *p == 0 )
	{
		/* See above. */
		cpu.IP = offend - 2;
		int_call( 0 );
		return;
	}
	cpu.regs.W.AX = (sigword)(tDXAX / *p); /* Quote      */
	cpu.regs.W.DX = (sigword)(tDXAX % *p); /* Remainder  */
}
