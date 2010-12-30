// x86/flags.cpp
// Flag-related mess

#include "vcpu.h"

static const BYTE parity_table[0x100] = {
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

void VCpu::adjustFlag32(DWORD result, WORD src, WORD dest)
{
    setAF((((result ^ (src ^ dest)) & 0x10) >> 4) & 1);
}

void VCpu::updateFlags32(DWORD data)
{
    this->PF = parity_table[data & 0xFF];
    this->SF = (data & 0x80000000) != 0;
    this->ZF = data == 0;
}

void VCpu::updateFlags16(WORD data)
{
    setPF(parity_table[data & 0xFF]);
    setSF(data & 0x8000);
    setZF(data == 0);
}

void VCpu::updateFlags8(BYTE data)
{
    setPF(parity_table[data]);
    setSF(data & 0x80);
    setZF(data == 0);
}

void VCpu::updateFlags(WORD data, BYTE bits)
{
    if (bits == 8) {
        data &= 0xFF;
        setSF(data & 0x80);
    } else
        setSF(data & 0x8000);

    setPF(parity_table[data & 0xFF]);
    setZF(data == 0);
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
    if (!cpu->getPE()) {
        cpu->setIF(1);
        return;
    }

    if (!cpu->getVM()) {
        if (cpu->getIOPL() >= cpu->getCPL()) {
            cpu->setIF(1);
        } else {
            if ((cpu->getIOPL() < cpu->getCPL()) && cpu->getCPL() == 3 && !cpu->getVIP())
                cpu->setVIF(1);
            else
                cpu->GP(0);
        }
    } else {
        if (cpu->getIOPL() == 3) {
            cpu->setIF(0);
        } else {
            if (cpu->getIOPL() < 3 && !cpu->getVIP() && cpu->getVME()) {
                cpu->setVIF(1);
            } else {
                cpu->GP(0);
            }
        }
    }
}

void _CLI(VCpu* cpu)
{
    if (!cpu->getPE()) {
        cpu->setIF(0);
        return;
    }

    if (!cpu->getVM()) {
        if (cpu->getIOPL() >= cpu->getCPL()) {
            cpu->setIF(0);
        } else {
            if ((cpu->getIOPL() < cpu->getCPL()) && cpu->getCPL() == 3 && cpu->getPVI())
                cpu->setVIF(0);
            else
                cpu->GP(0);
        }
    } else {
        if (cpu->getIOPL() == 3) {
            cpu->setIF(0);
        } else {
            if (cpu->getIOPL() < 3 && cpu->getVME()) {
                cpu->setVIF(0);
            } else {
                cpu->GP(0);
            }
        }
    }
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

void VCpu::mathFlags8(WORD result, BYTE dest, BYTE src)
{
    setCF(result & 0x0100);
    setSF(result & 0x0080);
    setZF((result & 0x00FF) == 0);
    setPF(parity_table[result & 0xFF]);
    adjustFlag32(result, dest, src);
}

void VCpu::mathFlags16(DWORD result, WORD dest, WORD src)
{
    setCF(result & 0x10000);
    setSF(result & 0x8000);
    setZF((result & 0xFFFF) == 0);
    setPF(parity_table[result & 0xFF]);
    adjustFlag32(result, dest, src);
}

void VCpu::mathFlags32(QWORD result, DWORD dest, DWORD src)
{
    setCF(result & 0x100000000ULL);
    setSF(result & 0x80000000);
    setZF((result & 0xFFFFFFFF) == 0);
    setPF(parity_table[result & 0xFF]);
    adjustFlag32(result, dest, src);
}

void VCpu::cmpFlags8(DWORD result, BYTE dest, BYTE src)
{
    mathFlags8(result, dest, src);
    setOF(((
        ((src)^(dest)) &
        ((src)^(src-dest))
        )>>(7))&1);
}

void VCpu::cmpFlags16(DWORD result, WORD dest, WORD src)
{
    mathFlags16(result, dest, src);
    setOF(((
        ((src)^(dest)) &
        ((src)^(src-dest))
        )>>(15))&1);
}

void VCpu::cmpFlags32(QWORD result, DWORD dest, DWORD src)
{
    mathFlags32(result, dest, src);
    setOF(((
        ((src)^(dest)) &
        ((src)^(src-dest))
        )>>(31))&1);
}

void VCpu::setFlags(WORD flags)
{
    setCF(flags & 0x0001);
    setPF(flags & 0x0004);
    setAF(flags & 0x0010);
    setZF(flags & 0x0040);
    setSF(flags & 0x0080);
    setTF(flags & 0x0100);
    setIF(flags & 0x0200);
    setDF(flags & 0x0400);
    setOF(flags & 0x0800);
    setIOPL((flags & 0x3000) >> 12);
    setNT(flags & 0x4000);
}

WORD VCpu::getFlags() const
{
    return 0x0002
        | (getCF() << 0)
        | (getPF() << 2)
        | (getAF() << 4)
        | (getZF() << 6)
        | (getSF() << 7)
        | (getTF() << 8)
        | (getIF() << 9)
        | (getDF() << 10)
        | (getOF() << 11)
        | (getIOPL() << 13)
        | (getNT() << 14);
}

void VCpu::setEFlags(DWORD eflags)
{
    setFlags(eflags & 0xFFFF);

    this->RF = (eflags & 0x10000) != 0;
    this->VM = (eflags & 0x20000) != 0;
    //this->AC = (eflags & 0x40000) != 0;
    this->VIF = (eflags & 0x80000) != 0;
    this->VIP = (eflags & 0x100000) != 0;
    this->ID = (eflags & 0x200000) != 0;
}

DWORD VCpu::getEFlags() const
{
    DWORD eflags = getFlags()
         | (this->RF << 16)
         | (this->VM << 17)
//       | (this->AC << 18)
         | (this->VIF << 19)
         | (this->VIP << 20)
         | (this->ID << 21);
    return eflags;
}
