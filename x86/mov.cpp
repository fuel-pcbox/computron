/*
 * Copyright (C) 2003-2011 Andreas Kling <kling@webkit.org>
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

void _MOV_RM8_imm8(VCpu* cpu)
{
    (void) cpu->resolveModRM8(cpu->fetchOpcodeByte());
    cpu->updateModRM8(cpu->fetchOpcodeByte());
}

void _MOV_RM16_imm16(VCpu* cpu)
{
    (void) cpu->resolveModRM16(cpu->fetchOpcodeByte());
    cpu->updateModRM16(cpu->fetchOpcodeWord());
}

void _MOV_RM32_imm32(VCpu* cpu)
{
    (void) cpu->resolveModRM32(cpu->fetchOpcodeByte());
    cpu->updateModRM32(cpu->fetchOpcodeDWord());
}

void _MOV_RM16_seg(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    int segmentIndex = vomit_modRMRegisterPart(rm);
    ASSERT_VALID_SEGMENT_INDEX(segmentIndex);
    cpu->writeModRM16(rm, cpu->getSegment(static_cast<VCpu::SegmentIndex>(segmentIndex)));
}

void _MOV_RM32_seg(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    int segmentIndex = vomit_modRMRegisterPart(rm);
    ASSERT_VALID_SEGMENT_INDEX(segmentIndex);
    cpu->writeModRM32(rm, cpu->getSegment(static_cast<VCpu::SegmentIndex>(segmentIndex)));
}

static void syncSegmentRegister(VCpu* cpu, VCpu::SegmentIndex segmentIndex)
{
    ASSERT_VALID_SEGMENT_INDEX(segmentIndex);
    VCpu::SegmentSelector& selector = cpu->m_selector[segmentIndex];

    WORD segment = cpu->getSegment(segmentIndex);

    if (segment % 8)
        vlog(VM_ALERT, "Segment selector index %u not divisible by 8.", segment);

    if (segment >= cpu->GDTR.limit)
        vlog(VM_ALERT, "Segment selector index %u >= GDTR.limit.", segment);

    DWORD hi = cpu->readMemory32(cpu->GDTR.base + segment + 4);
    DWORD lo = cpu->readMemory32(cpu->GDTR.base + segment);

    selector.base = (hi & 0xFF000000) | ((hi & 0xFF) << 16) | ((lo >> 16) & 0xFFFF);
    selector.limit = (hi & 0xF0000) | (lo & 0xFFFF);
    selector.acc = hi >> 7;
    selector.BRW = hi >> 8;
    selector.CE = hi >> 9;
    selector._32bit = hi >> 10;
    selector.DPL = (hi >> 12) & 3;
    selector.present = hi >> 14;
    selector.big = hi >> 22;
    selector.granularity = hi >> 23;
}

void _MOV_seg_RM16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    VCpu::SegmentIndex segmentIndex = static_cast<VCpu::SegmentIndex>(vomit_modRMRegisterPart(rm));
    ASSERT_VALID_SEGMENT_INDEX(segmentIndex);
    cpu->setSegment(segmentIndex, cpu->readModRM16(rm));

    if (cpu->getPE())
        syncSegmentRegister(cpu, segmentIndex);
}

void _MOV_seg_RM32(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    VCpu::SegmentIndex segmentIndex = static_cast<VCpu::SegmentIndex>(vomit_modRMRegisterPart(rm));
    ASSERT_VALID_SEGMENT_INDEX(segmentIndex);
    cpu->setSegment(segmentIndex, cpu->readModRM32(rm));

    if (cpu->getPE())
        syncSegmentRegister(cpu, static_cast<VCpu::SegmentIndex>(segmentIndex));
}

void _MOV_RM8_reg8(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    cpu->writeModRM8(rm, cpu->getRegister8(static_cast<VCpu::RegisterIndex8>(vomit_modRMRegisterPart(rm))));
}

void _MOV_reg8_RM8(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    cpu->setRegister8(static_cast<VCpu::RegisterIndex8>(vomit_modRMRegisterPart(rm)), cpu->readModRM8(rm));
}

void _MOV_RM16_reg16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    cpu->writeModRM16(rm, cpu->getRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm))));
}

void _MOV_RM32_reg32(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    cpu->writeModRM32(rm, cpu->getRegister32(static_cast<VCpu::RegisterIndex32>(vomit_modRMRegisterPart(rm))));
}

void _MOV_reg16_RM16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    cpu->setRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm)), cpu->readModRM16(rm));
}

void _MOV_reg32_RM32(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    cpu->setRegister32(static_cast<VCpu::RegisterIndex32>(vomit_modRMRegisterPart(rm)), cpu->readModRM32(rm));
}

void _MOV_reg32_CR(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    int crIndex = vomit_modRMRegisterPart(rm);

    if (cpu->getVM()) {
        cpu->GP(0);
        return;
    }

    if (cpu->getPE()) {
        // FIXME: Other GP(0) conditions:
        // If an attempt is made to write invalid bit combinations in CR0
        // (such as setting the PG flag to 1 when the PE flag is set to 0, or
        // setting the CD flag to 0 when the NW flag is set to 1).
        // If an attempt is made to write a 1 to any reserved bit in CR4.
        // If an attempt is made to write 1 to CR4.PCIDE.
        // If any of the reserved bits are set in the page-directory pointers
        // table (PDPT) and the loading of a control register causes the
        // PDPT to be loaded into the processor.
        if (cpu->getCPL() != 0) {
            cpu->GP(0);
            return;
        }
    } else {
        // FIXME: GP(0) conditions:
        // If an attempt is made to write a 1 to any reserved bit in CR4.
        // If an attempt is made to write 1 to CR4.PCIDE.
        // If an attempt is made to write invalid bit combinations in CR0
        // (such as setting the PG flag to 1 when the PE flag is set to 0).
        if (crIndex == 1 || crIndex == 5 || crIndex == 6 || crIndex == 7) {
            cpu->exception(6);
            return;
        }
    }

    cpu->setRegister32(static_cast<VCpu::RegisterIndex32>(rm & 7), cpu->getControlRegister(crIndex));
}

void _MOV_CR_reg32(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    int crIndex = vomit_modRMRegisterPart(rm);

    if (cpu->getVM()) {
        cpu->GP(0);
        return;
    }

    if (cpu->getPE()) {
        // FIXME: Other GP(0) conditions:
        // If an attempt is made to write invalid bit combinations in CR0
        // (such as setting the PG flag to 1 when the PE flag is set to 0, or
        // setting the CD flag to 0 when the NW flag is set to 1).
        // If an attempt is made to write a 1 to any reserved bit in CR4.
        // If an attempt is made to write 1 to CR4.PCIDE.
        // If any of the reserved bits are set in the page-directory pointers
        // table (PDPT) and the loading of a control register causes the
        // PDPT to be loaded into the processor.
        if (cpu->getCPL() != 0) {
            cpu->GP(0);
            return;
        }
    } else {
        // FIXME: GP(0) conditions:
        // If an attempt is made to write a 1 to any reserved bit in CR4.
        // If an attempt is made to write 1 to CR4.PCIDE.
        // If an attempt is made to write invalid bit combinations in CR0
        // (such as setting the PG flag to 1 when the PE flag is set to 0).
        if (crIndex == 1 || crIndex == 5 || crIndex == 6 || crIndex == 7) {
            cpu->exception(6);
            return;
        }
    }

    cpu->setControlRegister(crIndex, cpu->getRegister32(static_cast<VCpu::RegisterIndex32>(rm & 7)));

    if (crIndex == 0)
        vlog(VM_CPUMSG, "Protected mode Enable = %u", cpu->getPE());
}

void _MOV_AL_imm8(VCpu* cpu)
{
    cpu->regs.B.AL = cpu->fetchOpcodeByte();
}

void _MOV_BL_imm8(VCpu* cpu)
{
    cpu->regs.B.BL = cpu->fetchOpcodeByte();
}

void _MOV_CL_imm8(VCpu* cpu)
{
    cpu->regs.B.CL = cpu->fetchOpcodeByte();
}

void _MOV_DL_imm8(VCpu* cpu)
{
    cpu->regs.B.DL = cpu->fetchOpcodeByte();
}

void _MOV_AH_imm8(VCpu* cpu)
{
    cpu->regs.B.AH = cpu->fetchOpcodeByte();
}

void _MOV_BH_imm8(VCpu* cpu)
{
    cpu->regs.B.BH = cpu->fetchOpcodeByte();
}

void _MOV_CH_imm8(VCpu* cpu)
{
    cpu->regs.B.CH = cpu->fetchOpcodeByte();
}

void _MOV_DH_imm8(VCpu* cpu)
{
    cpu->regs.B.DH = cpu->fetchOpcodeByte();
}

void _MOV_EAX_imm32(VCpu* cpu)
{
    cpu->regs.D.EAX = cpu->fetchOpcodeDWord();
}

void _MOV_EBX_imm32(VCpu* cpu)
{
    cpu->regs.D.EBX = cpu->fetchOpcodeDWord();
}

void _MOV_ECX_imm32(VCpu* cpu)
{
    cpu->regs.D.ECX = cpu->fetchOpcodeDWord();
}

void _MOV_EDX_imm32(VCpu* cpu)
{
    cpu->regs.D.EDX = cpu->fetchOpcodeDWord();
}

void _MOV_EBP_imm32(VCpu* cpu)
{
    cpu->regs.D.EBP = cpu->fetchOpcodeDWord();
}

void _MOV_ESP_imm32(VCpu* cpu)
{
    cpu->regs.D.ESP = cpu->fetchOpcodeDWord();
}

void _MOV_ESI_imm32(VCpu* cpu)
{
    cpu->regs.D.ESI = cpu->fetchOpcodeDWord();
}

void _MOV_EDI_imm32(VCpu* cpu)
{
    cpu->regs.D.EDI = cpu->fetchOpcodeDWord();
}

void _MOV_AX_imm16(VCpu* cpu)
{
    cpu->regs.W.AX = cpu->fetchOpcodeWord();
}

void _MOV_BX_imm16(VCpu* cpu)
{
    cpu->regs.W.BX = cpu->fetchOpcodeWord();
}

void _MOV_CX_imm16(VCpu* cpu)
{
    cpu->regs.W.CX = cpu->fetchOpcodeWord();
}

void _MOV_DX_imm16(VCpu* cpu)
{
    cpu->regs.W.DX = cpu->fetchOpcodeWord();
}

void _MOV_BP_imm16(VCpu* cpu)
{
    cpu->regs.W.BP = cpu->fetchOpcodeWord();
}

void _MOV_SP_imm16(VCpu* cpu)
{
    cpu->regs.W.SP = cpu->fetchOpcodeWord();
}

void _MOV_SI_imm16(VCpu* cpu)
{
    cpu->regs.W.SI = cpu->fetchOpcodeWord();
}

void _MOV_DI_imm16(VCpu* cpu)
{
    cpu->regs.W.DI = cpu->fetchOpcodeWord();
}

void _MOV_AL_moff8(VCpu* cpu)
{
    if (cpu->a16())
        cpu->regs.B.AL = cpu->readMemory8(cpu->currentSegment(), cpu->fetchOpcodeWord());
    else
        cpu->regs.B.AL = cpu->readMemory8(cpu->currentSegment(), cpu->fetchOpcodeDWord());
}

void _MOV_AX_moff16(VCpu* cpu)
{
    if (cpu->a16())
        cpu->regs.W.AX = cpu->readMemory16(cpu->currentSegment(), cpu->fetchOpcodeWord());
    else
        cpu->regs.W.AX = cpu->readMemory16(cpu->currentSegment(), cpu->fetchOpcodeDWord());
}

void _MOV_EAX_moff32(VCpu* cpu)
{
    if (cpu->a16())
        cpu->regs.D.EAX = cpu->readMemory32(cpu->currentSegment(), cpu->fetchOpcodeWord());
    else
        cpu->regs.D.EAX = cpu->readMemory32(cpu->currentSegment(), cpu->fetchOpcodeDWord());
}

void _MOV_moff8_AL(VCpu* cpu)
{
    if (cpu->a16())
        cpu->writeMemory8(cpu->currentSegment(), cpu->fetchOpcodeWord(), cpu->getAL());
    else
        cpu->writeMemory8(cpu->currentSegment(), cpu->fetchOpcodeDWord(), cpu->getAL());
}

void _MOV_moff16_AX(VCpu* cpu)
{
    if (cpu->a16())
        cpu->writeMemory16(cpu->currentSegment(), cpu->fetchOpcodeWord(), cpu->getAX());
    else
        cpu->writeMemory16(cpu->currentSegment(), cpu->fetchOpcodeDWord(), cpu->getAX());
}

void _MOV_moff32_EAX(VCpu* cpu)
{
    if (cpu->a16())
        cpu->writeMemory32(cpu->currentSegment(), cpu->fetchOpcodeWord(), cpu->getEAX());
    else
        cpu->writeMemory32(cpu->currentSegment(), cpu->fetchOpcodeDWord(), cpu->getEAX());
}

void _MOVZX_reg16_RM8(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    cpu->setRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm)), cpu->readModRM8(rm));
}

void _MOVZX_reg32_RM8(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    cpu->setRegister32(static_cast<VCpu::RegisterIndex32>(vomit_modRMRegisterPart(rm)), cpu->readModRM8(rm));
}

void _MOVZX_reg32_RM16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    cpu->setRegister32(static_cast<VCpu::RegisterIndex32>(vomit_modRMRegisterPart(rm)), cpu->readModRM16(rm));
}
