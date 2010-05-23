/* 8086/string.c
 * String operations
 *
 */

#include "vomit.h"

void _LODSB(VCpu* cpu)
{
    /* Load byte at CurSeg:SI into AL */
    cpu->regs.B.AL = vomit_cpu_memory_read8(cpu, *(cpu->CurrentSegment), cpu->regs.W.SI);

    /* Modify SI according to DF */
    if (cpu->getDF() == 0)
        ++cpu->regs.W.SI;
    else
        --cpu->regs.W.SI;
}

void _LODSW(VCpu* cpu)
{
    /* Load word at CurSeg:SI into AX */
    cpu->regs.W.AX = vomit_cpu_memory_read16(cpu, *(cpu->CurrentSegment), cpu->regs.W.SI);

    /* Modify SI according to DF */
    if (cpu->getDF() == 0)
        cpu->regs.W.SI += 2;
    else
        cpu->regs.W.SI -= 2;
}

void _STOSB(VCpu* cpu)
{
    vomit_cpu_memory_write8(cpu, cpu->ES, cpu->regs.W.DI, cpu->regs.B.AL);

    if (cpu->getDF() == 0)
        ++cpu->regs.W.DI;
    else
        --cpu->regs.W.DI;
}

void _STOSW(VCpu* cpu)
{
    vomit_cpu_memory_write16(cpu, cpu->ES, cpu->regs.W.DI, cpu->regs.W.AX);

    if (cpu->getDF() == 0)
        cpu->regs.W.DI += 2;
    else
        cpu->regs.W.DI -= 2;
}

void _CMPSB(VCpu* cpu)
{
    BYTE src = vomit_cpu_memory_read8(cpu, *(cpu->CurrentSegment), cpu->regs.W.SI);
    BYTE dest = vomit_cpu_memory_read8(cpu, cpu->ES, cpu->regs.W.DI);

    cpu->cmpFlags8(src - dest, src, dest);

    if (cpu->getDF() == 0)
        ++cpu->regs.W.DI, ++cpu->regs.W.SI;
    else
        --cpu->regs.W.DI, --cpu->regs.W.SI;
}

void _CMPSW(VCpu* cpu)
{
    WORD src = vomit_cpu_memory_read16(cpu, *(cpu->CurrentSegment), cpu->regs.W.SI);
    WORD dest = vomit_cpu_memory_read16(cpu, cpu->ES, cpu->regs.W.DI);

    cpu->cmpFlags16(src - dest, src, dest);

    if (cpu->getDF() == 0)
        cpu->regs.W.DI += 2, cpu->regs.W.SI += 2;
    else
        cpu->regs.W.DI -= 2, cpu->regs.W.SI -= 2;
}

void _SCASB(VCpu* cpu)
{
    BYTE dest = vomit_cpu_memory_read8(cpu, cpu->ES, cpu->regs.W.DI);

    cpu->cmpFlags8(cpu->regs.B.AL - dest, dest, cpu->regs.B.AL);

    if (cpu->getDF() == 0)
        ++cpu->regs.W.DI;
    else
        --cpu->regs.W.DI;
}

void _SCASW(VCpu* cpu)
{
    WORD dest = vomit_cpu_memory_read16(cpu, cpu->ES, cpu->regs.W.DI);

    cpu->cmpFlags16(cpu->regs.W.AX - dest, dest, cpu->regs.W.AX);

    if (cpu->getDF() == 0)
        cpu->regs.W.DI += 2;
    else
        cpu->regs.W.DI -= 2;
}

void _MOVSB(VCpu* cpu)
{
    BYTE tmpb = vomit_cpu_memory_read8(cpu, *(cpu->CurrentSegment), cpu->regs.W.SI);
    vomit_cpu_memory_write8(cpu, cpu->ES, cpu->regs.W.DI, tmpb);

    if (cpu->getDF() == 0)
        ++cpu->regs.W.SI, ++cpu->regs.W.DI;
    else
        --cpu->regs.W.SI, --cpu->regs.W.DI;
}

void _MOVSW(VCpu* cpu)
{
    WORD tmpw = vomit_cpu_memory_read16(cpu, *(cpu->CurrentSegment), cpu->regs.W.SI);
    vomit_cpu_memory_write16(cpu, cpu->ES, cpu->regs.W.DI, tmpw);

    if (cpu->getDF() == 0)
        cpu->regs.W.SI += 2, cpu->regs.W.DI += 2;
    else
        cpu->regs.W.SI -= 2, cpu->regs.W.DI -= 2;
}
