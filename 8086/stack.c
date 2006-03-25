/* 8086/stack.c
 * Stack instructions
 *
 */

#include "vomit.h"

void
_PUSH_reg16()
{
	/* TODO: PUSH SP differs between 8086 and later processors. */
	mem_push( *treg16[cpu_opcode & 7] );
}

void
_POP_reg16()
{
	*treg16[cpu_opcode & 7] = mem_pop();
}

void
_PUSH_RM16()
{
	word *p = cpu_rmptr( cpu_rmbyte, 16 );
	mem_push( *p );
}

void
_POP_RM16()
{
	word *p = cpu_rmptr( cpu_rmbyte, 16 );
	*p = mem_pop();
}

void
_PUSH_CS()
{
	mem_push( cpu.CS );
}

void
_PUSH_DS()
{
	mem_push( cpu.DS );
}

void
_PUSH_ES()
{
	mem_push( cpu.ES );
}

void
_PUSH_SS()
{
	mem_push( cpu.SS );
}

void
_POP_CS()
{
	vlog( VM_ALERT, "%04X:%04X Attempted either POP CS or 286+ instruction.", BCS, BIP );
}
void
_POP_DS()
{
	cpu.DS = mem_pop();
}

void
_POP_ES()
{
	cpu.ES = mem_pop();
}

void
_POP_SS()
{
	cpu.SS = mem_pop();
}

void
_PUSHF()
{
	mem_push( cpu_getflags() );
}

void
_POPF()
{
	cpu_setflags( mem_pop() );
}
