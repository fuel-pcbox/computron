/* 8086/interrupt.c
 * Interrupt instructions
 *
 */

#include "vomit.h"
#include "debug.h"
#include <stdio.h>

#ifdef VOMIT_DOS_ON_LINUX_IDLE_HACK
#include <sched.h>
#endif

extern void bios_interrupt10();

void _INT_imm8(vomit_cpu_t *cpu)
{
    BYTE isr_index = vomit_cpu_pfq_getbyte(cpu);
    vomit_cpu_isr_call(cpu, isr_index);
}

void _INT3(vomit_cpu_t *cpu)
{
    vomit_cpu_isr_call(cpu, 3);
}

void _INTO(vomit_cpu_t *cpu)
{
    /* XXX: I've never seen this used, so it's probably good to log it. */
    vlog(VM_ALERT, "INTO used, can you believe it?");

    if (cpu->getOF())
        vomit_cpu_isr_call(cpu, 4);
}

void _IRET(vomit_cpu_t *cpu)
{
    WORD nip = vomit_cpu_pop(cpu);
    WORD ncs = vomit_cpu_pop(cpu);
    vomit_cpu_jump(cpu, ncs, nip);
    cpu->setFlags(vomit_cpu_pop(cpu));
}

void vomit_cpu_isr_call(vomit_cpu_t *cpu, BYTE isr_index)
{
#ifdef VOMIT_DEBUG
    if (trapint)
        vlog(VM_PICMSG, "%04X:%04X Interrupt %02X,%02X trapped", cpu->base_CS, cpu->base_IP, isr_index, cpu->regs.B.AH);
#endif

#ifdef VOMIT_DEBUG
    if (isr_index == 0x06) {
        vlog(VM_CPUMSG, "Invalid opcode trap at %04X:%04X (%02X)", cpu->base_CS, cpu->base_IP, cpu->code_memory[cpu->base_IP]);
    }
#endif

#ifdef VOMIT_DOS_ON_LINUX_IDLE_HACK
    if (isr_index == 0x28) {
        /* DOS idle interrupt, catch a quick rest! */
        sched_yield();
    }
#endif

    if (isr_index == 0x10) {
        /* Call C video BIOS (faster and more complete ATM) */
        bios_interrupt10();
        return;
    }

    vomit_cpu_push(cpu, cpu->getFlags());
    cpu->setIF(0);
    cpu->setTF(0);
    vomit_cpu_push(cpu, cpu->CS);
    vomit_cpu_push(cpu, cpu->IP);

    WORD segment = (cpu->memory[isr_index * 4 + 3] << 8) | cpu->memory[isr_index * 4 + 2];
    WORD offset = (cpu->memory[isr_index * 4 + 1] << 8) | cpu->memory[isr_index * 4];

    vomit_cpu_jump(cpu, segment, offset);
}
