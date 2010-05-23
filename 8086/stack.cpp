/* 8086/stack.cpp
 * Stack instructions
 *
 */

#include "vomit.h"
#include "debug.h"

void _PUSH_SP(VCpu* cpu)
{
    /* PUSH SP will use the value of SP *after* pushing on Intel's 8086 and 80186.
     * Since these are the only CPU's emulated by Vomit right now, we just
     * do things that way.
     */
    cpu->push(cpu->regs.W.SP - 2);
}

void _PUSH_reg16(VCpu* cpu)
{
    cpu->push(*cpu->treg16[cpu->opcode & 7]);
}

void
_POP_reg16(VCpu* cpu)
{
    *cpu->treg16[cpu->opcode & 7] = cpu->pop();
}

void _PUSH_RM16(VCpu* cpu)
{
    cpu->push(vomit_cpu_modrm_read16(cpu, cpu->rmbyte));
}

void _POP_RM16(VCpu* cpu)
{
    vomit_cpu_modrm_write16(cpu, cpu->rmbyte, cpu->pop());
}

void _PUSH_seg(VCpu* cpu)
{
    cpu->push(*cpu->tseg[rmreg(cpu->opcode)]);
}

void _POP_CS(VCpu* cpu)
{
    vlog(VM_ALERT, "%04X:%04X: 286+ instruction (or possibly POP CS...)", cpu->base_CS, cpu->base_IP);

    (void) cpu->fetchOpcodeByte();
    (void) cpu->fetchOpcodeByte();
    (void) cpu->fetchOpcodeByte();
    (void) cpu->fetchOpcodeByte();
}
void _POP_seg(VCpu* cpu)
{
    *cpu->tseg[rmreg(cpu->opcode)] = cpu->pop();
}

void _PUSHF(VCpu* cpu)
{
    cpu->push(cpu->getFlags());
}

void _POPF(VCpu* cpu)
{
    cpu->setFlags(cpu->pop());
}
