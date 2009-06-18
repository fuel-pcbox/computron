/* 8086/loop.c
 * Loop instructions
 *
 *
 */

#include "vomit.h"
#include "debug.h"

void
_LOOP_imm8()
{
	sigbyte disp = cpu_pfq_getbyte();
	cpu.regs.W.CX--;
	if( cpu.regs.W.CX )
	{
		cpu_jump_relative8( disp );
	}
}

void
_LOOPE_imm8()						/* both LOOPE and LOOPZ     */
{
    sigbyte disp = cpu_pfq_getbyte();
	cpu.regs.W.CX--;
    if( cpu.regs.W.CX && cpu.ZF )
	{
		cpu_jump_relative8( disp );
	}
}

void
_LOOPNE_imm8()						/* both LOOPNE and LOOPNZ   */
{
    sigbyte disp = cpu_pfq_getbyte();
	cpu.regs.W.CX--;
    if ( cpu.regs.W.CX && !cpu.ZF ) {
		cpu_jump_relative8( disp );
	}
}

#define DO_REP( func ) for( ; cpu.regs.W.CX; --cpu.regs.W.CX ) { func (); }
#define DO_REPZ( func ) for( cpu.ZF = should_equal; cpu.regs.W.CX && (cpu.ZF == should_equal ); --cpu.regs.W.CX ) { func (); }

static void
__rep( byte opcode, bool should_equal )
{
	switch( opcode )
	{
		case 0x26: cpu.SegmentPrefix = cpu.ES; cpu.CurrentSegment = &cpu.SegmentPrefix; break;
		case 0x2E: cpu.SegmentPrefix = cpu.CS; cpu.CurrentSegment = &cpu.SegmentPrefix; break;
		case 0x36: cpu.SegmentPrefix = cpu.SS; cpu.CurrentSegment = &cpu.SegmentPrefix; break;
		case 0x3E: cpu.SegmentPrefix = cpu.DS; cpu.CurrentSegment = &cpu.SegmentPrefix; break;

		case 0xA4: DO_REP( _MOVSB ); return;
		case 0xA5: DO_REP( _MOVSW ); return;
		case 0xAA: DO_REP( _STOSB ); return;
		case 0xAB: DO_REP( _STOSW ); return;
		case 0xAC: DO_REP( _LODSB ); return;
		case 0xAD: DO_REP( _LODSW ); return;

		case 0xA6: DO_REPZ( _CMPSB ); return;
		case 0xA7: DO_REPZ( _CMPSW ); return;
		case 0xAE: DO_REPZ( _SCASB ); return;
		case 0xAF: DO_REPZ( _SCASW ); return;

		default:
			vlog( VM_ALERT, "SUSPICIOUS: Opcode %02X used with REP* prefix", opcode );
			cpu_optable[opcode]();
			return;
	}

	/* Recurse if this opcode was a segment prefix. */
	/* FIXME: Infinite recursion IS possible here. */
	__rep( cpu_pfq_getbyte(), should_equal );
}

void
_REP()
{
	word *old_segment = cpu.CurrentSegment;
	__rep( cpu_pfq_getbyte(), true );
	cpu.CurrentSegment = old_segment;
}

void
_REPNE()
{
	word *old_segment = cpu.CurrentSegment;
	__rep( cpu_pfq_getbyte(), false );
	cpu.CurrentSegment = old_segment;
}
