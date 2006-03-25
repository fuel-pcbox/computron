/* 8086/string.c
 * String operations
 *
 */

#include "vomit.h"

void
_LODSB()
{
	/* Load byte at CurSeg:SI into AL */
	cpu.regs.B.AL = mem_getbyte( *(cpu.CurrentSegment), cpu.SI );

	/* Modify SI according to DF */
	if( cpu.DF == 0 )
		++cpu.SI;
	else
		--cpu.SI;
}

void
_LODSW()
{
	/* Load word at CurSeg:SI into AX */
	cpu.regs.W.AX = mem_getword( *(cpu.CurrentSegment), cpu.SI );

	/* Modify SI according to DF */
	if( cpu.DF == 0 )
		cpu.SI += 2;
	else
		cpu.SI -= 2;
}

void
_STOSB()
{
	mem_setbyte( cpu.ES, cpu.DI, cpu.regs.B.AL );

	if( cpu.DF == 0 )
		++cpu.DI;
	else
		--cpu.DI;
}

void
_STOSW()
{
	mem_setword( cpu.ES, cpu.DI, cpu.regs.W.AX );

	if( cpu.DF == 0 )
		cpu.DI += 2;
	else
		cpu.DI -= 2;
}

void
_CMPSB()
{
	byte src = mem_getbyte( *(cpu.CurrentSegment), cpu.SI );
	byte dest = mem_getbyte( cpu.ES, cpu.DI );
	cpu_cmpflags( src - dest, src, dest, 8 );
	if( cpu.DF == 0 )
		++cpu.DI, ++cpu.SI;
	else
		--cpu.DI, --cpu.SI;
}

void
_CMPSW()
{
	word src = mem_getword( *(cpu.CurrentSegment), cpu.SI );
	word dest = mem_getword( cpu.ES, cpu.DI );
	cpu_cmpflags( src - dest, src, dest, 16 );
	if( cpu.DF == 0 )
		cpu.DI += 2, cpu.SI += 2;
	else
		cpu.DI -= 2, cpu.SI -= 2;
}

void
_SCASB()
{
	byte dest = mem_getbyte( cpu.ES, cpu.DI );
	cpu_cmpflags( cpu.regs.B.AL - dest, dest, cpu.regs.B.AL, 8 );

	if( cpu.DF == 0 )
		++cpu.DI;
	else
		--cpu.DI;
}

void
_SCASW()
{
	word dest = mem_getword( cpu.ES, cpu.DI );
	cpu_cmpflags( cpu.regs.W.AX - dest, dest, cpu.regs.W.AX, 16 );

	if( cpu.DF == 0 )
		cpu.DI += 2;
	else
		cpu.DI -= 2;
}

void
_MOVSB()
{
	byte tmpb = mem_getbyte( *(cpu.CurrentSegment), cpu.SI );
	mem_setbyte( cpu.ES, cpu.DI, tmpb );

	if( cpu.DF == 0 )
		++cpu.SI, ++cpu.DI;
	else
		--cpu.SI, --cpu.DI;
}
void
_MOVSW()
{
	word tmpw = mem_getword( *(cpu.CurrentSegment), cpu.SI );
	mem_setword( cpu.ES, cpu.DI, tmpw );

	if( cpu.DF == 0 )
		cpu.SI += 2, cpu.DI += 2;
	else
		cpu.SI -= 2, cpu.DI -= 2;
}
