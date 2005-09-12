/* 8086/string.c
 * String operations
 *
 */

#include "vomit.h"

void
_LODSB() {
	*treg8[REG_AL] = mem_getbyte(*CurrentSegment, SI);	/* Load byte at CurSeg:SI into AL */
	if(DF==0)											/* Increment or decrement SI  */
		++SI;											/* depending on the value of  */
	else												/* the Direction Flag.        */
		--SI;
	return;
}

void
_LODSW() {
	AX = mem_getword(*CurrentSegment, SI);				/* Load word at CurSeg:SI into AX */
	if(DF==0)											/* Increment or decrement SI, */
		SI+=2;											/* etc, etc...                */
	else
		SI-=2;
	return;
}

void
_STOSB() {
	mem_setbyte(ES, DI, *treg8[REG_AL]);
	if(DF==0)
		++DI;
	else
		--DI;
	return;
}

void
_STOSW() {
	mem_setword(ES, DI, AX);
	if(DF==0)
		DI += 2;
	else
		DI -= 2;
	return;
}

void
_CMPSB() {
	byte src = mem_getbyte(*CurrentSegment, SI);
	byte dest = mem_getbyte(ES, DI);
	cpu_cmpflags(src-dest, src, dest, 8);
	if(DF==0) {++DI;++SI;} else {--DI;--SI;}
	return;
}

void
_CMPSW() {
	word src = mem_getword(*CurrentSegment, SI);
	word dest = mem_getword(ES, DI);
	cpu_cmpflags(src-dest, src, dest, 16);
	if(DF==0) {DI+=2;SI+=2;} else {DI-=2;SI-=2;}
	return;
}

void
_SCASB() {
	byte dest = mem_getbyte(ES, DI);
	cpu_cmpflags(*treg8[REG_AL]-dest, dest, *treg8[REG_AL], 8);
	if(DF==0) {++DI;} else {--DI;}
	return;
}

void
_SCASW() {
	word dest = mem_getword(ES, DI);
	cpu_cmpflags(AX-dest, dest, AX, 16);
	if(DF==0) {DI+=2;} else {DI-=2;}
	return;
}

void
_MOVSB() {
	byte tmpb = mem_getbyte(*CurrentSegment, SI);
	mem_setbyte(ES, DI, tmpb);
	if(DF==0) { ++SI; ++DI; } else { --SI; --DI; }
}
void
_MOVSW() {
	word tmpw = mem_getword(*CurrentSegment, SI);
	mem_setword(ES, DI, tmpw);
	if(DF==0) { SI+=2; DI+=2; } else { SI-=2; DI-=2; }
}

