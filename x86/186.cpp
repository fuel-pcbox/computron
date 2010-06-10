/* 186.cpp
 * 80186 instructions.
 * Mostly interpreted from the ever loveable IA32 manual.
 *
 */

#include "vomit.h"
#include "debug.h"

void _wrap_0x0F(VCpu* cpu)
{
    BYTE op = cpu->fetchOpcodeByte();
    switch (op) {
    case 0x01:
    {
        BYTE rm = cpu->fetchOpcodeByte();
        (void) cpu->readModRM16(rm);
        vlog(VM_ALERT, "Sliding by 0F 01 /%d\n", vomit_modRMRegisterPart(rm));
        break;
    }
    case 0xFF: // UD0
    case 0xB9: // UD1
    case 0x0B: // UD2
    default:
        vlog(VM_ALERT, "Undefinded opcode 0F %02X", op);
        cpu->exception(6);
        break;
    }
}

void _BOUND(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    DWORD value = cpu->readModRM32(rm);
    WORD index = *cpu->treg16[vomit_modRMRegisterPart(rm)];

    if (index < LSW(value) || index > MSW(value)) {
        /* Raise BR exception */
        cpu->exception(5);
    }
}

void _PUSH_imm8(VCpu* cpu)
{
    cpu->push(vomit_signExtend(cpu->fetchOpcodeByte()));
}

void _PUSH_imm16(VCpu* cpu)
{
    cpu->push(cpu->fetchOpcodeWord());
}

void _ENTER(VCpu* cpu)
{
    WORD Size = cpu->fetchOpcodeWord();
    BYTE NestingLevel = cpu->fetchOpcodeByte() % 32;
    WORD FrameTemp;
    cpu->push(cpu->regs.W.BP);
    FrameTemp = cpu->regs.W.SP;
    if (NestingLevel != 0) {
        for (WORD i = 1; i <= (NestingLevel - 1); ++i) {
            cpu->regs.W.BP -= 2;
            cpu->push(cpu->readMemory16(cpu->SS, cpu->regs.W.BP));
        }
    }
    cpu->push(FrameTemp);
    cpu->regs.W.BP = FrameTemp;
    cpu->regs.W.SP = cpu->regs.W.BP - Size;
}

void _LEAVE(VCpu* cpu)
{
    cpu->regs.W.SP = cpu->regs.W.BP;
    cpu->regs.W.BP = cpu->pop();
}

void _PUSHA(VCpu* cpu)
{
    WORD oldsp = cpu->regs.W.SP;
    cpu->push(cpu->regs.W.AX);
    cpu->push(cpu->regs.W.BX);
    cpu->push(cpu->regs.W.CX);
    cpu->push(cpu->regs.W.DX);
    cpu->push(cpu->regs.W.BP);
    cpu->push(oldsp);
    cpu->push(cpu->regs.W.SI);
    cpu->push(cpu->regs.W.DI);
}

void _POPA(VCpu* cpu)
{
    cpu->regs.W.DI = cpu->pop();
    cpu->regs.W.SI = cpu->pop();
    (void) cpu->pop();
    cpu->regs.W.BP = cpu->pop();
    cpu->regs.W.DX = cpu->pop();
    cpu->regs.W.CX = cpu->pop();
    cpu->regs.W.BX = cpu->pop();
    cpu->regs.W.AX = cpu->pop();
}
