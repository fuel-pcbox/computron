/* 8086/bitwise.c
 * Bitwise instructions
 *
 *
 *
 */

#define MASK_ITN_IF_80186 if( cpu_type == INTEL_80186 ) { itn &= 0x1F; }

#include "vomit.h"
#include "templates.h"

DEFAULT_RM8_reg8( cpu_and, _AND_RM8_reg8 )
DEFAULT_RM16_reg16( cpu_and, _AND_RM16_reg16 )
DEFAULT_reg8_RM8( cpu_and, _AND_reg8_RM8 )
DEFAULT_reg16_RM16( cpu_and, _AND_reg16_RM16 )
DEFAULT_RM8_imm8( cpu_and, _AND_RM8_imm8 )
DEFAULT_RM16_imm16( cpu_and, _AND_RM16_imm16 )
DEFAULT_RM16_imm8( cpu_and, _AND_RM16_imm8 )
DEFAULT_AL_imm8( cpu_and, _AND_AL_imm8 )
DEFAULT_AX_imm16( cpu_and, _AND_AX_imm16 )

DEFAULT_RM8_reg8( cpu_xor, _XOR_RM8_reg8 )
DEFAULT_RM16_reg16( cpu_xor, _XOR_RM16_reg16 )
DEFAULT_reg8_RM8( cpu_xor, _XOR_reg8_RM8 )
DEFAULT_reg16_RM16( cpu_xor, _XOR_reg16_RM16 )
DEFAULT_RM8_imm8( cpu_xor, _XOR_RM8_imm8 )
DEFAULT_RM16_imm16( cpu_xor, _XOR_RM16_imm16 )
DEFAULT_RM16_imm8( cpu_xor, _XOR_RM16_imm8 )
DEFAULT_AL_imm8( cpu_xor, _XOR_AL_imm8 )
DEFAULT_AX_imm16( cpu_xor, _XOR_AX_imm16 )

DEFAULT_RM8_reg8( cpu_or, _OR_RM8_reg8 )
DEFAULT_RM16_reg16( cpu_or, _OR_RM16_reg16 )
DEFAULT_reg8_RM8( cpu_or, _OR_reg8_RM8 )
DEFAULT_reg16_RM16( cpu_or, _OR_reg16_RM16 )
DEFAULT_RM8_imm8( cpu_or, _OR_RM8_imm8 )
DEFAULT_RM16_imm16( cpu_or, _OR_RM16_imm16 )
DEFAULT_RM16_imm8( cpu_or, _OR_RM16_imm8 )
DEFAULT_AL_imm8( cpu_or, _OR_AL_imm8 )
DEFAULT_AX_imm16( cpu_or, _OR_AX_imm16 )

READONLY_RM8_reg8( cpu_and, _TEST_RM8_reg8 )
READONLY_RM16_reg16( cpu_and, _TEST_RM16_reg16 )
READONLY_reg8_RM8( cpu_and, _TEST_reg8_RM8 )
READONLY_reg16_RM16( cpu_and, _TEST_reg16_RM16 )
READONLY_RM8_imm8( cpu_and, _TEST_RM8_imm8 )
READONLY_RM16_imm16( cpu_and, _TEST_RM16_imm16 )
READONLY_RM16_imm8( cpu_and, _TEST_RM16_imm8 )
READONLY_AL_imm8( cpu_and, _TEST_AL_imm8 )
READONLY_AX_imm16( cpu_and, _TEST_AX_imm16 )

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
_SALC()
{
	*treg8[REG_AL] = CF * 0xFF;
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
	MASK_ITN_IF_80186;
	if(bits==8) {
		for(i=0;i<itn;i++) {
			CF = (result>>7) & 1;
			result = result << 1;
		}
	} else {
		for(i=0;i<itn;i++) {
			CF = (result>>15) & 1;
			result = result << 1;
		}
	}
	if ( itn == 1 ) {
		OF = ( data >> ( bits - 1 ) ) ^ CF;
	}
	cpu_updflags( result, bits );
	return result;
}

dword cpu_shr(word data, byte itn, byte bits) {
	register int i;
	dword result = (dword)data;
	MASK_ITN_IF_80186;
	if(bits==8) {
		for(i=0;i<itn;i++) {
			CF = result & 1;
			result = result >> 1;
		}
	} else {
		for(i=0;i<itn;i++) {
			CF = result & 1;
			result = result >> 1;
		}
	}
	if ( itn == 1 ) {
		OF = ( data >> ( bits - 1 ) ) & 1;
	}
	cpu_updflags( result, bits );
	return result;
}

