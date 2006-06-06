/* 8086/stack.c
 * Stack instructions
 *
 */

#include "vomit.h"
#include "debug.h"

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
_PUSH_seg()
{
	mem_push( *tseg[rmreg(cpu_opcode)] );
}

void
_POP_CS()
{
	vlog( VM_ALERT, "%04X:%04X Attempted either POP CS or 286+ instruction.", cpu.base_CS, cpu.base_IP );
}
void
_POP_seg()
{
	*tseg[rmreg(cpu_opcode)] = mem_pop();
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
