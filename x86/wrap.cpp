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

void VCpu::_wrap_0x80()
{
    rmbyte = fetchOpcodeByte();
    switch(vomit_modRMRegisterPart(rmbyte)) {
    case 0: _ADD_RM8_imm8(); break;
    case 1:  _OR_RM8_imm8(); break;
    case 2: _ADC_RM8_imm8(); break;
    case 3: _SBB_RM8_imm8(); break;
    case 4: _AND_RM8_imm8(); break;
    case 5: _SUB_RM8_imm8(); break;
    case 6: _XOR_RM8_imm8(); break;
    case 7: _CMP_RM8_imm8(); break;
    }
}

void VCpu::_wrap_0x81_16()
{
    rmbyte = fetchOpcodeByte();
    switch(vomit_modRMRegisterPart(rmbyte)) {
    case 0: _ADD_RM16_imm16(); break;
    case 1:  _OR_RM16_imm16(); break;
    case 2: _ADC_RM16_imm16(); break;
    case 3: _SBB_RM16_imm16(); break;
    case 4: _AND_RM16_imm16(); break;
    case 5: _SUB_RM16_imm16(); break;
    case 6: _XOR_RM16_imm16(); break;
    case 7: _CMP_RM16_imm16(); break;
    }
}

void VCpu::_wrap_0x81_32()
{
    rmbyte = fetchOpcodeByte();
    switch(vomit_modRMRegisterPart(rmbyte)) {
    case 0: _ADD_RM32_imm32(); break;
    case 1:  _OR_RM32_imm32(); break;
    case 2: _ADC_RM32_imm32(); break;
    case 3: _SBB_RM32_imm32(); break;
    case 4: _AND_RM32_imm32(); break;
    case 5: _SUB_RM32_imm32(); break;
    case 6: _XOR_RM32_imm32(); break;
    case 7: _CMP_RM32_imm32(); break;
    }
}

void VCpu::_wrap_0x83_16()
{
    rmbyte = fetchOpcodeByte();
    switch(vomit_modRMRegisterPart(rmbyte)) {
    case 0: _ADD_RM16_imm8(); break;
    case 1:  _OR_RM16_imm8(); break;
    case 2: _ADC_RM16_imm8(); break;
    case 3: _SBB_RM16_imm8(); break;
    case 4: _AND_RM16_imm8(); break;
    case 5: _SUB_RM16_imm8(); break;
    case 6: _XOR_RM16_imm8(); break;
    case 7: _CMP_RM16_imm8(); break;
    }
}

void VCpu::_wrap_0x83_32()
{
    rmbyte = fetchOpcodeByte();
    switch(vomit_modRMRegisterPart(rmbyte)) {
    case 0: _ADD_RM32_imm8(); break;
    case 1:  _OR_RM32_imm8(); break;
    case 2: _ADC_RM32_imm8(); break;
    case 3: _SBB_RM32_imm8(); break;
    case 4: _AND_RM32_imm8(); break;
    case 5: _SUB_RM32_imm8(); break;
    case 6: _XOR_RM32_imm8(); break;
    case 7: _CMP_RM32_imm8(); break;
    }
}

void VCpu::_wrap_0x8F_16()
{
    rmbyte = fetchOpcodeByte();
    switch(vomit_modRMRegisterPart(rmbyte)) {
    case 0: _POP_RM16(); break;
    default:
        vlog(VM_ALERT, "[16bit] 8F /%u not wrapped", vomit_modRMRegisterPart(rmbyte));
        exception(6);
    }
}

void VCpu::_wrap_0x8F_32()
{
    rmbyte = fetchOpcodeByte();
    switch(vomit_modRMRegisterPart(rmbyte)) {
    case 0: _POP_RM32(); break;
    default:
        vlog(VM_ALERT, "[32bit] 8F /%u not wrapped", vomit_modRMRegisterPart(rmbyte));
        exception(6);
    }
}

void VCpu::_wrap_0xC0()
{
    BYTE rm = fetchOpcodeByte();
    BYTE value = readModRM8(rm);
    BYTE imm = fetchOpcodeByte();

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM8(cpu_rol(this, value, imm, 8)); break;
    case 1: updateModRM8(cpu_ror(this, value, imm, 8)); break;
    case 2: updateModRM8(cpu_rcl(this, value, imm, 8)); break;
    case 3: updateModRM8(cpu_rcr(this, value, imm, 8)); break;
    case 4: updateModRM8(cpu_shl(this, value, imm, 8)); break;
    case 5: updateModRM8(cpu_shr(this, value, imm, 8)); break;
    case 6:
        vlog(VM_ALERT, "C0 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM8(cpu_sar(this, value, imm, 8)); break;
    }
}

