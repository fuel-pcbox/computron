/* 186.cpp
 * 80186 instructions.
 * Mostly interpreted from the ever loveable IA32 manual.
 *
 */

#include "vomit.h"
#include "debug.h"

void _wrap_0x0F(vomit_cpu_t *cpu)
{
    BYTE op = vomit_cpu_pfq_getbyte(cpu);
    switch (op) {
    case 0x01:
    {
        BYTE rm = vomit_cpu_pfq_getbyte(cpu);
        (void) vomit_cpu_modrm_read16(cpu, rm);
        vlog(VM_ALERT, "Sliding by 0F 01 /%d\n", rmreg(rm));
        break;
    }
    case 0xFF:		/* UD0 */
    case 0xB9:		/* UD1 */
    case 0x0B:		/* UD2 */
    default:
        vlog(VM_ALERT, "Undefinded opcode 0F %02X", op);
        cpu->exception(6);
        break;
    }
}

void _BOUND(vomit_cpu_t *cpu)
{
    BYTE rm = vomit_cpu_pfq_getbyte(cpu);
    DWORD value = vomit_cpu_modrm_read32(cpu, rm);
    WORD index = *cpu->treg16[rmreg(rm)];

    if (index < LSW(value) || index > MSW(value)) {
        /* Raise BR exception */
        cpu->exception(5);
    }
}

void _PUSH_imm8(vomit_cpu_t *cpu)
{
    vomit_cpu_push(cpu, signext(vomit_cpu_pfq_getbyte(cpu)));
}

void _PUSH_imm16(vomit_cpu_t *cpu)
{
    vomit_cpu_push(cpu, vomit_cpu_pfq_getword(cpu));
}

void _ENTER(vomit_cpu_t *cpu)
{
    WORD Size = vomit_cpu_pfq_getword(cpu);
    BYTE NestingLevel = vomit_cpu_pfq_getbyte(cpu) % 32;
    WORD FrameTemp;
    vomit_cpu_push(cpu, cpu->regs.W.BP);
    FrameTemp = cpu->regs.W.SP;
    if (NestingLevel != 0) {
        for (WORD i = 1; i <= (NestingLevel - 1); ++i) {
            cpu->regs.W.BP -= 2;
            vomit_cpu_push(cpu, vomit_cpu_memory_read16(cpu, cpu->SS, cpu->regs.W.BP));
        }
    }
    vomit_cpu_push(cpu, FrameTemp);
    cpu->regs.W.BP = FrameTemp;
    cpu->regs.W.SP = cpu->regs.W.BP - Size;
}

void _LEAVE(vomit_cpu_t *cpu)
{
    cpu->regs.W.SP = cpu->regs.W.BP;
    cpu->regs.W.BP = vomit_cpu_pop(cpu);
}

void _PUSHA(vomit_cpu_t *cpu)
{
    WORD oldsp = cpu->regs.W.SP;
    vomit_cpu_push(cpu, cpu->regs.W.AX);
    vomit_cpu_push(cpu, cpu->regs.W.BX);
    vomit_cpu_push(cpu, cpu->regs.W.CX);
    vomit_cpu_push(cpu, cpu->regs.W.DX);
    vomit_cpu_push(cpu, cpu->regs.W.BP);
    vomit_cpu_push(cpu, oldsp);
    vomit_cpu_push(cpu, cpu->regs.W.SI);
    vomit_cpu_push(cpu, cpu->regs.W.DI);
}

void _POPA(vomit_cpu_t *cpu)
{
    cpu->regs.W.DI = vomit_cpu_pop(cpu);
    cpu->regs.W.SI = vomit_cpu_pop(cpu);
    (void) vomit_cpu_pop(cpu);
    cpu->regs.W.BP = vomit_cpu_pop(cpu);
    cpu->regs.W.DX = vomit_cpu_pop(cpu);
    cpu->regs.W.CX = vomit_cpu_pop(cpu);
    cpu->regs.W.BX = vomit_cpu_pop(cpu);
    cpu->regs.W.AX = vomit_cpu_pop(cpu);
}
