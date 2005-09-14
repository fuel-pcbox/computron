/* 8086/loop.c
 * Loop instructions
 *
 *
 */

#include "vomit.h"

#ifdef VOMIT_NATIVE_REP_STUFF
#include <string.h>
#endif

void
_LOOP_imm8()
{
	byte disp = cpu_pfq_getbyte();
	CX--;
	if ( CX ) {
		cpu_jump( CS, IP + (sigbyte)disp );
	}
}

void
_LOOPE_imm8()						/* both LOOPE and LOOPZ     */
{
    byte disp = cpu_pfq_getbyte();
	CX--;
    if ( CX && ZF ) {
		cpu_jump( CS, IP + (sigbyte)disp );
	}
}

void
_LOOPNE_imm8()						/* both LOOPNE and LOOPNZ   */
{
    byte disp = cpu_pfq_getbyte();
	CX--;
    if ( CX && !ZF ) {
		cpu_jump( CS, IP + (sigbyte)disp );
	}
}

void
_REP()
{
	byte rop = cpu_pfq_getbyte();
	word *old_segment = CurrentSegment;
#ifdef VOMIT_NATIVE_REP_STUFF
	byte *bdest, *bsrc;
	word *wdest;
#endif

	/* If the following opcode is a segment override instruction,
	 * we override our "CurrentSegment" accordingly and use the
	 * next instruction for REPing. */

	if ( rop == 0x26 || rop == 0x2E || rop == 0x36 || rop == 0x3E ) {
		switch( rop ) {
			case 0x26: CurrentSegment = &ES; break;
			case 0x2E: CurrentSegment = &CS; break;
			case 0x36: CurrentSegment = &SS; break;
			case 0x3E: CurrentSegment = &DS; break;
		}
		rop = cpu_pfq_getbyte();
	}

#ifdef VOMIT_NATIVE_REP_STUFF

	/* Detect incremental REP STOS and REP MOVS and perform them
	 * natively (in C, at least) instead. */

	if ( DF == 0 ) {

		/* REP STOSB */
		if ( rop == 0xAA ) {
			bdest = mem_space + ( ES << 4 ) + DI;
			memset( bdest, *treg8[REG_AL], CX );
			DI += CX;
			CX = 0;
			return;
		}

		/* REP STOSW */
		else if ( rop == 0xAB ) {
			if ( *treg8[REG_AH] != *treg8[REG_AL] ) {
				wdest = (word *)(mem_space + ( ES << 4 ) + DI);
				DI += CX * 2;
				for( ; CX; --CX ) {
					*(wdest++) = AX;
				}
			} else {
				bdest = mem_space + ( ES << 4 ) + DI;
				memset( bdest, *treg8[REG_AL], CX * 2 );
				DI += CX * 2;
			}
			CX = 0;
			return;
		}

		/* REP MOVSB */
		else if ( rop == 0xA4 ) {
			bdest = mem_space + ( ES << 4 ) + DI;
			bsrc = mem_space + ( *CurrentSegment << 4 ) + SI;
			DI += CX;
			SI += CX;
			memcpy( bdest, bsrc, CX );
			CX = 0;
			return;
		}

		/* REP MOVSW */
		else if ( rop == 0xA5 ) {
			bdest = mem_space + ( ES << 4 ) + DI;
			bsrc = mem_space + ( *CurrentSegment << 4 ) + SI;
			DI += CX * 2;
			SI += CX * 2;
			memcpy( bdest, bsrc, CX * 2 );
			CX = 0;
			return;
		}
	}

#endif

	/* If not CMPS or SCAS -- REP, else REPE */
	if ( rop != 0xA6 && rop != 0xA7 && rop != 0xAE && rop != 0xAF ) {
		while ( CX ) {	/* REP	*/
			cpu_optable[rop]();
			--CX;
		}
	} else {
		/* REPE / REPZ */
		while ( CX ) {
			cpu_optable[rop]();
			--CX;
			if ( !ZF )
				break;
		}
	}
	CurrentSegment = old_segment;
}

void
_REPNE()
{
	byte rop = cpu_pfq_getbyte();
	word *old_segment = CurrentSegment;

	/* Same as above. */
	if ( rop == 0x26 || rop == 0x2E || rop == 0x36 || rop == 0x3E ) {
		switch( rop ) {
			case 0x26: CurrentSegment = &ES; break;
			case 0x2E: CurrentSegment = &CS; break;
			case 0x36: CurrentSegment = &SS; break;
			case 0x3E: CurrentSegment = &DS; break;
		}
		rop = cpu_pfq_getbyte();
	}

	while ( CX ) {
		cpu_optable[rop]();
		--CX;
		if ( ZF )
			break;
	}

	CurrentSegment = old_segment;
}
