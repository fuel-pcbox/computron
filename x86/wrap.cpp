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
#include "debugger.h"

template<typename T>
T VCpu::doRol(T data, int steps)
{
    T result = data;
    steps &= 0x1F;

    if (steps) {
        setCF(data >> (BitSizeOfType<T>::bits - steps) & 1);
        if ((steps &= BitSizeOfType<T>::bits - 1))
            result = (data << steps) | (data >> (BitSizeOfType<T>::bits - steps));
        if (steps == 1)
            setOF(((data >> (BitSizeOfType<T>::bits - 1)) & 1) ^ getCF());
    } else
        setCF(0); // FIXME: Verify that this is correct behavior.

    return result;
}

template<typename T>
T VCpu::doRor(T data, int steps)
{
    T result = data;
    steps &= 0x1F;

    if (steps) {
        setCF((result >> (steps - 1)) & 1);
        if ((steps &= BitSizeOfType<T>::bits - 1))
            result = (data >> steps) | (data << (BitSizeOfType<T>::bits - steps));
        if (steps == 1)
            setOF((result >> (BitSizeOfType<T>::bits - 1)) ^ ((result >> (BitSizeOfType<T>::bits - 2) & 1)));
    } else
        setCF(0); // FIXME: Verify that this is correct behavior.

    return result;
}

template<typename T>
T VCpu::rightShift(T data, int steps)
{
    T result = data;

    steps &= 0x1F;

    if (steps) {
        if (steps <= BitSizeOfType<T>::bits) {
            setCF((result >> (steps - 1)) & 1);
            if (steps == 1)
                setOF((data >> (BitSizeOfType<T>::bits - 1)) & 1);
        } else
            setCF(0); // FIXME: Verify that this is correct behavior.
        result >>= steps;
    }

    if (steps == 1)
        updateFlags(result, BitSizeOfType<T>::bits);
    return result;
}

template<typename T>
T VCpu::leftShift(T data, int steps)
{
    T result = data;

    steps &= 0x1F;

    if (steps) {
        if (steps <= BitSizeOfType<T>::bits) {
            setCF(result >> (BitSizeOfType<T>::bits - steps) & 1);
            if (steps == 1)
                setOF((data >> (BitSizeOfType<T>::bits - 1)) ^ getCF());
        } else
            setCF(0); // FIXME: Verify that this is correct behavior.
        result <<= steps;
    }

    if (steps == 1)
        updateFlags(result, BitSizeOfType<T>::bits);
    return result;
}


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
        vlog(LogAlert, "[16bit] 8F /%u not wrapped", vomit_modRMRegisterPart(rmbyte));
        exception(6);
    }
}

void VCpu::_wrap_0x8F_32()
{
    rmbyte = fetchOpcodeByte();
    switch(vomit_modRMRegisterPart(rmbyte)) {
    case 0: _POP_RM32(); break;
    default:
        vlog(LogAlert, "[32bit] 8F /%u not wrapped", vomit_modRMRegisterPart(rmbyte));
        exception(6);
    }
}

void VCpu::_wrap_0xC0()
{
    BYTE rm = fetchOpcodeByte();
    BYTE value = readModRM8(rm);
    BYTE imm = fetchOpcodeByte();

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM8(doRol(value, imm)); break;
    case 1: updateModRM8(doRor(value, imm)); break;
    case 2: updateModRM8(cpu_rcl(*this, value, imm, 8)); break;
    case 3: updateModRM8(cpu_rcr(*this, value, imm, 8)); break;
    case 4: updateModRM8(leftShift(value, imm)); break;
    case 5: updateModRM8(rightShift(value, imm)); break;
    case 6:
        vlog(LogAlert, "C0 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM8(cpu_sar(*this, value, imm, 8)); break;
    }
}