void VCpu::_wrap_0xC1_16()
{
    BYTE rm = fetchOpcodeByte();
    WORD value = readModRM16(rm);
    BYTE imm = fetchOpcodeByte();

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM16(cpu_rol(this, value, imm, 16)); break;
    case 1: updateModRM16(cpu_ror(this, value, imm, 16)); break;
    case 2: updateModRM16(cpu_rcl(this, value, imm, 16)); break;
    case 3: updateModRM16(cpu_rcr(this, value, imm, 16)); break;
    case 4: updateModRM16(cpu_shl(this, value, imm, 16)); break;
    case 5: updateModRM16(cpu_shr(this, value, imm, 16)); break;
    case 6:
        vlog(VM_ALERT, "[16bit] C1 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM16(cpu_sar(this, value, imm, 16)); break;
    }
}

void VCpu::_wrap_0xC1_32()
{
    BYTE rm = fetchOpcodeByte();
    DWORD value = readModRM32(rm);
    BYTE imm = fetchOpcodeByte();

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM32(cpu_rol(this, value, imm, 32)); break;
    case 1: updateModRM32(cpu_ror(this, value, imm, 32)); break;
    case 2: updateModRM32(cpu_rcl(this, value, imm, 32)); break;
    case 3: updateModRM32(cpu_rcr(this, value, imm, 32)); break;
    case 4: updateModRM32(cpu_shl(this, value, imm, 32)); break;
    case 5: updateModRM32(cpu_shr(this, value, imm, 32)); break;
    case 6:
        vlog(VM_ALERT, "[32bit] C1 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM32(cpu_sar(this, value, imm, 32)); break;
    }
}

void VCpu::_wrap_0xD0()
{
    BYTE rm = fetchOpcodeByte();
    BYTE value = readModRM8(rm);

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM8(cpu_rol(this, value, 1, 8 )); break;
    case 1: updateModRM8(cpu_ror(this, value, 1, 8 )); break;
    case 2: updateModRM8(cpu_rcl(this, value, 1, 8 )); break;
    case 3: updateModRM8(cpu_rcr(this, value, 1, 8 )); break;
    case 4: updateModRM8(cpu_shl(this, value, 1, 8 )); break;
    case 5: updateModRM8(cpu_shr(this, value, 1, 8 )); break;
    case 6:
        vlog(VM_ALERT, "D0 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM8(cpu_sar(this, value, 1, 8 )); break;
    }
}

void VCpu::_wrap_0xD1_16()
{
    BYTE rm = fetchOpcodeByte();
    WORD value = readModRM16(rm);

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM16(cpu_rol(this, value, 1, 16)); break;
    case 1: updateModRM16(cpu_ror(this, value, 1, 16)); break;
    case 2: updateModRM16(cpu_rcl(this, value, 1, 16)); break;
    case 3: updateModRM16(cpu_rcr(this, value, 1, 16)); break;
    case 4: updateModRM16(cpu_shl(this, value, 1, 16)); break;
    case 5: updateModRM16(cpu_shr(this, value, 1, 16)); break;
    case 6:
        vlog(VM_ALERT, "[16bit] D1 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM16(cpu_sar(this, value, 1, 16)); break;
    }
}

void VCpu::_wrap_0xD1_32()
{
    BYTE rm = fetchOpcodeByte();
    DWORD value = readModRM32(rm);

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM32(cpu_rol(this, value, 1, 32)); break;
    case 1: updateModRM32(cpu_ror(this, value, 1, 32)); break;
    case 2: updateModRM32(cpu_rcl(this, value, 1, 32)); break;
    case 3: updateModRM32(cpu_rcr(this, value, 1, 32)); break;
    case 4: updateModRM32(cpu_shl(this, value, 1, 32)); break;
    case 5: updateModRM32(cpu_shr(this, value, 1, 32)); break;
    case 6:
        vlog(VM_ALERT, "[32bit] D1 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM32(cpu_sar(this, value, 1, 32)); break;
    }
}


void VCpu::_wrap_0xD2()
{
    BYTE rm = fetchOpcodeByte();
    BYTE value = readModRM8(rm);

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM8(cpu_rol(this, value, regs.B.CL, 8 )); break;
    case 1: updateModRM8(cpu_ror(this, value, regs.B.CL, 8 )); break;
    case 2: updateModRM8(cpu_rcl(this, value, regs.B.CL, 8 )); break;
    case 3: updateModRM8(cpu_rcr(this, value, regs.B.CL, 8 )); break;
    case 4: updateModRM8(cpu_shl(this, value, regs.B.CL, 8 )); break;
    case 5: updateModRM8(cpu_shr(this, value, regs.B.CL, 8 )); break;
    case 6:
        vlog(VM_ALERT, "D2 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM8(cpu_sar(this, value, regs.B.CL, 8 )); break;
    }
}

void VCpu::_wrap_0xD3_16()
{
    BYTE rm = fetchOpcodeByte();
    WORD value = readModRM16(rm);

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM16(cpu_rol(this, value, regs.B.CL, 16)); break;
    case 1: updateModRM16(cpu_ror(this, value, regs.B.CL, 16)); break;
    case 2: updateModRM16(cpu_rcl(this, value, regs.B.CL, 16)); break;
    case 3: updateModRM16(cpu_rcr(this, value, regs.B.CL, 16)); break;
    case 4: updateModRM16(cpu_shl(this, value, regs.B.CL, 16)); break;
    case 5: updateModRM16(cpu_shr(this, value, regs.B.CL, 16)); break;
    case 6:
        vlog(VM_ALERT, "[16bit] D3 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM16(cpu_sar(this, value, regs.B.CL, 16)); break;
    }
}

