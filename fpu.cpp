/* fpu.cpp
 *
 * Actually, this file only contains the "ESCAPE" instruction, which handles
 * all FPU instructions by swallowing all ModR/M data and logging a little.
 *
 * Someday.
 *
 */

#include "vomit.h"
#include "debug.h"

void _ESCAPE(VCpu* cpu)
{
    vlog(VM_CPUMSG, "%04X:%04X FPU escape via %02X /%u",
        cpu->base_CS, cpu->base_IP,
        cpu->opcode, rmreg(cpu->readMemory8(cpu->getBaseCS(), cpu->getBaseIP() + 1)));

    //vm_exit(0);

    BYTE rm = cpu->fetchOpcodeByte();
    (void) vomit_cpu_modrm_read16(cpu, rm);

    return;

#if 0
    printf("Swallowed %d bytes: ", cpu->IP - cpu->base_IP);
    for (int i = 0; i < cpu.IP - cpu->base_IP; ++i)
        printf("%02X ", vomit_cpu_memory_read8(cpu, cpu->base_CS, cpu->base_IP + i));
    printf("\n");
#endif

#if 0
    /* 80286+: Coprocessor not available exception. */
    cpu->exception(7);
#endif
}
