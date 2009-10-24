/* 8086/string.c
 * String operations
 *
 */

#include "vomit.h"

void _LODSB(vomit_cpu_t *cpu)
{
    /* Load byte at CurSeg:SI into AL */
    cpu->regs.B.AL = vomit_cpu_memory_read8(cpu, *(cpu->CurrentSegment), cpu->regs.W.SI);

    /* Modify SI according to DF */
    if (cpu->getDF() == 0)
        ++cpu->regs.W.SI;
    else
        --cpu->regs.W.SI;
}

void _LODSW(vomit_cpu_t *cpu)
{
    /* Load word at CurSeg:SI into AX */
    cpu->regs.W.AX = vomit_cpu_memory_read16(cpu, *(cpu->CurrentSegment), cpu->regs.W.SI);

    /* Modify SI according to DF */
    if (cpu->getDF() == 0)
        cpu->regs.W.SI += 2;
    else
        cpu->regs.W.SI -= 2;
}

void _STOSB(vomit_cpu_t *cpu)
{
    vomit_cpu_memory_write8(cpu, cpu->ES, cpu->regs.W.DI, cpu->regs.B.AL);

    if (cpu->getDF() == 0)
        ++cpu->regs.W.DI;
    else
        --cpu->regs.W.DI;
}

void _STOSW(vomit_cpu_t *cpu)
{
    vomit_cpu_memory_write16(cpu, cpu->ES, cpu->regs.W.DI, cpu->regs.W.AX);

    if (cpu->getDF() == 0)
        cpu->regs.W.DI += 2;
    else
        cpu->regs.W.DI -= 2;
}

void _CMPSB(vomit_cpu_t *cpu)
{
    BYTE src = vomit_cpu_memory_read8(cpu, *(cpu->CurrentSegment), cpu->regs.W.SI);
    BYTE dest = vomit_cpu_memory_read8(cpu, cpu->ES, cpu->regs.W.DI);

    cpu->cmpFlags8(src - dest, src, dest);

    if (cpu->getDF() == 0)
        ++cpu->regs.W.DI, ++cpu->regs.W.SI;
    else
        --cpu->regs.W.DI, --cpu->regs.W.SI;
}

void _CMPSW(vomit_cpu_t *cpu)
{
    WORD src = vomit_cpu_memory_read16(cpu, *(cpu->CurrentSegment), cpu->regs.W.SI);
    WORD dest = vomit_cpu_memory_read16(cpu, cpu->ES, cpu->regs.W.DI);

    cpu->cmpFlags16(src - dest, src, dest);

    if (cpu->getDF() == 0)
        cpu->regs.W.DI += 2, cpu->regs.W.SI += 2;
    else
        cpu->regs.W.DI -= 2, cpu->regs.W.SI -= 2;
}

void _SCASB(vomit_cpu_t *cpu)
{
    BYTE dest = vomit_cpu_memory_read8(cpu, cpu->ES, cpu->regs.W.DI);

    cpu->cmpFlags8(cpu->regs.B.AL - dest, dest, cpu->regs.B.AL);

    if (cpu->getDF() == 0)
        ++cpu->regs.W.DI;
    else
        --cpu->regs.W.DI;
}

void _SCASW(vomit_cpu_t *cpu)
{
    WORD dest = vomit_cpu_memory_read16(cpu, cpu->ES, cpu->regs.W.DI);

    cpu->cmpFlags16(cpu->regs.W.AX - dest, dest, cpu->regs.W.AX);

    if (cpu->getDF() == 0)
        cpu->regs.W.DI += 2;
    else
        cpu->regs.W.DI -= 2;
}

void _MOVSB(vomit_cpu_t *cpu)
{
    BYTE tmpb = vomit_cpu_memory_read8(cpu, *(cpu->CurrentSegment), cpu->regs.W.SI);
    vomit_cpu_memory_write8(cpu, cpu->ES, cpu->regs.W.DI, tmpb);

    if (cpu->getDF() == 0)
        ++cpu->regs.W.SI, ++cpu->regs.W.DI;
    else
        --cpu->regs.W.SI, --cpu->regs.W.DI;
}

void _MOVSW(vomit_cpu_t *cpu)
{
    WORD tmpw = vomit_cpu_memory_read16(cpu, *(cpu->CurrentSegment), cpu->regs.W.SI);
    vomit_cpu_memory_write16(cpu, cpu->ES, cpu->regs.W.DI, tmpw);

    if (cpu->getDF() == 0)
        cpu->regs.W.SI += 2, cpu->regs.W.DI += 2;
    else
        cpu->regs.W.SI -= 2, cpu->regs.W.DI -= 2;
}
