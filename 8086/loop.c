/* 8086/loop.c
 * Loop instructions
 *
 *
 */

#include "vomit.h"

void
_LOOP_imm8()
{
	sigbyte disp = cpu_pfq_getbyte();
	CX--;
	if( CX )
	{
		cpu_jump_relative8( disp );
	}
}

void
_LOOPE_imm8()						/* both LOOPE and LOOPZ     */
{
    sigbyte disp = cpu_pfq_getbyte();
	CX--;
    if( CX && ZF )
	{
		cpu_jump_relative8( disp );
	}
}

void
_LOOPNE_imm8()						/* both LOOPNE and LOOPNZ   */
{
    sigbyte disp = cpu_pfq_getbyte();
	CX--;
    if ( CX && !ZF ) {
		cpu_jump_relative8( disp );
	}
}

void
_REP()
{
	byte rop = cpu_pfq_getbyte();
	word *old_segment = CurrentSegment;

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
