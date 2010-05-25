/* 8086/flags.cpp
 * Handler of the hell that is flags
 */

#include "vcpu.h"

#if VOMIT_CPU_LEVEL <= 1
#define VOMIT_CPU_STATIC_FLAGS 0xF002
#else
#define VOMIT_CPU_STATIC_FLAGS 0x0000
#endif

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

void vomit_cpu_setAF(VCpu* cpu, DWORD result, WORD src, WORD dest)
{
    cpu->setAF((((result ^ (src ^ dest)) & 0x10) >> 4) & 1);
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

void _STC(VCpu* cpu)
{
    cpu->setCF(1);
}

void _STD(VCpu* cpu)
{
    cpu->setDF(1);
}

void _STI(VCpu* cpu)
{
    cpu->setIF(1);
}

void _CLI(VCpu* cpu)
{
    cpu->setIF(0);
}

void _CLC(VCpu* cpu)
{
    cpu->setCF(0);
}

void _CLD(VCpu* cpu)
{
    cpu->setDF(0);
}

void _CMC(VCpu* cpu)
{
    cpu->setCF(!cpu->getCF());
}

void _LAHF(VCpu* cpu)
{
    cpu->regs.B.AH = cpu->getCF() | (cpu->getPF() * 4) | (cpu->getAF() * 16) | (cpu->getZF() * 64) | (cpu->getSF() * 128) | 2;
}

void _SAHF(VCpu* cpu)
{
    cpu->setCF(cpu->regs.B.AH & 0x01);
    cpu->setPF(cpu->regs.B.AH & 0x04);
    cpu->setAF(cpu->regs.B.AH & 0x10);
    cpu->setZF(cpu->regs.B.AH & 0x40);
    cpu->setSF(cpu->regs.B.AH & 0x80);
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

void VCpu::setFlags(WORD flags)
{
    this->CF = (flags & 0x0001) != 0;
    this->PF = (flags & 0x0004) != 0;
    this->AF = (flags & 0x0010) != 0;
    this->ZF = (flags & 0x0040) != 0;
    this->SF = (flags & 0x0080) != 0;
    this->TF = (flags & 0x0100) != 0;
    this->IF = (flags & 0x0200) != 0;
    this->DF = (flags & 0x0400) != 0;
    this->OF = (flags & 0x0800) != 0;
}

WORD VCpu::getFlags()
 {
    return this->CF | (this->PF << 2) | (this->AF << 4) | (this->ZF << 6) | (this->SF << 7) | (this->TF << 8) | (this->IF << 9) | (this->DF << 10) | (this->OF << 11) | VOMIT_CPU_STATIC_FLAGS;
}

