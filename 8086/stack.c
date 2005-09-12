/* 8086/stack.c
 * Stack instructions
 *
 */

#include "vomit.h"
#include <stdio.h>

void _PUSH_reg16() { mem_push(*treg16[cpu_opcode&7]); }
void _POP_reg16() { *treg16[cpu_opcode&7]=mem_pop(); }
void
_PUSH_RM16() {
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	mem_push(*p);
}
void
_POP_RM16() {
	byte rm = cpu_rmbyte;
	word *p = cpu_rmptr(rm, 16);
	*p = mem_pop();
}

void _PUSH_CS() { mem_push(CS); }
void _PUSH_DS() { mem_push(DS); }
void _PUSH_ES() { mem_push(ES); }
void _PUSH_SS() { mem_push(SS); }

void
_POP_CS() {
	#ifdef VM_DEBUG
		printf("%04X:%04X ", BCS, BIP);
	#endif
	printf("\npanic: Attempted either POP CS or 286+ instruction.\n");
	vm_exit(1);
}
void _POP_DS() { DS = mem_pop(); }
void _POP_ES() { ES = mem_pop(); }
void _POP_SS() { SS = mem_pop(); }

void _PUSHF() { mem_push(cpu_getflags()); }
void _POPF() { cpu_setflags(mem_pop()); }

