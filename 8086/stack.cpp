/* 8086/stack.cpp
 * Stack instructions
 *
 */

#include "vomit.h"
#include "debug.h"

void _PUSH_reg16(vomit_cpu_t *cpu)
{
    /* PUSH SP will use the value AFTER the push on Intel 8086. */
    if (cpu->type() == VCpu::Intel8086 && (cpu->opcode & 7) == REG_SP) {
        cpu->push(cpu->regs.W.SP + 2);
    } else {
        cpu->push(*cpu->treg16[cpu->opcode & 7]);
    }
}

void
_POP_reg16(vomit_cpu_t *cpu)
{
    *cpu->treg16[cpu->opcode & 7] = cpu->pop();
}

void _PUSH_RM16(vomit_cpu_t *cpu)
{
    cpu->push(vomit_cpu_modrm_read16(cpu, cpu->rmbyte));
}

void _POP_RM16(vomit_cpu_t *cpu)
{
    vomit_cpu_modrm_write16(cpu, cpu->rmbyte, cpu->pop());
}

void _PUSH_seg(vomit_cpu_t *cpu)
{
    cpu->push(*cpu->tseg[rmreg(cpu->opcode)]);
}

void _POP_CS(vomit_cpu_t *cpu)
{
    vlog(VM_ALERT, "%04X:%04X: 286+ instruction (or possibly POP CS...)", cpu->base_CS, cpu->base_IP);

    (void) cpu->fetchOpcodeByte();
    (void) cpu->fetchOpcodeByte();
    (void) cpu->fetchOpcodeByte();
    (void) cpu->fetchOpcodeByte();
}
void _POP_seg(vomit_cpu_t *cpu)
{
    *cpu->tseg[rmreg(cpu->opcode)] = cpu->pop();
}

void _PUSHF(vomit_cpu_t *cpu)
{
    cpu->push(cpu->getFlags());
}

void _POPF(vomit_cpu_t *cpu)
{
    cpu->setFlags(cpu->pop());
}
