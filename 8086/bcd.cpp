/* 8086/bcd.cpp
 * BCD instruction handlers
 *
 * Based entirely on the Intel IA32 manual.
 *
 */

#include "vomit.h"

void _AAA(vomit_cpu_t *cpu)
{
    if (((cpu->regs.B.AL & 0x0F)>9) || cpu->AF) {
        cpu->regs.B.AL += 6;
        cpu->regs.B.AH += 1;
        cpu->AF = 1;
        cpu->CF = 1;
    } else {
        cpu->AF = 0;
        cpu->CF = 0;
    }
    cpu->regs.B.AL &= 0x0F;
}

void _AAM(vomit_cpu_t *cpu)
{
    BYTE imm = vomit_cpu_pfq_getbyte(cpu);

    if (imm == 0) {
        /* Exceptions return to offending IP. */
        --cpu->IP;
        vomit_cpu_isr_call(cpu, 0);
        return;
    }

    BYTE tempAL = cpu->regs.B.AL;
    cpu->regs.B.AH = tempAL / imm;
    cpu->regs.B.AL = tempAL % imm;
    vomit_cpu_update_flags8(cpu, cpu->regs.B.AL);
}

void _AAD(vomit_cpu_t *cpu)
{
    BYTE tempAL = cpu->regs.B.AL;
    BYTE tempAH = cpu->regs.B.AH;
    BYTE imm = vomit_cpu_pfq_getbyte(cpu);

    cpu->regs.B.AL = (tempAL + (tempAH * imm)) & 0xFF;
    cpu->regs.B.AH = 0x00;
    vomit_cpu_update_flags8(cpu, cpu->regs.B.AL);
}

void _AAS(vomit_cpu_t *cpu)
{
    if (((cpu->regs.B.AL & 0x0F) > 9) || cpu->AF) {
        cpu->regs.B.AL -= 6;
        cpu->regs.B.AH -= 1;
        cpu->AF = 1;
        cpu->CF = 1;
    } else {
        cpu->AF = 0;
        cpu->CF = 0;
    }
}

void _DAS(vomit_cpu_t *cpu)
{
    bool oldCF = cpu->CF;
    BYTE oldAL = cpu->regs.B.AL;

    cpu->CF = 0;

    if (((cpu->regs.B.AL & 0x0F) > 0x09) || cpu->AF) {
        cpu->CF = ((cpu->regs.B.AL - 6) >> 8) & 1;
        cpu->regs.B.AL -= 0x06;
        cpu->CF = oldCF | cpu->CF;
        cpu->AF = 1;
    } else {
        cpu->AF = 0;
    }

    if (oldAL > 0x99 || oldCF == 1) {
        cpu->regs.B.AL -= 0x60;
        cpu->CF = 1;
    } else {
        cpu->CF = 0;
    }
}

void _DAA(vomit_cpu_t *cpu)
{
    bool oldCF = cpu->CF;
    BYTE oldAL = cpu->regs.B.AL;

    cpu->CF = 0;

    if (((cpu->regs.B.AL & 0x0F) > 0x09) || cpu->AF) {
        cpu->CF = ((cpu->regs.B.AL + 6) >> 8) & 1;
        cpu->regs.B.AL += 6;
        cpu->CF = oldCF | cpu->CF;
        cpu->AF = 1;
    } else {
        cpu->AF = 0;
    }

    if (oldAL > 0x99 || oldCF == 1) {
        cpu->regs.B.AL += 0x60;
        cpu->CF = 1;
    } else {
        cpu->CF = 0;
    }
}

