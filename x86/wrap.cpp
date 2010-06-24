// x86/wrap.cpp
// Multibyte opcode wrappers

#include "vcpu.h"
#include "debug.h"

void _wrap_0x80(VCpu* cpu)
{
    cpu->rmbyte = cpu->fetchOpcodeByte();
    switch(vomit_modRMRegisterPart(cpu->rmbyte)) {
    case 0: _ADD_RM8_imm8(cpu); break;
    case 1:  _OR_RM8_imm8(cpu); break;
    case 2: _ADC_RM8_imm8(cpu); break;
    case 3: _SBB_RM8_imm8(cpu); break;
    case 4: _AND_RM8_imm8(cpu); break;
    case 5: _SUB_RM8_imm8(cpu); break;
    case 6: _XOR_RM8_imm8(cpu); break;
    case 7: _CMP_RM8_imm8(cpu); break;
    }
}

void _wrap_0x81_16(VCpu* cpu)
{
    cpu->rmbyte = cpu->fetchOpcodeByte();
    switch(vomit_modRMRegisterPart(cpu->rmbyte)) {
    case 0: _ADD_RM16_imm16(cpu); break;
    case 1:  _OR_RM16_imm16(cpu); break;
    case 2: _ADC_RM16_imm16(cpu); break;
    case 3: _SBB_RM16_imm16(cpu); break;
    case 4: _AND_RM16_imm16(cpu); break;
    case 5: _SUB_RM16_imm16(cpu); break;
    case 6: _XOR_RM16_imm16(cpu); break;
    case 7: _CMP_RM16_imm16(cpu); break;
    }
}

void _wrap_0x81_32(VCpu* cpu)
{
    cpu->rmbyte = cpu->fetchOpcodeByte();
    switch(vomit_modRMRegisterPart(cpu->rmbyte)) {
    case 0: _ADD_RM32_imm32(cpu); break;
    case 1:  _OR_RM32_imm32(cpu); break;
    case 2: _ADC_RM32_imm32(cpu); break;
    case 3: _SBB_RM32_imm32(cpu); break;
    case 4: _AND_RM32_imm32(cpu); break;
    case 5: _SUB_RM32_imm32(cpu); break;
    case 6: _XOR_RM32_imm32(cpu); break;
    case 7: _CMP_RM32_imm32(cpu); break;
    }
}

void _wrap_0x83(VCpu* cpu)
{
    cpu->rmbyte = cpu->fetchOpcodeByte();
    switch(vomit_modRMRegisterPart(cpu->rmbyte)) {
    case 0: _ADD_RM16_imm8(cpu); break;
    case 1:  _OR_RM16_imm8(cpu); break;
    case 2: _ADC_RM16_imm8(cpu); break;
    case 3: _SBB_RM16_imm8(cpu); break;
    case 4: _AND_RM16_imm8(cpu); break;
    case 5: _SUB_RM16_imm8(cpu); break;
    case 6: _XOR_RM16_imm8(cpu); break;
    case 7: _CMP_RM16_imm8(cpu); break;
    }
}

void _wrap_0x8F(VCpu* cpu)
{
    cpu->rmbyte = cpu->fetchOpcodeByte();
    switch(vomit_modRMRegisterPart(cpu->rmbyte)) {
    case 0: _POP_RM16(cpu); break;
    default:
        vlog(VM_ALERT, "8F /%u not wrapped", vomit_modRMRegisterPart(cpu->rmbyte));
        cpu->exception(6);
    }
}

void _wrap_0xC0(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    BYTE value = cpu->readModRM8(rm);
    BYTE imm = cpu->fetchOpcodeByte();

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: cpu->updateModRM8(cpu_rol(cpu, value, imm, 8)); break;
    case 1: cpu->updateModRM8(cpu_ror(cpu, value, imm, 8)); break;
    case 2: cpu->updateModRM8(cpu_rcl(cpu, value, imm, 8)); break;
    case 3: cpu->updateModRM8(cpu_rcr(cpu, value, imm, 8)); break;
    case 4: cpu->updateModRM8(cpu_shl(cpu, value, imm, 8)); break;
    case 5: cpu->updateModRM8(cpu_shr(cpu, value, imm, 8)); break;
    case 6:
        vlog(VM_ALERT, "C0 /6 not wrapped");
        cpu->exception(6);
        break;
    case 7: cpu->updateModRM8(cpu_sar(cpu, value, imm, 8)); break;
    }
}

