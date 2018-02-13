/*
 * Copyright (C) 2003-2018 Andreas Kling <awesomekling@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ANDREAS KLING ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANDREAS KLING OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "vcpu.h"
#include "debug.h"

void CPU::_MOV_RM8_imm8(Instruction& insn)
{
    insn.modrm().write8(insn.imm8());
}

void CPU::_MOV_RM16_imm16(Instruction& insn)
{
    insn.modrm().write16(insn.imm16());
}

void CPU::_MOV_RM32_imm32(Instruction& insn)
{
    insn.modrm().write32(insn.imm32());
}

void CPU::_MOV_RM16_seg(Instruction& insn)
{
    insn.modrm().write16(insn.segreg());
}

void CPU::_MOV_RM32_seg(Instruction& insn)
{
    insn.modrm().write32(insn.segreg());
}

void CPU::_MOV_seg_RM16(Instruction& insn)
{
    insn.segreg() = insn.modrm().read16();
    syncSegmentRegister(insn.segmentRegisterIndex());
}

void CPU::_MOV_seg_RM32(Instruction& insn)
{
    insn.segreg() = insn.modrm().read32();
    syncSegmentRegister(insn.segmentRegisterIndex());
}

void CPU::_MOV_RM8_reg8(Instruction& insn)
{
    insn.modrm().write8(insn.reg8());
}

void CPU::_MOV_reg8_RM8(Instruction& insn)
{
    insn.reg8() = insn.modrm().read8();
}

void CPU::_MOV_RM16_reg16(Instruction& insn)
{
    insn.modrm().write16(insn.reg16());
}

void CPU::_MOV_RM32_reg32(Instruction& insn)
{
    insn.modrm().write32(insn.reg32());
}

void CPU::_MOV_reg16_RM16(Instruction& insn)
{
    insn.reg16() = insn.modrm().read16();
}

void CPU::_MOV_reg32_RM32(Instruction& insn)
{
    insn.reg32() = insn.modrm().read32();
}

void CPU::_MOV_reg32_CR(Instruction& insn)
{
    int crIndex = insn.registerIndex();

    if (getVM()) {
        GP(0);
        return;
    }

    if (getPE()) {
        // FIXME: Other GP(0) conditions:
        // If an attempt is made to write invalid bit combinations in CR0
        // (such as setting the PG flag to 1 when the PE flag is set to 0, or
        // setting the CD flag to 0 when the NW flag is set to 1).
        // If an attempt is made to write a 1 to any reserved bit in CR4.
        // If an attempt is made to write 1 to CR4.PCIDE.
        // If any of the reserved bits are set in the page-directory pointers
        // table (PDPT) and the loading of a control register causes the
        // PDPT to be loaded into the processor.
        if (getCPL() != 0) {
            GP(0);
            return;
        }
    } else {
        // FIXME: GP(0) conditions:
        // If an attempt is made to write a 1 to any reserved bit in CR4.
        // If an attempt is made to write 1 to CR4.PCIDE.
        // If an attempt is made to write invalid bit combinations in CR0
        // (such as setting the PG flag to 1 when the PE flag is set to 0).
        if (crIndex == 1 || crIndex == 5 || crIndex == 6 || crIndex == 7) {
            exception(6);
            return;
        }
    }

    setRegister32(static_cast<CPU::RegisterIndex32>(insn.rm() & 7), getControlRegister(crIndex));
}

void CPU::_MOV_CR_reg32(Instruction& insn)
{
    int crIndex = insn.registerIndex();

    if (getVM()) {
        GP(0);
        return;
    }

    if (getPE()) {
        // FIXME: Other GP(0) conditions:
        // If an attempt is made to write invalid bit combinations in CR0
        // (such as setting the PG flag to 1 when the PE flag is set to 0, or
        // setting the CD flag to 0 when the NW flag is set to 1).
        // If an attempt is made to write a 1 to any reserved bit in CR4.
        // If an attempt is made to write 1 to CR4.PCIDE.
        // If any of the reserved bits are set in the page-directory pointers
        // table (PDPT) and the loading of a control register causes the
        // PDPT to be loaded into the processor.
        if (getCPL() != 0) {
            GP(0);
            return;
        }
    } else {
        // FIXME: GP(0) conditions:
        // If an attempt is made to write a 1 to any reserved bit in CR4.
        // If an attempt is made to write 1 to CR4.PCIDE.
        // If an attempt is made to write invalid bit combinations in CR0
        // (such as setting the PG flag to 1 when the PE flag is set to 0).
        if (crIndex == 1 || crIndex == 5 || crIndex == 6 || crIndex == 7) {
            exception(6);
            return;
        }
    }

    setControlRegister(crIndex, getRegister32(static_cast<CPU::RegisterIndex32>(insn.rm() & 7)));

    vlog(LogCPU, "MOV CR%u <- %08X", crIndex, getControlRegister(crIndex));
}

void CPU::_MOV_reg32_DR(Instruction& insn)
{
    int drIndex = insn.registerIndex();
    auto registerIndex = static_cast<CPU::RegisterIndex32>(insn.rm() & 7);

    if (getVM()) {
        GP(0);
        return;
    }

    if (getPE()) {
        if (getCPL() != 0) {
            GP(0);
            return;
        }
    }

    setRegister32(registerIndex, getDebugRegister(drIndex));
    vlog(LogCPU, "MOV %s <- DR%u (%08X)", registerName(registerIndex), drIndex, getDebugRegister(drIndex));
}

void CPU::_MOV_DR_reg32(Instruction& insn)
{
    int drIndex = insn.registerIndex();
    auto registerIndex = static_cast<CPU::RegisterIndex32>(insn.rm() & 7);

    if (getVM()) {
        GP(0);
        return;
    }

    if (getPE()) {
        if (getCPL() != 0) {
            GP(0);
            return;
        }
    }

    setDebugRegister(drIndex, getRegister32(registerIndex));
    //vlog(LogCPU, "MOV DR%u <- %08X", drIndex, getDebugRegister(drIndex));
}

void CPU::_MOV_AL_imm8(Instruction& insn)
{
    regs.B.AL = insn.imm8();
}

void CPU::_MOV_BL_imm8(Instruction& insn)
{
    regs.B.BL = insn.imm8();
}

void CPU::_MOV_CL_imm8(Instruction& insn)
{
    regs.B.CL = insn.imm8();
}

void CPU::_MOV_DL_imm8(Instruction& insn)
{
    regs.B.DL = insn.imm8();
}

void CPU::_MOV_AH_imm8(Instruction& insn)
{
    regs.B.AH = insn.imm8();
}

void CPU::_MOV_BH_imm8(Instruction& insn)
{
    regs.B.BH = insn.imm8();
}

void CPU::_MOV_CH_imm8(Instruction& insn)
{
    regs.B.CH = insn.imm8();
}

void CPU::_MOV_DH_imm8(Instruction& insn)
{
    regs.B.DH = insn.imm8();
}

void CPU::_MOV_EAX_imm32(Instruction& insn)
{
    regs.D.EAX = insn.imm32();
}

void CPU::_MOV_EBX_imm32(Instruction& insn)
{
    regs.D.EBX = insn.imm32();
}

void CPU::_MOV_ECX_imm32(Instruction& insn)
{
    regs.D.ECX = insn.imm32();
}

void CPU::_MOV_EDX_imm32(Instruction& insn)
{
    regs.D.EDX = insn.imm32();
}

void CPU::_MOV_EBP_imm32(Instruction& insn)
{
    regs.D.EBP = insn.imm32();
}

void CPU::_MOV_ESP_imm32(Instruction& insn)
{
    regs.D.ESP = insn.imm32();
}

void CPU::_MOV_ESI_imm32(Instruction& insn)
{
    regs.D.ESI = insn.imm32();
}

void CPU::_MOV_EDI_imm32(Instruction& insn)
{
    regs.D.EDI = insn.imm32();
}

void CPU::_MOV_AX_imm16(Instruction& insn)
{
    regs.W.AX = insn.imm16();
}

void CPU::_MOV_BX_imm16(Instruction& insn)
{
    regs.W.BX = insn.imm16();
}

void CPU::_MOV_CX_imm16(Instruction& insn)
{
    regs.W.CX = insn.imm16();
}

void CPU::_MOV_DX_imm16(Instruction& insn)
{
    regs.W.DX = insn.imm16();
}

void CPU::_MOV_BP_imm16(Instruction& insn)
{
    regs.W.BP = insn.imm16();
}

void CPU::_MOV_SP_imm16(Instruction& insn)
{
    regs.W.SP = insn.imm16();
}

void CPU::_MOV_SI_imm16(Instruction& insn)
{
    regs.W.SI = insn.imm16();
}

void CPU::_MOV_DI_imm16(Instruction& insn)
{
    regs.W.DI = insn.imm16();
}

void CPU::_MOV_AL_moff8(Instruction& insn)
{
    if (a16())
        regs.B.AL = readMemory8(currentSegment(), insn.imm16());
    else
        regs.B.AL = readMemory8(currentSegment(), insn.imm32());
}

void CPU::_MOV_AX_moff16(Instruction& insn)
{
    if (a16())
        regs.W.AX = readMemory16(currentSegment(), insn.imm16());
    else
        regs.W.AX = readMemory16(currentSegment(), insn.imm32());
}

void CPU::_MOV_EAX_moff32(Instruction& insn)
{
    if (a16())
        regs.D.EAX = readMemory32(currentSegment(), insn.imm16());
    else
        regs.D.EAX = readMemory32(currentSegment(), insn.imm32());
}

void CPU::_MOV_moff8_AL(Instruction& insn)
{
    if (a16())
        writeMemory8(currentSegment(), insn.imm16(), getAL());
    else
        writeMemory8(currentSegment(), insn.imm32(), getAL());
}

void CPU::_MOV_moff16_AX(Instruction& insn)
{
    if (a16())
        writeMemory16(currentSegment(), insn.imm16(), getAX());
    else
        writeMemory16(currentSegment(), insn.imm32(), getAX());
}

void CPU::_MOV_moff32_EAX(Instruction& insn)
{
    if (a16())
        writeMemory32(currentSegment(), insn.imm16(), getEAX());
    else
        writeMemory32(currentSegment(), insn.imm32(), getEAX());
}

void CPU::_MOVZX_reg16_RM8(Instruction& insn)
{
    insn.reg16() = insn.modrm().read8();
}

void CPU::_MOVZX_reg32_RM8(Instruction& insn)
{
    insn.reg32() = insn.modrm().read8();
}

void CPU::_MOVZX_reg32_RM16(Instruction& insn)
{
    insn.reg32() = insn.modrm().read16();
}

void CPU::_MOVSX_reg16_RM8(Instruction& insn)
{
    insn.reg16() = vomit_signExtend<WORD>(insn.modrm().read8());
}

void CPU::_MOVSX_reg32_RM8(Instruction& insn)
{
    insn.reg32() = vomit_signExtend<DWORD>(insn.modrm().read8());
}

void CPU::_MOVSX_reg32_RM16(Instruction& insn)
{
    insn.reg32() = vomit_signExtend<DWORD>(insn.modrm().read16());
}