void VCpu::_wrap_0xD3_32()
{
    BYTE rm = fetchOpcodeByte();
    WORD value = readModRM32(rm);

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM32(cpu_rol(this, value, regs.B.CL, 32)); break;
    case 1: updateModRM32(cpu_ror(this, value, regs.B.CL, 32)); break;
    case 2: updateModRM32(cpu_rcl(this, value, regs.B.CL, 32)); break;
    case 3: updateModRM32(cpu_rcr(this, value, regs.B.CL, 32)); break;
    case 4: updateModRM32(cpu_shl(this, value, regs.B.CL, 32)); break;
    case 5: updateModRM32(cpu_shr(this, value, regs.B.CL, 32)); break;
    case 6:
        vlog(VM_ALERT, "[32bit] D3 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM32(cpu_sar(this, value, regs.B.CL, 32)); break;
    }
}

void VCpu::_wrap_0xF6()
{
    rmbyte = fetchOpcodeByte();

    switch (vomit_modRMRegisterPart(rmbyte)) {
    case 0: _TEST_RM8_imm8(); break;
    case 1:
        vlog(VM_ALERT, "F6 /1 not wrapped");
        exception(6);
        break;
    case 2: _NOT_RM8(); break;
    case 3: _NEG_RM8(); break;
    case 4: _MUL_RM8(); break;
    case 5: _IMUL_RM8(); break;
    case 6: _DIV_RM8(); break;
    case 7: _IDIV_RM8(); break;
    }
}

void VCpu::_wrap_0xF7_16()
{
    rmbyte = fetchOpcodeByte();

    switch (vomit_modRMRegisterPart(rmbyte)) {
    case 0: _TEST_RM16_imm16(); break;
    case 2: _NOT_RM16(); break;
    case 3: _NEG_RM16(); break;
    case 4: _MUL_RM16(); break;
    case 5: _IMUL_RM16(); break;
    case 6: _DIV_RM16(); break;
    case 7: _IDIV_RM16(); break;
    default: // 1
        vlog(VM_ALERT, "[16bit] F7 /1 not wrapped");
        exception(6);
        break;
    }
}

void VCpu::_wrap_0xF7_32()
{
    rmbyte = fetchOpcodeByte();

    switch (vomit_modRMRegisterPart(rmbyte)) {
    case 0: _TEST_RM32_imm32(); break;
    case 4: _MUL_RM32(); break;
    case 6: _DIV_RM32(); break;
#if 0
    case 2: _NOT_RM32(); break;
    case 3: _NEG_RM32(); break;
    case 5: _IMUL_RM32(); break;
    case 7: _IDIV_RM32(); break;
#endif
    default: // 1
        vlog(VM_ALERT, "[32bit] F7 /%u not wrapped", vomit_modRMRegisterPart(rmbyte));
        exception(6);
        break;

    }
}

void VCpu::_wrap_0xFE()
{
    rmbyte = fetchOpcodeByte();
    switch (vomit_modRMRegisterPart(rmbyte)) {
    case 0: _INC_RM8(); break;
    case 1: _DEC_RM8(); break;
    default:
        vlog(VM_ALERT, "FE /%u not wrapped", vomit_modRMRegisterPart(rmbyte));
        exception(6);
        break;
    }
}

void VCpu::_wrap_0xFF_16()
{
    rmbyte = fetchOpcodeByte();
    switch (vomit_modRMRegisterPart(rmbyte)) {
    case 0: _INC_RM16(); break;
    case 1: _DEC_RM16(); break;
    case 2: _CALL_RM16(); break;
    case 3: _CALL_FAR_mem16(); break;
    case 4: _JMP_RM16(); break;
    case 5: _JMP_FAR_mem16(); break;
    case 6: _PUSH_RM16(); break;
    case 7:
        vlog(VM_ALERT, "[16bit] FF /7 not wrapped");
        exception(6);
        break;
    }
}

void VCpu::_wrap_0xFF_32()
{
    vlog(VM_ALERT, "NOT IMPLEMENTED: 32bit 0xFF");
    exception(6);
#if 0
    rmbyte = fetchOpcodeByte();
    switch (vomit_modRMRegisterPart(rmbyte)) {
    case 0: _INC_RM16(); break;
    case 1: _DEC_RM16(); break;
    case 2: _CALL_RM16(); break;
    case 3: _CALL_FAR_mem16(); break;
    case 4: _JMP_RM16(); break;
    case 5: _JMP_FAR_mem16(); break;
    case 6: _PUSH_RM16(); break;
    case 7:
        vlog(VM_ALERT, "[32bit] FF /7 not wrapped");
        exception(6);
        break;
    }
#endif
}
