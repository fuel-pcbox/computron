// x86/stack.cpp
// Stack instructions

#include "vcpu.h"
#include "debug.h"

void VCpu::push32(DWORD value)
{
    if (a16()) {
        this->regs.W.SP -= 4;
        writeMemory32(getSS(), this->getSP(), value);
    } else {
        this->regs.D.ESP -= 4;
        writeMemory32(getSS(), this->getESP(), value);
    }
}

void VCpu::push(WORD value)
{
    if (a16()) {
        this->regs.W.SP -= 2;
        writeMemory16(getSS(), this->getSP(), value);
    } else {
        this->regs.D.ESP -= 2;
        writeMemory16(getSS(), this->getESP(), value);
    }
}

DWORD VCpu::pop32()
{
    DWORD d;
    if (a16()) {
        d = readMemory32(getSS(), this->getSP());
        this->regs.W.SP += 4;
    } else {
        d = readMemory32(getSS(), this->getESP());
        this->regs.D.ESP += 4;
    }
    return d;
}

WORD VCpu::pop()
{
    WORD w;
    if (a16()) {
        w = readMemory16(getSS(), this->getSP());
        this->regs.W.SP += 2;
    } else {
        w = readMemory16(getSS(), this->getESP());
        this->regs.D.ESP += 2;
    }
    return w;
}

void _PUSH_AX(VCpu* cpu)
{
    cpu->push(cpu->getAX());
}

void _PUSH_BX(VCpu* cpu)
{
    cpu->push(cpu->getBX());
}

void _PUSH_CX(VCpu* cpu)
{
    cpu->push(cpu->getCX());
}

void _PUSH_DX(VCpu* cpu)
{
    cpu->push(cpu->getDX());
}

void _PUSH_BP(VCpu* cpu)
{
    cpu->push(cpu->getBP());
}

void _PUSH_SP(VCpu* cpu)
{
    cpu->push(cpu->getSP());
}

void _PUSH_SI(VCpu* cpu)
{
    cpu->push(cpu->getSI());
}

void _PUSH_DI(VCpu* cpu)
{
    cpu->push(cpu->getDI());
}

void _POP_AX(VCpu* cpu)
{
    cpu->regs.W.AX = cpu->pop();
}

void _POP_BX(VCpu* cpu)
{
    cpu->regs.W.BX = cpu->pop();
}

void _POP_EBX(VCpu* cpu)
{
    cpu->regs.D.EBX = cpu->pop32();
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
    cpu->push(cpu->readModRM16(cpu->rmbyte));
}

void _POP_RM16(VCpu* cpu)
{
    cpu->writeModRM16(cpu->rmbyte, cpu->pop());
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

void _PUSH_FS(VCpu* cpu)
{
    cpu->push(cpu->getFS());
}

void _PUSH_GS(VCpu* cpu)
{
    cpu->push(cpu->getGS());
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

void _POP_FS(VCpu* cpu)
{
    cpu->FS = cpu->pop();
}

void _POP_GS(VCpu* cpu)
{
    cpu->GS = cpu->pop();
}

void _PUSHFD(VCpu* cpu)
{
    cpu->push32(cpu->getEFlags());
}

void _PUSH_imm32(VCpu* cpu)
{
    cpu->push32(cpu->fetchOpcodeDWord());
}

void _PUSHF(VCpu* cpu)
{
    cpu->push(cpu->getFlags());
}

void _POPF(VCpu* cpu)
{
    cpu->setFlags(cpu->pop());
}

void _POPFD(VCpu* cpu)
{
    cpu->setEFlags(cpu->pop32());
}
