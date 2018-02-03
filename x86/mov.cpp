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

void VCpu::_MOV_RM8_imm8()
{
    auto location = resolveModRM(fetchOpcodeByte());
    location.write8(fetchOpcodeByte());
}

void VCpu::_MOV_RM16_imm16()
{
    auto location = resolveModRM(fetchOpcodeByte());
    location.write16(fetchOpcodeWord());
}

void VCpu::_MOV_RM32_imm32()
{
    auto location = resolveModRM(fetchOpcodeByte());
    location.write32(fetchOpcodeDWord());
}

void VCpu::_MOV_RM16_seg()
{
    BYTE rm = fetchOpcodeByte();
    int segmentIndex = vomit_modRMRegisterPart(rm);
    ASSERT_VALID_SEGMENT_INDEX(segmentIndex);
    writeModRM16(rm, getSegment(static_cast<SegmentRegisterIndex>(segmentIndex)));
}

void VCpu::_MOV_RM32_seg()
{
    BYTE rm = fetchOpcodeByte();
    int segmentIndex = vomit_modRMRegisterPart(rm);
    ASSERT_VALID_SEGMENT_INDEX(segmentIndex);
    writeModRM32(rm, getSegment(static_cast<SegmentRegisterIndex>(segmentIndex)));
}

void VCpu::_MOV_seg_RM16()
{
    BYTE rm = fetchOpcodeByte();
    auto segmentIndex = static_cast<SegmentRegisterIndex>(vomit_modRMRegisterPart(rm));
    ASSERT_VALID_SEGMENT_INDEX(segmentIndex);
    setSegment(segmentIndex, readModRM16(rm));

    syncSegmentRegister(segmentIndex);
}

void VCpu::_MOV_seg_RM32()
{
    BYTE rm = fetchOpcodeByte();
    auto segmentIndex = static_cast<SegmentRegisterIndex>(vomit_modRMRegisterPart(rm));
    ASSERT_VALID_SEGMENT_INDEX(segmentIndex);
    setSegment(segmentIndex, readModRM32(rm));

    syncSegmentRegister(segmentIndex);
}

void VCpu::_MOV_RM8_reg8()
{
    BYTE rm = fetchOpcodeByte();
    writeModRM8(rm, getRegister8(static_cast<VCpu::RegisterIndex8>(vomit_modRMRegisterPart(rm))));
}

void VCpu::_MOV_reg8_RM8()
{
    BYTE rm = fetchOpcodeByte();
    setRegister8(static_cast<VCpu::RegisterIndex8>(vomit_modRMRegisterPart(rm)), readModRM8(rm));
}

void VCpu::_MOV_RM16_reg16()
{
    BYTE rm = fetchOpcodeByte();
    writeModRM16(rm, getRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm))));
}

void VCpu::_MOV_RM32_reg32()
{
    BYTE rm = fetchOpcodeByte();
    writeModRM32(rm, getRegister32(static_cast<VCpu::RegisterIndex32>(vomit_modRMRegisterPart(rm))));
}

void VCpu::_MOV_reg16_RM16()
{
    BYTE rm = fetchOpcodeByte();
    setRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm)), readModRM16(rm));
}

void VCpu::_MOV_reg32_RM32()
{
    BYTE rm = fetchOpcodeByte();
    setRegister32(static_cast<VCpu::RegisterIndex32>(vomit_modRMRegisterPart(rm)), readModRM32(rm));
}

void VCpu::_MOV_reg32_CR()
{
    BYTE rm = fetchOpcodeByte();
    int crIndex = vomit_modRMRegisterPart(rm);

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

    setRegister32(static_cast<VCpu::RegisterIndex32>(rm & 7), getControlRegister(crIndex));
}

void VCpu::_MOV_CR_reg32()
{
    BYTE rm = fetchOpcodeByte();
    int crIndex = vomit_modRMRegisterPart(rm);

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

    setControlRegister(crIndex, getRegister32(static_cast<VCpu::RegisterIndex32>(rm & 7)));

    //updateSizeModes();

    if (crIndex == 0)
        vlog(LogCPU, "MOV CR0 set CR0=%08X, PE=%u", CR0, getPE());

    //if (crIndex == 0 && !getPE())
//        updateSizeModes();
}

void VCpu::_MOV_AL_imm8()
{
    regs.B.AL = fetchOpcodeByte();
}

void VCpu::_MOV_BL_imm8()
{
    regs.B.BL = fetchOpcodeByte();
}

void VCpu::_MOV_CL_imm8()
{
    regs.B.CL = fetchOpcodeByte();
}

void VCpu::_MOV_DL_imm8()
{
    regs.B.DL = fetchOpcodeByte();
}

void VCpu::_MOV_AH_imm8()
{
    regs.B.AH = fetchOpcodeByte();
}

