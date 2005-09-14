/* 8086/loop.c
 * Loop instructions
 *
 *
 */

#include "vomit.h"

void
_LOOP_imm8()
{
	byte disp = cpu_pfq_getbyte();
	CX--;
	if (CX != 0)
		cpu_jump(CS, IP+(sigbyte)disp);
}

void
_LOOPE_imm8()						/* both LOOPE and LOOPZ     */
{
    byte disp = cpu_pfq_getbyte();
	CX--;
    if ( ( CX != 0 ) && ZF )
		cpu_jump( CS, IP + (sigbyte)disp );
    return;
}

void
_LOOPNE_imm8()						/* both LOOPNE and LOOPNZ   */
{
    byte disp = cpu_pfq_getbyte();
	CX--;
    if ( ( CX != 0 ) && !ZF )
		cpu_jump( CS, IP + (sigbyte)disp );
}

void
_REP()
{
	byte rop = cpu_pfq_getbyte();
	word *segpfx = CurrentSegment;

	if((rop==0x26)||(rop==0x2E)||(rop==0x36)||(rop==0x3E)) {
		cpu_optable[rop]();
		segpfx = CurrentSegment;
		rop = cpu_pfq_getbyte();
	}

	/* If not CMPS or SCAS -- REP, else REPE */
	if ((rop!=0xA6)&&(rop!=0xA7)&&(rop!=0xAE)&&(rop!=0xAF)) {
		while(CX != 0) {	/* REP	*/
			CurrentSegment = segpfx;
			cpu_optable[rop]();
			--CX;
		}
	} else {
		while ( CX!=0 ) {	/* REPE/REPZ	*/
			CurrentSegment = segpfx;
			cpu_optable[rop]();
			--CX;
			if ( !ZF )
				break;
		}
	}
}

void
_REPNE()
{
	byte rop = cpu_pfq_getbyte();
	word *segpfx = CurrentSegment;
	if((rop==0x26)||(rop==0x2E)||(rop==0x36)||(rop==0x3E)) {
		cpu_optable[rop]();
		segpfx = CurrentSegment;
		rop = cpu_pfq_getbyte();
	}
	while ( CX != 0 ) {
		CurrentSegment = segpfx;
		cpu_optable[rop]();
		--CX;
		if ( ZF )
			break;
	}
}
