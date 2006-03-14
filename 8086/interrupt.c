/* 8086/interrupt.c
 * Interrupt instructions
 *
 */

#include "vomit.h"
#include <stdio.h>

void
int_init() {
	int i;
	for(i=0;i<0x100;i++)
		cpu_addint(i, 0x0000, 0x0000);
}

void
_INT_imm8() {
	byte imm = cpu_pfq_getbyte();
	int_call(imm);
}

void
_INT3() {
	int_call(0x03);
}

void
_INTO() {
	if(OF==1)
		int_call(0x04);
}

void
_IRET()
{
	word nip = mem_pop();
	word ncs = mem_pop();
	cpu_jump( ncs, nip );
	cpu_setflags( mem_pop() );
}

void
int_call( byte isr ) {
	#ifdef VM_DEBUG
		char tmp[40];
		if(trapint) {
			sprintf(tmp, "%04X:%04X cpu: Interrupt %02X,%02X trapped.\n", BCS, BIP, isr, *treg8[REG_AH]);
			vm_out(tmp, VM_LOGMSG);
		}
	#endif
	if( isr == 0x10 )
	{
		bios_interrupt10();
		return;
	}
	mem_push(cpu_getflags());
	IF = 0;
	TF = 0;
	mem_push(CS);
	mem_push(IP);
#ifdef VM_DEBUG
	if(!mempeek) {
		cpu_jump(mem_getword(0, isr*4+2), mem_getword(0, isr*4));
	} else {
		mempeek = 0;
#endif
		cpu_jump(mem_getword(0, isr*4+2), mem_getword(0, isr*4));
#ifdef VM_DEBUG
		mempeek = 1;
	}
#endif
}