void VCpu::_MOV_BH_imm8()
{
    regs.B.BH = fetchOpcodeByte();
}

void VCpu::_MOV_CH_imm8()
{
    regs.B.CH = fetchOpcodeByte();
}

void VCpu::_MOV_DH_imm8()
{
    regs.B.DH = fetchOpcodeByte();
}

void VCpu::_MOV_EAX_imm32()
{
    regs.D.EAX = fetchOpcodeDWord();
}

void VCpu::_MOV_EBX_imm32()
{
    regs.D.EBX = fetchOpcodeDWord();
}

void VCpu::_MOV_ECX_imm32()
{
    regs.D.ECX = fetchOpcodeDWord();
}

void VCpu::_MOV_EDX_imm32()
{
    regs.D.EDX = fetchOpcodeDWord();
}

void VCpu::_MOV_EBP_imm32()
{
    regs.D.EBP = fetchOpcodeDWord();
}

void VCpu::_MOV_ESP_imm32()
{
    regs.D.ESP = fetchOpcodeDWord();
}

void VCpu::_MOV_ESI_imm32()
{
    regs.D.ESI = fetchOpcodeDWord();
}

void VCpu::_MOV_EDI_imm32()
{
    regs.D.EDI = fetchOpcodeDWord();
}

void VCpu::_MOV_AX_imm16()
{
    regs.W.AX = fetchOpcodeWord();
}

void VCpu::_MOV_BX_imm16()
{
    regs.W.BX = fetchOpcodeWord();
}

void VCpu::_MOV_CX_imm16()
{
    regs.W.CX = fetchOpcodeWord();
}

void VCpu::_MOV_DX_imm16()
{
    regs.W.DX = fetchOpcodeWord();
}

void VCpu::_MOV_BP_imm16()
{
    regs.W.BP = fetchOpcodeWord();
}

void VCpu::_MOV_SP_imm16()
{
    regs.W.SP = fetchOpcodeWord();
}

void VCpu::_MOV_SI_imm16()
{
    regs.W.SI = fetchOpcodeWord();
}

void VCpu::_MOV_DI_imm16()
{
    regs.W.DI = fetchOpcodeWord();
}

void VCpu::_MOV_AL_moff8()
{
    if (a16())
        regs.B.AL = readMemory8(currentSegment(), fetchOpcodeWord());
    else
        regs.B.AL = readMemory8(currentSegment(), fetchOpcodeDWord());
}

void VCpu::_MOV_AX_moff16()
{
    if (a16())
        regs.W.AX = readMemory16(currentSegment(), fetchOpcodeWord());
    else
        regs.W.AX = readMemory16(currentSegment(), fetchOpcodeDWord());
}

void VCpu::_MOV_EAX_moff32()
{
    if (a16())
        regs.D.EAX = readMemory32(currentSegment(), fetchOpcodeWord());
    else
        regs.D.EAX = readMemory32(currentSegment(), fetchOpcodeDWord());
}

void VCpu::_MOV_moff8_AL()
{
    if (a16())
        writeMemory8(currentSegment(), fetchOpcodeWord(), getAL());
    else
        writeMemory8(currentSegment(), fetchOpcodeDWord(), getAL());
}

void VCpu::_MOV_moff16_AX()
{
    if (a16())
        writeMemory16(currentSegment(), fetchOpcodeWord(), getAX());
    else
        writeMemory16(currentSegment(), fetchOpcodeDWord(), getAX());
}

void VCpu::_MOV_moff32_EAX()
{
    if (a16())
        writeMemory32(currentSegment(), fetchOpcodeWord(), getEAX());
    else
        writeMemory32(currentSegment(), fetchOpcodeDWord(), getEAX());
}

void VCpu::_MOVZX_reg16_RM8()
{
    BYTE rm = fetchOpcodeByte();
    setRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm)), readModRM8(rm));
    VM_ASSERT(0 == (getRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm))) & 0xFF00));
}

void VCpu::_MOVZX_reg32_RM8()
{
    BYTE rm = fetchOpcodeByte();
    setRegister32(static_cast<VCpu::RegisterIndex32>(vomit_modRMRegisterPart(rm)), readModRM8(rm));
    VM_ASSERT(0 == (getRegister32(static_cast<VCpu::RegisterIndex32>(vomit_modRMRegisterPart(rm))) & 0xFFFFFF00));
}

void VCpu::_MOVZX_reg32_RM16()
{
    BYTE rm = fetchOpcodeByte();
    setRegister32(static_cast<VCpu::RegisterIndex32>(vomit_modRMRegisterPart(rm)), readModRM16(rm));
    VM_ASSERT(0 == (getRegister32(static_cast<VCpu::RegisterIndex32>(vomit_modRMRegisterPart(rm))) & 0xFFFF0000));
}
