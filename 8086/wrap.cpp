/* 8086/wrap.c
 * ModR/M Instruction Wrappers
 *
 */

#include "8086.h"
#include "debug.h"

void _wrap_0x80(VCpu* cpu)
{
    cpu->rmbyte = cpu->fetchOpcodeByte();
    switch(rmreg(cpu->rmbyte)) {
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

void _wrap_0x81(VCpu* cpu)
{
    cpu->rmbyte = cpu->fetchOpcodeByte();
    switch(rmreg(cpu->rmbyte)) {
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

void _wrap_0x83(VCpu* cpu)
{
    cpu->rmbyte = cpu->fetchOpcodeByte();
    switch(rmreg(cpu->rmbyte)) {
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
    switch(rmreg(cpu->rmbyte)) {
    case 0: _POP_RM16(cpu); break;
    default:
        vlog( VM_ALERT, "8F /%d not wrapped", rmreg(cpu->rmbyte));
    }
}

void _wrap_0xC0(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    BYTE value = vomit_cpu_modrm_read8(cpu, rm);
    BYTE imm = cpu->fetchOpcodeByte();

    switch (rmreg(rm)) {
    case 0: vomit_cpu_modrm_update8(cpu, cpu_rol(cpu, value, imm, 8)); break;
    case 1: vomit_cpu_modrm_update8(cpu, cpu_ror(cpu, value, imm, 8)); break;
    case 2: vomit_cpu_modrm_update8(cpu, cpu_rcl(cpu, value, imm, 8)); break;
    case 3: vomit_cpu_modrm_update8(cpu, cpu_rcr(cpu, value, imm, 8)); break;
    case 4: vomit_cpu_modrm_update8(cpu, cpu_shl(cpu, value, imm, 8)); break;
    case 5: vomit_cpu_modrm_update8(cpu, cpu_shr(cpu, value, imm, 8)); break;
    case 7: vomit_cpu_modrm_update8(cpu, cpu_sar(cpu, value, imm, 8)); break;
    default: vlog( VM_ALERT, "C0 /%d not wrapped", rmreg(rm));
    }
}

void _wrap_0xC1(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    WORD value = vomit_cpu_modrm_read16(cpu, rm );
    BYTE imm = cpu->fetchOpcodeByte();

    switch (rmreg(rm)) {
    case 0: vomit_cpu_modrm_update16(cpu, cpu_rol(cpu, value, imm, 16)); break;
    case 1: vomit_cpu_modrm_update16(cpu, cpu_ror(cpu, value, imm, 16)); break;
    case 2: vomit_cpu_modrm_update16(cpu, cpu_rcl(cpu, value, imm, 16)); break;
    case 3: vomit_cpu_modrm_update16(cpu, cpu_rcr(cpu, value, imm, 16)); break;
    case 4: vomit_cpu_modrm_update16(cpu, cpu_shl(cpu, value, imm, 16)); break;
    case 5: vomit_cpu_modrm_update16(cpu, cpu_shr(cpu, value, imm, 16)); break;
    case 7: vomit_cpu_modrm_update16(cpu, cpu_sar(cpu, value, imm, 16)); break;
    default:
        vlog(VM_ALERT, "C1 /%d not wrapped", rmreg(rm));
    }
}

void _wrap_0xD0(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    BYTE value = vomit_cpu_modrm_read8(cpu, rm);

    switch (rmreg(rm)) {
    case 0: vomit_cpu_modrm_update8(cpu, cpu_rol(cpu, value, 1, 8 )); break;
    case 1: vomit_cpu_modrm_update8(cpu, cpu_ror(cpu, value, 1, 8 )); break;
    case 2: vomit_cpu_modrm_update8(cpu, cpu_rcl(cpu, value, 1, 8 )); break;
    case 3: vomit_cpu_modrm_update8(cpu, cpu_rcr(cpu, value, 1, 8 )); break;
    case 4: vomit_cpu_modrm_update8(cpu, cpu_shl(cpu, value, 1, 8 )); break;
    case 5: vomit_cpu_modrm_update8(cpu, cpu_shr(cpu, value, 1, 8 )); break;
    case 7: vomit_cpu_modrm_update8(cpu, cpu_sar(cpu, value, 1, 8 )); break;
    default:
        vlog(VM_ALERT, "D0 /%d not wrapped", rmreg(rm));
    }
}

void _wrap_0xD1(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    WORD value = vomit_cpu_modrm_read16(cpu, rm);

    switch (rmreg(rm)) {
    case 0: vomit_cpu_modrm_update16(cpu, cpu_rol(cpu, value, 1, 16 )); break;
    case 1: vomit_cpu_modrm_update16(cpu, cpu_ror(cpu, value, 1, 16 )); break;
    case 2: vomit_cpu_modrm_update16(cpu, cpu_rcl(cpu, value, 1, 16 )); break;
    case 3: vomit_cpu_modrm_update16(cpu, cpu_rcr(cpu, value, 1, 16 )); break;
    case 4: vomit_cpu_modrm_update16(cpu, cpu_shl(cpu, value, 1, 16 )); break;
    case 5: vomit_cpu_modrm_update16(cpu, cpu_shr(cpu, value, 1, 16 )); break;
    case 7: vomit_cpu_modrm_update16(cpu, cpu_sar(cpu, value, 1, 16 )); break;
    default:
        vlog(VM_ALERT, "D1 /%d not wrapped", rmreg(rm));
    }
}

void _wrap_0xD2(VCpu* cpu)
{
    byte rm = cpu->fetchOpcodeByte();
    byte value = vomit_cpu_modrm_read8(cpu, rm);

    switch (rmreg(rm)) {
    case 0: vomit_cpu_modrm_update8(cpu, cpu_rol(cpu, value, cpu->regs.B.CL, 8 )); break;
    case 1: vomit_cpu_modrm_update8(cpu, cpu_ror(cpu, value, cpu->regs.B.CL, 8 )); break;
    case 2: vomit_cpu_modrm_update8(cpu, cpu_rcl(cpu, value, cpu->regs.B.CL, 8 )); break;
    case 3: vomit_cpu_modrm_update8(cpu, cpu_rcr(cpu, value, cpu->regs.B.CL, 8 )); break;
    case 4: vomit_cpu_modrm_update8(cpu, cpu_shl(cpu, value, cpu->regs.B.CL, 8 )); break;
    case 5: vomit_cpu_modrm_update8(cpu, cpu_shr(cpu, value, cpu->regs.B.CL, 8 )); break;
    case 7: vomit_cpu_modrm_update8(cpu, cpu_sar(cpu, value, cpu->regs.B.CL, 8 )); break;
    default:
        vlog(VM_ALERT, "D2 /%d not wrapped", rmreg(rm));
    }
}

void _wrap_0xD3(VCpu* cpu)
{
    byte rm = cpu->fetchOpcodeByte();
    word value = vomit_cpu_modrm_read16(cpu, rm);

    switch (rmreg(rm)) {
    case 0: vomit_cpu_modrm_update16(cpu, cpu_rol(cpu, value, cpu->regs.B.CL, 16)); break;
    case 1: vomit_cpu_modrm_update16(cpu, cpu_ror(cpu, value, cpu->regs.B.CL, 16)); break;
    case 2: vomit_cpu_modrm_update16(cpu, cpu_rcl(cpu, value, cpu->regs.B.CL, 16)); break;
    case 3: vomit_cpu_modrm_update16(cpu, cpu_rcr(cpu, value, cpu->regs.B.CL, 16)); break;
    case 4: vomit_cpu_modrm_update16(cpu, cpu_shl(cpu, value, cpu->regs.B.CL, 16)); break;
    case 5: vomit_cpu_modrm_update16(cpu, cpu_shr(cpu, value, cpu->regs.B.CL, 16)); break;
    case 7: vomit_cpu_modrm_update16(cpu, cpu_sar(cpu, value, cpu->regs.B.CL, 16)); break;
    default:
        vlog(VM_ALERT, "D3 /%d not wrapped", rmreg(rm));
    }
}

void _wrap_0xF6(VCpu* cpu)
{
    cpu->rmbyte = cpu->fetchOpcodeByte();

    switch (rmreg(cpu->rmbyte)) {
    case 0: _TEST_RM8_imm8(cpu); break;
    case 2: _NOT_RM8(cpu); break;
    case 3: _NEG_RM8(cpu); break;
    case 4: _MUL_RM8(cpu); break;
    case 5: _IMUL_RM8(cpu); break;
    case 6: _DIV_RM8(cpu); break;
    case 7: _IDIV_RM8(cpu); break;
    default:
        vlog( VM_ALERT, "F6 /%d not wrapped", rmreg(cpu->rmbyte));
    }
}

void _wrap_0xF7(VCpu* cpu)
{
    cpu->rmbyte = cpu->fetchOpcodeByte();

    switch (rmreg(cpu->rmbyte)) {
    case 0: _TEST_RM16_imm16(cpu); break;
    case 2: _NOT_RM16(cpu); break;
    case 3: _NEG_RM16(cpu); break;
    case 4: _MUL_RM16(cpu); break;
    case 5: _IMUL_RM16(cpu); break;
    case 6: _DIV_RM16(cpu); break;
    case 7: _IDIV_RM16(cpu); break;
    default:
        vlog(VM_ALERT, "F7 /%d not wrapped", rmreg(cpu->rmbyte));
    }
}


void _wrap_0xFE(VCpu* cpu)
{
    byte rm = cpu->fetchOpcodeByte();
    byte value = vomit_cpu_modrm_read8(cpu, rm);

    word i = value;

    switch (rmreg(rm)) {
    case 0:
        cpu->setOF(value == 0x7F);
        i++;
        vomit_cpu_setAF(cpu, i, value, 1);
        cpu->updateFlags8(i);
        vomit_cpu_modrm_update8(cpu, value + 1);
        break;
    case 1:
        cpu->setOF(value == 0x80);
        i--;
        vomit_cpu_setAF(cpu, i, value, 1);
        cpu->updateFlags8(i);
        vomit_cpu_modrm_update8(cpu, value - 1);
        break;
    default:
        vlog(VM_ALERT, "FE /%d not wrapped", rmreg(rm));
    }
}

void _wrap_0xFF(VCpu* cpu)
{
    cpu->rmbyte = cpu->fetchOpcodeByte();
    switch (rmreg(cpu->rmbyte)) {
    case 0: _INC_RM16(cpu); break;
    case 1: _DEC_RM16(cpu); break;
    case 2: _CALL_RM16(cpu); break;
    case 3: _CALL_FAR_mem16(cpu); break;
    case 4: _JMP_RM16(cpu); break;
    case 5: _JMP_FAR_mem16(cpu); break;
    case 6: _PUSH_RM16(cpu); break;
    case 7: vlog( VM_ALERT, "FF /%d not wrapped", rmreg( cpu->rmbyte ));
    }
}