void _wrap_0xC1(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    WORD value = cpu->readModRM16(rm );
    BYTE imm = cpu->fetchOpcodeByte();

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: cpu->updateModRM16(cpu_rol(cpu, value, imm, 16)); break;
    case 1: cpu->updateModRM16(cpu_ror(cpu, value, imm, 16)); break;
    case 2: cpu->updateModRM16(cpu_rcl(cpu, value, imm, 16)); break;
    case 3: cpu->updateModRM16(cpu_rcr(cpu, value, imm, 16)); break;
    case 4: cpu->updateModRM16(cpu_shl(cpu, value, imm, 16)); break;
    case 5: cpu->updateModRM16(cpu_shr(cpu, value, imm, 16)); break;
    case 6:
        vlog(VM_ALERT, "C1 /6 not wrapped");
        cpu->exception(6);
        break;
    case 7: cpu->updateModRM16(cpu_sar(cpu, value, imm, 16)); break;
    }
}

void _wrap_0xD0(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    BYTE value = cpu->readModRM8(rm);

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: cpu->updateModRM8(cpu_rol(cpu, value, 1, 8 )); break;
    case 1: cpu->updateModRM8(cpu_ror(cpu, value, 1, 8 )); break;
    case 2: cpu->updateModRM8(cpu_rcl(cpu, value, 1, 8 )); break;
    case 3: cpu->updateModRM8(cpu_rcr(cpu, value, 1, 8 )); break;
    case 4: cpu->updateModRM8(cpu_shl(cpu, value, 1, 8 )); break;
    case 5: cpu->updateModRM8(cpu_shr(cpu, value, 1, 8 )); break;
    case 6:
        vlog(VM_ALERT, "D0 /6 not wrapped");
        cpu->exception(6);
        break;
    case 7: cpu->updateModRM8(cpu_sar(cpu, value, 1, 8 )); break;
    }
}

void _wrap_0xD1_16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    WORD value = cpu->readModRM16(rm);

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: cpu->updateModRM16(cpu_rol(cpu, value, 1, 16)); break;
    case 1: cpu->updateModRM16(cpu_ror(cpu, value, 1, 16)); break;
    case 2: cpu->updateModRM16(cpu_rcl(cpu, value, 1, 16)); break;
    case 3: cpu->updateModRM16(cpu_rcr(cpu, value, 1, 16)); break;
    case 4: cpu->updateModRM16(cpu_shl(cpu, value, 1, 16)); break;
    case 5: cpu->updateModRM16(cpu_shr(cpu, value, 1, 16)); break;
    case 6:
        vlog(VM_ALERT, "[16bit] D1 /6 not wrapped");
        cpu->exception(6);
        break;
    case 7: cpu->updateModRM16(cpu_sar(cpu, value, 1, 16)); break;
    }
}

void _wrap_0xD1_32(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    DWORD value = cpu->readModRM32(rm);

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: cpu->updateModRM32(cpu_rol(cpu, value, 1, 32)); break;
    case 1: cpu->updateModRM32(cpu_ror(cpu, value, 1, 32)); break;
    case 2: cpu->updateModRM32(cpu_rcl(cpu, value, 1, 32)); break;
    case 3: cpu->updateModRM32(cpu_rcr(cpu, value, 1, 32)); break;
    case 4: cpu->updateModRM32(cpu_shl(cpu, value, 1, 32)); break;
    case 5: cpu->updateModRM32(cpu_shr(cpu, value, 1, 32)); break;
    case 6:
        vlog(VM_ALERT, "[32bit] D1 /6 not wrapped");
        cpu->exception(6);
        break;
    case 7: cpu->updateModRM32(cpu_sar(cpu, value, 1, 32)); break;
    }
}


void _wrap_0xD2(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    BYTE value = cpu->readModRM8(rm);

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: cpu->updateModRM8(cpu_rol(cpu, value, cpu->regs.B.CL, 8 )); break;
    case 1: cpu->updateModRM8(cpu_ror(cpu, value, cpu->regs.B.CL, 8 )); break;
    case 2: cpu->updateModRM8(cpu_rcl(cpu, value, cpu->regs.B.CL, 8 )); break;
    case 3: cpu->updateModRM8(cpu_rcr(cpu, value, cpu->regs.B.CL, 8 )); break;
    case 4: cpu->updateModRM8(cpu_shl(cpu, value, cpu->regs.B.CL, 8 )); break;
    case 5: cpu->updateModRM8(cpu_shr(cpu, value, cpu->regs.B.CL, 8 )); break;
    case 6:
        vlog(VM_ALERT, "D2 /6 not wrapped");
        cpu->exception(6);
        break;
    case 7: cpu->updateModRM8(cpu_sar(cpu, value, cpu->regs.B.CL, 8 )); break;
    }
}

