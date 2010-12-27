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

void _PUSH_EAX(VCpu *cpu)
{
    cpu->push32(cpu->getEAX());
}

void _PUSH_EBX(VCpu *cpu)
{
    cpu->push32(cpu->getEBX());
}

void _PUSH_ECX(VCpu *cpu)
{
    cpu->push32(cpu->getECX());
}

void _PUSH_EDX(VCpu *cpu)
{
    cpu->push32(cpu->getEDX());
}

void _PUSH_ESP(VCpu *cpu)
{
    cpu->push32(cpu->getESP());
}

void _PUSH_EBP(VCpu *cpu)
{
    cpu->push32(cpu->getEBP());
}

void _PUSH_ESI(VCpu *cpu)
{
    cpu->push32(cpu->getESI());
}

void _PUSH_EDI(VCpu *cpu)
{
    cpu->push32(cpu->getEDI());
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

void _POP_EAX(VCpu* cpu)
{
    cpu->regs.D.EAX = cpu->pop32();
}

void _POP_EBX(VCpu* cpu)
{
    cpu->regs.D.EBX = cpu->pop32();
}

void _POP_ECX(VCpu* cpu)
{
    cpu->regs.D.ECX = cpu->pop32();
}

void _POP_EDX(VCpu* cpu)
{
    cpu->regs.D.EDX = cpu->pop32();
}

void _POP_ESP(VCpu* cpu)
{
    cpu->regs.D.ESP = cpu->pop32();
}

void _POP_EBP(VCpu* cpu)
{
    cpu->regs.D.EBP = cpu->pop32();
}

void _POP_ESI(VCpu* cpu)
{
    cpu->regs.D.ESI = cpu->pop32();
}

void _POP_EDI(VCpu* cpu)
{
    cpu->regs.D.EDI = cpu->pop32();
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

void _POP_RM32(VCpu* cpu)
{
    cpu->writeModRM32(cpu->rmbyte, cpu->pop32());
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
    if (!cpu->getPE() || (cpu->getPE() && ((!cpu->getVM() || (cpu->getVM() && cpu->getIOPL() == 3)))))
        cpu->push32(cpu->getEFlags() & 0x00FCFFFF);
    else
        cpu->GP(0);
}

void _PUSH_imm32(VCpu* cpu)
{
    cpu->push32(cpu->fetchOpcodeDWord());
}

void _PUSHF(VCpu* cpu)
{
    if (!cpu->getPE() || (cpu->getPE() && ((!cpu->getVM() || (cpu->getVM() && cpu->getIOPL() == 3)))))
        cpu->push(cpu->getFlags());
    else
        cpu->GP(0);
}

void _POPF(VCpu* cpu)
{
    if (!cpu->getVM()) {
        if (cpu->getCPL() == 0)
            cpu->setFlags(cpu->pop());
        else {
            bool oldIOPL = cpu->getIOPL();
            cpu->setFlags(cpu->pop());
            cpu->setIOPL(oldIOPL);
        }
    } else {
        if (cpu->getIOPL() == 3) {
            bool oldIOPL = cpu->getIOPL();
            cpu->setFlags(cpu->pop());
            cpu->setIOPL(oldIOPL);
        } else
            cpu->GP(0);
    }
}

void _POPFD(VCpu* cpu)
{
    if (!cpu->getVM()) {
        if (cpu->getCPL() == 0)
            cpu->setEFlags(cpu->pop32());
        else {
            bool oldIOPL = cpu->getIOPL();
            cpu->setEFlags(cpu->pop32());
            cpu->setIOPL(oldIOPL);
        }
    } else {
        if (cpu->getIOPL() == 3) {
            bool oldIOPL = cpu->getIOPL();
            cpu->setEFlags(cpu->pop32());
            cpu->setIOPL(oldIOPL);
        } else
            cpu->GP(0);
    }
}
