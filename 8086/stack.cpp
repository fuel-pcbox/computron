/* 8086/stack.c
 * Stack instructions
 *
 */

#include "vomit.h"
#include "debug.h"

void
_PUSH_reg16()
{
	/* PUSH SP will use the value AFTER the push on Intel 8086. */
	if( cpu.type == INTEL_8086 && (cpu.opcode & 7) == REG_SP )
	{
		mem_push( cpu.regs.W.SP + 2 );
	}
	else
	{
		mem_push( *treg16[cpu.opcode & 7] );
	}
}

void
_POP_reg16()
{
	*treg16[cpu.opcode & 7] = mem_pop();
}

void
_PUSH_RM16()
{
	mem_push( modrm_read16( cpu.rmbyte ));
}

void
_POP_RM16()
{
	modrm_write16( cpu.rmbyte, mem_pop() );
}

void
_PUSH_seg()
{
	mem_push( *tseg[rmreg(cpu.opcode)] );
}

void
_POP_CS()
{
	vlog( VM_ALERT, "%04X:%04X: 286+ instruction (or possibly POP CS...)", cpu.base_CS, cpu.base_IP );

	(void) cpu_pfq_getbyte();
	(void) cpu_pfq_getbyte();
	(void) cpu_pfq_getbyte();
	(void) cpu_pfq_getbyte();
}
void
_POP_seg()
{
	*tseg[rmreg(cpu.opcode)] = mem_pop();
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