dword cpu_sar(word data, byte itn, byte bits) {
	register int i;
	dword result = (dword)data;
	word n;
	MASK_ITN_IF_80186;
	if(bits==8) {
		for(i=0;i<itn;i++) {
			n = result;
			result = (result>>1) | (n&0x80);
			CF = n&1;
		}
	} else {
		for(i=0;i<itn;i++) {
			n = result;
			result = (result>>1) | (n&0x8000);
			CF = n&1;
		}
	}
	if ( itn == 1 ) {
		OF = 0;
	}
	cpu_updflags( result, bits );
	return result;
}

dword cpu_rol(word data, byte itn, byte bits) {
	register int i;
	dword result = (dword)data;
	MASK_ITN_IF_80186;
	if(bits==8) {
		for(i=0;i<itn;i++) {
			CF = (result>>7)&1;
			result = (result<<1) | CF;
		}
	} else {
		for(i=0;i<itn;i++) {
			CF = (result>>15)&1;
			result = (result<<1) | CF;
		}
	}
	if ( itn == 1 ) {
		OF = ( ( result >> ( bits - 1 ) ) & 1 ) ^ CF;
	}
	return result;
}

dword cpu_ror(word data, byte itn, byte bits) {
	register int i;
	dword result = (dword)data;
	MASK_ITN_IF_80186;
	if(bits==8) {
		for(i=0;i<itn;i++) {
			CF = result & 1;
			result = (result>>1) | (CF<<7);
		}
	} else {
		for(i=0;i<itn;i++) {
			CF = result & 1;
			result = (result>>1) | (CF<<15);
		}
	}
	if ( itn == 1 ) {
		OF = ( result >> ( bits - 1 ) )
		   ^ ( ( result >> ( bits - 2 ) & 1 ) );
	}
	return result;
}

dword cpu_rcl(word data, byte itn, byte bits) {
	register int i;
	dword result = (dword)data;
	word n;
	MASK_ITN_IF_80186;
	if(bits==8) {
		for(i=0;i<itn;i++) {
			n = result;
			result = ((result<<1)&0xFF) | CF;
			CF = (n>>7)&1;
		}
	} else {
		for(i=0;i<itn;i++) {
			n = result;
			result = ((result<<1)&0xFFFF) | CF;
			CF = (n>>15)&1;
		}
	}
	if ( itn == 1 ) {
		OF = ( result >> ( bits - 1 ) ) ^ CF;
	}
	return result;
}

dword cpu_rcr(word data, byte itn, byte bits) {
	register int i;
	dword result = (dword)data;
	word n;
	MASK_ITN_IF_80186;
	if(bits==8) {
		for(i=0;i<itn;i++) {
			n = result;
			result = (result>>1) | (CF<<7);
			CF = n&1;
		}
	} else {
		for(i=0;i<itn;i++) {
			n = result;
			result = (result>>1) | (CF<<15);
			CF = n&1;
		}
	}
	if ( itn == 1 ) {
		OF = ( result >> ( bits - 1 ) )
		   ^ ( ( result >> ( bits - 2 ) & 1 ) );
	}
	return result;
}

void
_NOT_RM8()
{
	byte *p = cpu_rmptr( cpu_rmbyte, 8 );
	*p = ~(*p);
}

void
_NOT_RM16()
{
	word *p = cpu_rmptr( cpu_rmbyte, 16 );
	*p = ~(*p);
}

void
_NEG_RM8()
{
	byte *p = cpu_rmptr( cpu_rmbyte, 8 );
	byte old = *p;
	*p = 0 - *p;
	CF = ( old != 0 );
	cpu_updflags(*p, 8);
	OF =	((
			((0)^(old)) &
			((0)^(*p))
			)>>(7))&1;
	cpu_setAF(*p, 0, old);
}

void
_NEG_RM16()
{
	word *p = cpu_rmptr( cpu_rmbyte, 16 );
	word old = *p;
	*p = -old;
	CF = ( old != 0 );
	cpu_updflags(*p, 16);
	OF =	((
			((0)^(old)) &
			((0)^(*p))
			)>>(15))&1;
	cpu_setAF(*p, 0, old);
}
