// x86/stack.cpp
// Stack instructions

#include "vcpu.h"
#include "debug.h"

void VCpu::push(WORD value)
{
    this->regs.W.SP -= 2;
    writeMemory16(getSS(), this->regs.W.SP, value);
}

WORD VCpu::pop()
{
    WORD w = readMemory16(getSS(), this->regs.W.SP);
    this->regs.W.SP += 2;
    return w;
}

void _PUSH_SP_8086_80186(VCpu* cpu)
{
    // PUSH SP will use the value of SP *after* pushing on Intel's 8086 and 80186.
    cpu->push(cpu->regs.W.SP - 2);
}

void _PUSH_AX(VCpu* cpu)
{
    cpu->push(cpu->regs.W.AX);
}

void _PUSH_BX(VCpu* cpu)
{
    cpu->push(cpu->regs.W.BX);
}

void _PUSH_CX(VCpu* cpu)
{
    cpu->push(cpu->regs.W.CX);
}

void _PUSH_DX(VCpu* cpu)
{
    cpu->push(cpu->regs.W.DX);
}

void _PUSH_BP(VCpu* cpu)
{
    cpu->push(cpu->regs.W.BP);
}

void _PUSH_SP(VCpu* cpu)
{
    cpu->push(cpu->regs.W.SP);
}

void _PUSH_SI(VCpu* cpu)
{
    cpu->push(cpu->regs.W.SI);
}

void _PUSH_DI(VCpu* cpu)
{
    cpu->push(cpu->regs.W.DI);
}

void _POP_AX(VCpu* cpu)
{
    cpu->regs.W.AX = cpu->pop();
}

void _POP_BX(VCpu* cpu)
{
    cpu->regs.W.BX = cpu->pop();
}

void _POP_CX(VCpu* cpu)
{
    cpu->regs.W.CX = cpu->pop();
}

void _POP_DX(VCpu* cpu)
{
    cpu->regs.W.DX = cpu->pop();
}

void _POP_BP(VCpu* cpu)
{
    cpu->regs.W.BP = cpu->pop();
}

void _POP_SP(VCpu* cpu)
{
    cpu->regs.W.SP = cpu->pop();
}

void _POP_SI(VCpu* cpu)
{
    cpu->regs.W.SI = cpu->pop();
}

void _POP_DI(VCpu* cpu)
{
    cpu->regs.W.DI = cpu->pop();
}

void _PUSH_RM16(VCpu* cpu)
{
    cpu->push(vomit_cpu_modrm_read16(cpu, cpu->rmbyte));
}

void _POP_RM16(VCpu* cpu)
{
    vomit_cpu_modrm_write16(cpu, cpu->rmbyte, cpu->pop());
}

void _PUSH_CS(VCpu* cpu)
{
    cpu->push(cpu->getCS());
}

void _PUSH_DS(VCpu* cpu)
{
    cpu->push(cpu->getDS());
}

void _PUSH_ES(VCpu* cpu)
{
    cpu->push(cpu->getES());
}

void _PUSH_SS(VCpu* cpu)
{
    cpu->push(cpu->getSS());
}

void _POP_CS(VCpu* cpu)
{
    vlog(VM_ALERT, "%04X:%04X: 286+ instruction (or possibly POP CS...)", cpu->getBaseCS(), cpu->getBaseIP());

    (void) cpu->fetchOpcodeByte();
    (void) cpu->fetchOpcodeByte();
    (void) cpu->fetchOpcodeByte();
    (void) cpu->fetchOpcodeByte();
}

void _POP_DS(VCpu* cpu)
{
    cpu->DS = cpu->pop();
}

void _POP_ES(VCpu* cpu)
{
    cpu->ES = cpu->pop();
}

void _POP_SS(VCpu* cpu)
{
    cpu->SS = cpu->pop();
}

void _PUSHF(VCpu* cpu)
{
    cpu->push(cpu->getFlags());
}

void _POPF(VCpu* cpu)
{
    cpu->setFlags(cpu->pop());
}
