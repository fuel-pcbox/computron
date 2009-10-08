/* 8086/flags.cpp
 * Handler of the hell that is flags
 */

#include "vomit.h"

static const byte parity_table[0x100] = {
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
};

void vomit_cpu_setAF(vomit_cpu_t *cpu, DWORD result, WORD src, WORD dest)
{
    cpu->AF = (((result ^ (src ^ dest)) & 0x10) >> 4) & 1;
}

WORD vomit_cpu_static_flags(vomit_cpu_t *cpu)
{
    switch( cpu->type ) {
    case INTEL_8086:
    case INTEL_80186:
        return 0xF002;
    default:
        return 0x0000;
    }
}

void vomit_cpu_update_flags16(vomit_cpu_t *cpu, WORD data)
{
    cpu->PF = parity_table[data & 0xFF];
    cpu->SF = (data & 0x8000) != 0;
    cpu->ZF = data == 0;
}

void vomit_cpu_update_flags8(vomit_cpu_t *cpu, BYTE data)
{
    cpu->PF = parity_table[data];
    cpu->SF = (data & 0x80) != 0;
    cpu->ZF = data == 0;
}

void vomit_cpu_update_flags(vomit_cpu_t *cpu, WORD data, BYTE bits)
{
    if (bits == 8) {
        data &= 0xFF;
        cpu->PF = parity_table[data];
        cpu->SF = (data & 0x80) != 0;
    } else {
        cpu->PF = parity_table[data & 0xFF];
        cpu->SF = (data & 0x8000) != 0;
    }
    cpu->ZF = data == 0;
}

void _STC(vomit_cpu_t *cpu)
{
    cpu->CF = 1;
}

void _STD(vomit_cpu_t *cpu)
{
    cpu->DF = 1;
}

void _STI(vomit_cpu_t *cpu)
{
    cpu->IF = 1;
}

void _CLI(vomit_cpu_t *cpu)
{
    cpu->IF = 0;
}

void _CLC(vomit_cpu_t *cpu)
{
    cpu->CF = 0;
}

void _CLD(vomit_cpu_t *cpu)
{
    cpu->DF = 0;
}

void _CMC(vomit_cpu_t *cpu)
{
    cpu->CF = !cpu->CF;
}

void vomit_cpu_math_flags8(vomit_cpu_t *cpu, DWORD result, BYTE dest, BYTE src)
{
    cpu->CF = (result & 0x0100) != 0;
    cpu->SF = (result & 0x0080) != 0;
    cpu->ZF = (result & 0x00FF) == 0;
    cpu->PF = parity_table[result & 0xFF];
    vomit_cpu_setAF(cpu, result, dest, src);
}

void vomit_cpu_math_flags16(vomit_cpu_t *cpu, DWORD result, WORD dest, WORD src)
{
    cpu->CF = (result & 0x10000) != 0;
    cpu->SF = (result &  0x8000) != 0;
    cpu->ZF = (result &  0xFFFF) == 0;
    cpu->PF = parity_table[result & 0xFF];
    vomit_cpu_setAF(cpu, result, dest, src);
}

void vomit_cpu_cmp_flags8(vomit_cpu_t *cpu, DWORD result, BYTE dest, BYTE src)
{
    vomit_cpu_math_flags8(cpu, result, dest, src);
    cpu->OF = ((
        ((src)^(dest)) &
        ((src)^(src-dest))
        )>>(7))&1;
}

void vomit_cpu_cmp_flags16(vomit_cpu_t *cpu, DWORD result, WORD dest, WORD src )
{
    vomit_cpu_math_flags16(cpu, result, dest, src);
    cpu->OF = ((
        ((src)^(dest)) &
        ((src)^(src-dest))
        )>>(15))&1;
}