void VCpu::_wrap_0xC1_16()
{
    BYTE rm = fetchOpcodeByte();
    WORD value = readModRM16(rm);
    BYTE imm = fetchOpcodeByte();

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM16(doRol(value, imm)); break;
    case 1: updateModRM16(doRor(value, imm)); break;
    case 2: updateModRM16(cpu_rcl(*this, value, imm, 16)); break;
    case 3: updateModRM16(cpu_rcr(*this, value, imm, 16)); break;
    case 4: updateModRM16(leftShift(value, imm)); break;
    case 5: updateModRM16(rightShift(value, imm)); break;
    case 6:
        vlog(LogAlert, "[16bit] C1 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM16(cpu_sar(*this, value, imm, 16)); break;
    }
}

void VCpu::_wrap_0xC1_32()
{
    BYTE rm = fetchOpcodeByte();
    DWORD value = readModRM32(rm);
    BYTE imm = fetchOpcodeByte();

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM32(doRol(value, imm)); break;
    case 1: updateModRM32(doRor(value, imm)); break;
    case 2: updateModRM32(cpu_rcl(*this, value, imm, 32)); break;
    case 3: updateModRM32(cpu_rcr(*this, value, imm, 32)); break;
    case 4: updateModRM32(leftShift(value, imm)); break;
    case 5: updateModRM32(rightShift(value, imm)); break;
    case 6:
        vlog(LogAlert, "[32bit] C1 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM32(cpu_sar(*this, value, imm, 32)); break;
    }
}

void VCpu::_wrap_0xD0()
{
    BYTE rm = fetchOpcodeByte();
    BYTE value = readModRM8(rm);

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM8(doRol(value, 1)); break;
    case 1: updateModRM8(doRor(value, 1)); break;
    case 2: updateModRM8(cpu_rcl(*this, value, 1, 8 )); break;
    case 3: updateModRM8(cpu_rcr(*this, value, 1, 8 )); break;
    case 4: updateModRM8(leftShift(value, 1)); break;
    case 5: updateModRM8(rightShift(value, 1)); break;
    case 6:
        vlog(LogAlert, "D0 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM8(cpu_sar(*this, value, 1, 8 )); break;
    }
}

void VCpu::_wrap_0xD1_16()
{
    BYTE rm = fetchOpcodeByte();
    WORD value = readModRM16(rm);

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM16(doRol(value, 1)); break;
    case 1: updateModRM16(doRor(value, 1)); break;
    case 2: updateModRM16(cpu_rcl(*this, value, 1, 16)); break;
    case 3: updateModRM16(cpu_rcr(*this, value, 1, 16)); break;
    case 4: updateModRM16(leftShift(value, 1)); break;
    case 5: updateModRM16(rightShift(value, 1)); break;
    case 6:
        vlog(LogAlert, "[16bit] D1 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM16(cpu_sar(*this, value, 1, 16)); break;
    }
}

void VCpu::_wrap_0xD1_32()
{
    BYTE rm = fetchOpcodeByte();
    DWORD value = readModRM32(rm);

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM32(doRol(value, 1)); break;
    case 1: updateModRM32(doRor(value, 1)); break;
    case 2: updateModRM32(cpu_rcl(*this, value, 1, 32)); break;
    case 3: updateModRM32(cpu_rcr(*this, value, 1, 32)); break;
    case 4: updateModRM32(leftShift(value, 1)); break;
    case 5: updateModRM32(rightShift(value, 1)); break;
    case 6:
        vlog(LogAlert, "[32bit] D1 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM32(cpu_sar(*this, value, 1, 32)); break;
    }
}


void VCpu::_wrap_0xD2()
{
    BYTE rm = fetchOpcodeByte();
    BYTE value = readModRM8(rm);

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM8(doRol(value, regs.B.CL)); break;
    case 1: updateModRM8(doRor(value, regs.B.CL)); break;
    case 2: updateModRM8(cpu_rcl(*this, value, regs.B.CL, 8 )); break;
    case 3: updateModRM8(cpu_rcr(*this, value, regs.B.CL, 8 )); break;
    case 4: updateModRM8(leftShift(value, regs.B.CL)); break;
    case 5: updateModRM8(rightShift(value, regs.B.CL)); break;
    case 6:
        vlog(LogAlert, "D2 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM8(cpu_sar(*this, value, regs.B.CL, 8 )); break;
    }
}