void _wrap_0xD3(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    WORD value = cpu->readModRM16(rm);

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: cpu->updateModRM16(cpu_rol(cpu, value, cpu->regs.B.CL, 16)); break;
    case 1: cpu->updateModRM16(cpu_ror(cpu, value, cpu->regs.B.CL, 16)); break;
    case 2: cpu->updateModRM16(cpu_rcl(cpu, value, cpu->regs.B.CL, 16)); break;
    case 3: cpu->updateModRM16(cpu_rcr(cpu, value, cpu->regs.B.CL, 16)); break;
    case 4: cpu->updateModRM16(cpu_shl(cpu, value, cpu->regs.B.CL, 16)); break;
    case 5: cpu->updateModRM16(cpu_shr(cpu, value, cpu->regs.B.CL, 16)); break;
    case 6:
        vlog(VM_ALERT, "D3 /6 not wrapped");
        cpu->exception(6);
        break;
    case 7: cpu->updateModRM16(cpu_sar(cpu, value, cpu->regs.B.CL, 16)); break;
    }
}

void _wrap_0xF6(VCpu* cpu)
{
    cpu->rmbyte = cpu->fetchOpcodeByte();

    switch (vomit_modRMRegisterPart(cpu->rmbyte)) {
    case 0: _TEST_RM8_imm8(cpu); break;
    case 1:
        vlog(VM_ALERT, "F6 /1 not wrapped");
        cpu->exception(6);
        break;
    case 2: _NOT_RM8(cpu); break;
    case 3: _NEG_RM8(cpu); break;
    case 4: _MUL_RM8(cpu); break;
    case 5: _IMUL_RM8(cpu); break;
    case 6: _DIV_RM8(cpu); break;
    case 7: _IDIV_RM8(cpu); break;
    }
}

void _wrap_0xF7_16(VCpu* cpu)
{
    cpu->rmbyte = cpu->fetchOpcodeByte();

    switch (vomit_modRMRegisterPart(cpu->rmbyte)) {
    case 0: _TEST_RM16_imm16(cpu); break;
    case 2: _NOT_RM16(cpu); break;
    case 3: _NEG_RM16(cpu); break;
    case 4: _MUL_RM16(cpu); break;
    case 5: _IMUL_RM16(cpu); break;
    case 6: _DIV_RM16(cpu); break;
    case 7: _IDIV_RM16(cpu); break;
    default: // 1
        vlog(VM_ALERT, "[16bit] F7 /1 not wrapped");
        cpu->exception(6);
        break;
    }
}

void _wrap_0xF7_32(VCpu* cpu)
{
    cpu->rmbyte = cpu->fetchOpcodeByte();

    switch (vomit_modRMRegisterPart(cpu->rmbyte)) {
    case 0: _TEST_RM32_imm32(cpu); break;
#if 0
    case 2: _NOT_RM32(cpu); break;
    case 3: _NEG_RM32(cpu); break;
    case 4: _MUL_RM32(cpu); break;
    case 5: _IMUL_RM32(cpu); break;
    case 6: _DIV_RM32(cpu); break;
    case 7: _IDIV_RM32(cpu); break;
#endif
    default: // 1
        vlog(VM_ALERT, "[32bit] F7 /1 not wrapped");
        cpu->exception(6);
        break;

    }
}

void _wrap_0xFE(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    BYTE value = cpu->readModRM8(rm);

    WORD i = value;

    switch (vomit_modRMRegisterPart(rm)) {
    case 0:
        cpu->setOF(value == 0x7F);
        i++;
        vomit_cpu_setAF(cpu, i, value, 1);
        cpu->updateFlags8(i);
        cpu->updateModRM8(value + 1);
        break;
    case 1:
        cpu->setOF(value == 0x80);
        i--;
        vomit_cpu_setAF(cpu, i, value, 1);
        cpu->updateFlags8(i);
        cpu->updateModRM8(value - 1);
        break;
    default:
        vlog(VM_ALERT, "FE /%u not wrapped", vomit_modRMRegisterPart(rm));
        cpu->exception(6);
        break;
    }
}

void _wrap_0xFF(VCpu* cpu)
{
    cpu->rmbyte = cpu->fetchOpcodeByte();
    switch (vomit_modRMRegisterPart(cpu->rmbyte)) {
    case 0: _INC_RM16(cpu); break;
    case 1: _DEC_RM16(cpu); break;
    case 2: _CALL_RM16(cpu); break;
    case 3: _CALL_FAR_mem16(cpu); break;
    case 4: _JMP_RM16(cpu); break;
    case 5: _JMP_FAR_mem16(cpu); break;
    case 6: _PUSH_RM16(cpu); break;
    case 7:
        vlog(VM_ALERT, "FF /7 not wrapped");
        cpu->exception(6);
        break;
    }
}
