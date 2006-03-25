/* 8086/string.c
 * String operations
 *
 */

#include "vomit.h"

void
_LODSB()
{
	/* Load byte at CurSeg:SI into AL */
	cpu.regs.B.AL = mem_getbyte( *(cpu.CurrentSegment), cpu.regs.W.SI );

	/* Modify SI according to DF */
	if( cpu.DF == 0 )
		++cpu.regs.W.SI;
	else
		--cpu.regs.W.SI;
}

void
_LODSW()
{
	/* Load word at CurSeg:SI into AX */
	cpu.regs.W.AX = mem_getword( *(cpu.CurrentSegment), cpu.regs.W.SI );

	/* Modify SI according to DF */
	if( cpu.DF == 0 )
		cpu.regs.W.SI += 2;
	else
		cpu.regs.W.SI -= 2;
}

void
_STOSB()
{
	mem_setbyte( cpu.ES, cpu.regs.W.DI, cpu.regs.B.AL );

	if( cpu.DF == 0 )
		++cpu.regs.W.DI;
	else
		--cpu.regs.W.DI;
}

void
_STOSW()
{
	mem_setword( cpu.ES, cpu.regs.W.DI, cpu.regs.W.AX );

	if( cpu.DF == 0 )
		cpu.regs.W.DI += 2;
	else
		cpu.regs.W.DI -= 2;
}

void
_CMPSB()
{
	byte src = mem_getbyte( *(cpu.CurrentSegment), cpu.regs.W.SI );
	byte dest = mem_getbyte( cpu.ES, cpu.regs.W.DI );
	cpu_cmpflags( src - dest, src, dest, 8 );
	if( cpu.DF == 0 )
		++cpu.regs.W.DI, ++cpu.regs.W.SI;
	else
		--cpu.regs.W.DI, --cpu.regs.W.SI;
}

void
_CMPSW()
{
	word src = mem_getword( *(cpu.CurrentSegment), cpu.regs.W.SI );
	word dest = mem_getword( cpu.ES, cpu.regs.W.DI );
	cpu_cmpflags( src - dest, src, dest, 16 );
	if( cpu.DF == 0 )
		cpu.regs.W.DI += 2, cpu.regs.W.SI += 2;
	else
		cpu.regs.W.DI -= 2, cpu.regs.W.SI -= 2;
}

void
_SCASB()
{
	byte dest = mem_getbyte( cpu.ES, cpu.regs.W.DI );
	cpu_cmpflags( cpu.regs.B.AL - dest, dest, cpu.regs.B.AL, 8 );

	if( cpu.DF == 0 )
		++cpu.regs.W.DI;
	else
		--cpu.regs.W.DI;
}

void
_SCASW()
{
	word dest = mem_getword( cpu.ES, cpu.regs.W.DI );
	cpu_cmpflags( cpu.regs.W.AX - dest, dest, cpu.regs.W.AX, 16 );

	if( cpu.DF == 0 )
		cpu.regs.W.DI += 2;
	else
		cpu.regs.W.DI -= 2;
}

void
_MOVSB()
{
	byte tmpb = mem_getbyte( *(cpu.CurrentSegment), cpu.regs.W.SI );
	mem_setbyte( cpu.ES, cpu.regs.W.DI, tmpb );

	if( cpu.DF == 0 )
		++cpu.regs.W.SI, ++cpu.regs.W.DI;
	else
		--cpu.regs.W.SI, --cpu.regs.W.DI;
}
void
_MOVSW()
{
	word tmpw = mem_getword( *(cpu.CurrentSegment), cpu.regs.W.SI );
	mem_setword( cpu.ES, cpu.regs.W.DI, tmpw );

	if( cpu.DF == 0 )
		cpu.regs.W.SI += 2, cpu.regs.W.DI += 2;
	else
		cpu.regs.W.SI -= 2, cpu.regs.W.DI -= 2;
}