void VCpu::_wrap_0xD3_16()
{
    BYTE rm = fetchOpcodeByte();
    WORD value = readModRM16(rm);

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM16(doRol(value, regs.B.CL)); break;
    case 1: updateModRM16(doRor(value, regs.B.CL)); break;
    case 2: updateModRM16(cpu_rcl(*this, value, regs.B.CL, 16)); break;
    case 3: updateModRM16(cpu_rcr(*this, value, regs.B.CL, 16)); break;
    case 4: updateModRM16(leftShift(value, regs.B.CL)); break;
    case 5: updateModRM16(rightShift(value, regs.B.CL)); break;
    case 6:
        vlog(LogAlert, "[16bit] D3 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM16(cpu_sar(*this, value, regs.B.CL, 16)); break;
    }
}

void VCpu::_wrap_0xD3_32()
{
    BYTE rm = fetchOpcodeByte();
    DWORD value = readModRM32(rm);

    switch (vomit_modRMRegisterPart(rm)) {
    case 0: updateModRM32(doRol(value, regs.B.CL)); break;
    case 1: updateModRM32(doRor(value, regs.B.CL)); break;
    case 2: updateModRM32(cpu_rcl(*this, value, regs.B.CL, 32)); break;
    case 3: updateModRM32(cpu_rcr(*this, value, regs.B.CL, 32)); break;
    case 4: updateModRM32(leftShift(value, regs.B.CL)); break;
    case 5: updateModRM32(rightShift(value, regs.B.CL)); break;
    case 6:
        vlog(LogAlert, "[32bit] D3 /6 not wrapped");
        exception(6);
        break;
    case 7: updateModRM32(cpu_sar(*this, value, regs.B.CL, 32)); break;
    }
}

void VCpu::_wrap_0xF6()
{
    rmbyte = fetchOpcodeByte();

    switch (vomit_modRMRegisterPart(rmbyte)) {
    case 0: _TEST_RM8_imm8(); break;
    case 1:
        vlog(LogAlert, "F6 /1 not wrapped");
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
        vlog(LogAlert, "[16bit] F7 /1 not wrapped");
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
    case 2: _NOT_RM32(); break;
    case 3: _NEG_RM32(); break;
#if 0
    case 5: _IMUL_RM32(); break;
#endif
    case 7: _IDIV_RM32(); break;
    default: // 1
        vlog(LogAlert, "[32bit] F7 /%u not wrapped", vomit_modRMRegisterPart(rmbyte));
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
        vlog(LogAlert, "FE /%u not wrapped", vomit_modRMRegisterPart(rmbyte));
        exception(6);
        break;
    }
}

void VCpu::_wrap_0xFF()
{
    rmbyte = fetchOpcodeByte();
    switch (vomit_modRMRegisterPart(rmbyte)) {
    case 0: CALL_HANDLER(_INC_RM16, _INC_RM32); break;
    case 1: CALL_HANDLER(_DEC_RM16, _DEC_RM32); break;
    case 2: CALL_HANDLER(_CALL_RM16, _CALL_RM32); break;
    case 3: CALL_HANDLER(_CALL_FAR_mem16, _CALL_FAR_mem32); break;
    case 4: CALL_HANDLER(_JMP_RM16, _JMP_RM32); break;
    case 5: CALL_HANDLER(_JMP_FAR_mem16, _JMP_FAR_mem32); break;
    case 6: CALL_HANDLER(_PUSH_RM16, _PUSH_RM32); break;
    case 7:
        vlog(LogAlert, "FF /7 not wrapped");
        exception(6);
        break;
    }
}
