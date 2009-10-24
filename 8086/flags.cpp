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
    cpu->setAF((((result ^ (src ^ dest)) & 0x10) >> 4) & 1);
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

void VCpu::updateFlags16(WORD data)
{
    this->PF = parity_table[data & 0xFF];
    this->SF = (data & 0x8000) != 0;
    this->ZF = data == 0;
}

void VCpu::updateFlags8(BYTE data)
{
    this->PF = parity_table[data];
    this->SF = (data & 0x80) != 0;
    this->ZF = data == 0;
}

void VCpu::updateFlags(WORD data, BYTE bits)
{
    if (bits == 8) {
        data &= 0xFF;
        this->PF = parity_table[data];
        this->SF = (data & 0x80) != 0;
    } else {
        this->PF = parity_table[data & 0xFF];
        this->SF = (data & 0x8000) != 0;
    }
    this->ZF = data == 0;
}

void _STC(vomit_cpu_t *cpu)
{
    cpu->setCF(1);
}

void _STD(vomit_cpu_t *cpu)
{
    cpu->setDF(1);
}

void _STI(vomit_cpu_t *cpu)
{
    cpu->setIF(1);
}

void _CLI(vomit_cpu_t *cpu)
{
    cpu->setIF(0);
}

void _CLC(vomit_cpu_t *cpu)
{
    cpu->setCF(0);
}

void _CLD(vomit_cpu_t *cpu)
{
    cpu->setDF(0);
}

void _CMC(vomit_cpu_t *cpu)
{
    cpu->setCF(!cpu->getCF());
}

void VCpu::mathFlags8(DWORD result, BYTE dest, BYTE src)
{
    this->CF = (result & 0x0100) != 0;
    this->SF = (result & 0x0080) != 0;
    this->ZF = (result & 0x00FF) == 0;
    this->PF = parity_table[result & 0xFF];
    vomit_cpu_setAF(this, result, dest, src);
}

void VCpu::mathFlags16(DWORD result, WORD dest, WORD src)
{
    this->CF = (result & 0x10000) != 0;
    this->SF = (result &  0x8000) != 0;
    this->ZF = (result &  0xFFFF) == 0;
    this->PF = parity_table[result & 0xFF];
    vomit_cpu_setAF(this, result, dest, src);
}

void VCpu::cmpFlags8(DWORD result, BYTE dest, BYTE src)
{
    mathFlags8(result, dest, src);
    this->OF = ((
        ((src)^(dest)) &
        ((src)^(src-dest))
        )>>(7))&1;
}

void VCpu::cmpFlags16(DWORD result, WORD dest, WORD src )
{
    mathFlags16(result, dest, src);
    this->OF = ((
        ((src)^(dest)) &
        ((src)^(src-dest))
        )>>(15))&1;
}
