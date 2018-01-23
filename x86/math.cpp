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
#include "templates.h"

template<typename T>
inline void updateCpuMathFlags(VCpu* cpu, QWORD result, T dest, T src)
{
    if (BitSizeOfType<T>::bits == 8)
        cpu->mathFlags8(result, dest, src);
    else if (BitSizeOfType<T>::bits == 16)
        cpu->mathFlags16(result, dest, src);
    else if (BitSizeOfType<T>::bits == 32)
        cpu->mathFlags32(result, dest, src);
}

template<typename T>
inline void updateCpuCmpFlags(VCpu* cpu, QWORD result, T dest, T src)
{
    if (BitSizeOfType<T>::bits == 8)
        cpu->cmpFlags8(result, dest, src);
    else if (BitSizeOfType<T>::bits == 16)
        cpu->cmpFlags16(result, dest, src);
    else if (BitSizeOfType<T>::bits == 32)
        cpu->cmpFlags32(result, dest, src);
}

template<typename T>
QWORD VCpu::doAdd(T dest, T src)
{
    QWORD result = (QWORD)dest + (QWORD)src;
    updateCpuMathFlags(this, result, dest, src);
    setOF(((
          ((result)^(dest)) &
          ((result)^(src))
         )>>(BitSizeOfType<T>::bits - 1))&1);
    return result;
}

template<typename T>
QWORD VCpu::doAdc(T dest, T src)
{
    QWORD result;
    src += getCF();
    result = (QWORD)dest + (QWORD)src;

    updateCpuMathFlags(this, result, dest, src);
    setOF(((
          ((result)^(dest)) &
          ((result)^(src))
         )>>(BitSizeOfType<T>::bits))&1);
    return result;
}

template<typename T>
QWORD VCpu::doSub(T dest, T src)
{
    QWORD result = (QWORD)dest - (QWORD)src;
    updateCpuCmpFlags(this, result, dest, src);
    return result;
}

template<typename T>
QWORD VCpu::doSbb(T dest, T src)
{
    QWORD result;
    src += getCF();
    result = (QWORD)dest - (QWORD)src;
    updateCpuCmpFlags(this, result, dest, src);
    return result;
}

template<typename T>
QWORD VCpu::doMul(T acc, T multi)
{
    QWORD result = (DWORD)acc * (DWORD)multi;
    updateCpuMathFlags(this, result, acc, multi);
    return result;
}

template<typename T>
SIGNED_QWORD VCpu::doImul(T acc, T multi)
{
    // FIXME: This function should protect against T being an unsigned type.
    SIGNED_QWORD result = (SIGNED_QWORD)acc * (SIGNED_QWORD)multi;
    return result;
}

DEFAULT_RM8_reg8(doAdd, _ADD_RM8_reg8)
DEFAULT_RM16_reg16(doAdd, _ADD_RM16_reg16)
DEFAULT_RM32_reg32(doAdd, _ADD_RM32_reg32)
DEFAULT_reg8_RM8(doAdd, _ADD_reg8_RM8)
DEFAULT_reg16_RM16(doAdd, _ADD_reg16_RM16)
DEFAULT_reg32_RM32(doAdd, _ADD_reg32_RM32)
DEFAULT_RM8_imm8(doAdd, _ADD_RM8_imm8)
DEFAULT_RM16_imm16(doAdd, _ADD_RM16_imm16)
DEFAULT_RM32_imm32(doAdd, _ADD_RM32_imm32)
DEFAULT_RM16_imm8(doAdd, _ADD_RM16_imm8)
DEFAULT_RM32_imm8(doAdd, _ADD_RM32_imm8)
DEFAULT_AL_imm8(doAdd, _ADD_AL_imm8)
DEFAULT_AX_imm16(doAdd, _ADD_AX_imm16)
DEFAULT_EAX_imm32(doAdd, _ADD_EAX_imm32)

DEFAULT_RM8_reg8(doAdc, _ADC_RM8_reg8)
DEFAULT_RM16_reg16(doAdc, _ADC_RM16_reg16)
DEFAULT_RM32_reg32(doAdc, _ADC_RM32_reg32)
DEFAULT_reg8_RM8(doAdc, _ADC_reg8_RM8)
DEFAULT_reg16_RM16(doAdc, _ADC_reg16_RM16)
DEFAULT_reg32_RM32(doAdc, _ADC_reg32_RM32)
DEFAULT_RM8_imm8(doAdc, _ADC_RM8_imm8)
DEFAULT_RM16_imm16(doAdc, _ADC_RM16_imm16)
DEFAULT_RM32_imm32(doAdc, _ADC_RM32_imm32)
DEFAULT_RM16_imm8(doAdc, _ADC_RM16_imm8)
DEFAULT_RM32_imm8(doAdc, _ADC_RM32_imm8)
DEFAULT_AL_imm8(doAdc, _ADC_AL_imm8)
DEFAULT_AX_imm16(doAdc, _ADC_AX_imm16)
DEFAULT_EAX_imm32(doAdc, _ADC_EAX_imm32)

DEFAULT_RM8_reg8(doSub, _SUB_RM8_reg8)
DEFAULT_RM16_reg16(doSub, _SUB_RM16_reg16)
DEFAULT_RM32_reg32(doSub, _SUB_RM32_reg32)
DEFAULT_reg8_RM8(doSub, _SUB_reg8_RM8)
DEFAULT_reg16_RM16(doSub, _SUB_reg16_RM16)
DEFAULT_reg32_RM32(doSub, _SUB_reg32_RM32)
DEFAULT_RM8_imm8(doSub, _SUB_RM8_imm8)
DEFAULT_RM16_imm16(doSub, _SUB_RM16_imm16)
DEFAULT_RM32_imm32(doSub, _SUB_RM32_imm32)
DEFAULT_RM16_imm8(doSub, _SUB_RM16_imm8)
DEFAULT_RM32_imm8(doSub, _SUB_RM32_imm8)
DEFAULT_AL_imm8(doSub, _SUB_AL_imm8)
DEFAULT_AX_imm16(doSub, _SUB_AX_imm16)
DEFAULT_EAX_imm32(doSub, _SUB_EAX_imm32)

DEFAULT_RM8_reg8(doSbb, _SBB_RM8_reg8)
DEFAULT_RM16_reg16(doSbb, _SBB_RM16_reg16)
DEFAULT_RM32_reg32(doSbb, _SBB_RM32_reg32)
DEFAULT_reg8_RM8(doSbb, _SBB_reg8_RM8)
DEFAULT_reg16_RM16(doSbb, _SBB_reg16_RM16)
DEFAULT_reg32_RM32(doSbb, _SBB_reg32_RM32)
DEFAULT_RM8_imm8(doSbb, _SBB_RM8_imm8)
DEFAULT_RM16_imm16(doSbb, _SBB_RM16_imm16)
DEFAULT_RM32_imm32(doSbb, _SBB_RM32_imm32)
DEFAULT_RM16_imm8(doSbb, _SBB_RM16_imm8)
DEFAULT_RM32_imm8(doSbb, _SBB_RM32_imm8)
DEFAULT_AL_imm8(doSbb, _SBB_AL_imm8)
DEFAULT_AX_imm16(doSbb, _SBB_AX_imm16)
DEFAULT_EAX_imm32(doSbb, _SBB_EAX_imm32)

READONLY_RM8_reg8(doSub, _CMP_RM8_reg8)
READONLY_RM16_reg16(doSub, _CMP_RM16_reg16)
READONLY_RM32_reg32(doSub, _CMP_RM32_reg32)
READONLY_reg8_RM8(doSub, _CMP_reg8_RM8)
READONLY_reg16_RM16(doSub, _CMP_reg16_RM16)
READONLY_reg32_RM32(doSub, _CMP_reg32_RM32)
READONLY_RM8_imm8(doSub, _CMP_RM8_imm8)
READONLY_RM16_imm16(doSub, _CMP_RM16_imm16)
READONLY_RM32_imm32(doSub, _CMP_RM32_imm32)
READONLY_RM16_imm8(doSub, _CMP_RM16_imm8)
READONLY_RM32_imm8(doSub, _CMP_RM32_imm8)
READONLY_AL_imm8(doSub, _CMP_AL_imm8)
READONLY_AX_imm16(doSub, _CMP_AX_imm16)
READONLY_EAX_imm32(doSub, _CMP_EAX_imm32)

void VCpu::_MUL_RM8()
{
    BYTE value = readModRM8(rmbyte);
    regs.W.AX = doMul(regs.B.AL, value);

    if (regs.B.AH == 0x00) {
        setCF(0);
        setOF(0);
    } else {
        setCF(1);
        setOF(1);
    }
}

void VCpu::_MUL_RM16()
{
    WORD value = readModRM16(rmbyte);
    DWORD result = doMul(regs.W.AX, value);
    regs.W.AX = result & 0xFFFF;
    regs.W.DX = (result >> 16) & 0xFFFF;

    if (regs.W.DX == 0x0000) {
        setCF(0);
        setOF(0);
    } else {
        setCF(1);
        setOF(1);
    }
}

void VCpu::_MUL_RM32()
{
    WORD value = readModRM32(rmbyte);
    QWORD result = doMul(regs.W.AX, value);
    setEAX(result & 0xFFFFFFFF);
    setEDX((result >> 32) & 0xFFFFFFFF);

    if (getEDX() == 0x00000000) {
        setCF(0);
        setOF(0);
    } else {
        setCF(1);
        setOF(1);
    }
}

void VCpu::_IMUL_RM8()
{
    SIGNED_BYTE value = readModRM8(rmbyte);
    regs.W.AX = doImul(static_cast<SIGNED_BYTE>(getAL()), value);

    if (regs.B.AH == 0x00 || regs.B.AH == 0xFF) {
        setCF(0);
        setOF(0);
    } else {
        setCF(1);
        setOF(1);
    }
}

void VCpu::_IMUL_reg32_RM32_imm8()
{
    vlog(LogCPU, "Not implemented: IMUL reg32,rm32,imm8");
    vomit_exit(1);
}

void VCpu::_IMUL_reg32_RM32_imm32()
{
    vlog(LogCPU, "Not implemented: IMUL reg32,rm32,imm32");
    vomit_exit(1);
}

void VCpu::_IMUL_reg16_RM16_imm16()
{
    BYTE rm = fetchOpcodeByte();
    SIGNED_WORD imm = fetchOpcodeWord();
    SIGNED_WORD value = readModRM16(rm);
    SIGNED_WORD result = doImul(value, static_cast<SIGNED_WORD>(imm));
    SIGNED_DWORD largeResult = doImul(value, static_cast<SIGNED_WORD>(imm));

    setRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm)), result);

    setCF(result != largeResult);
    setOF(result != largeResult);
}

void VCpu::_IMUL_reg16_RM16()
{
    BYTE rm = fetchOpcodeByte();
    SIGNED_WORD src = readModRM16(rm);
    SIGNED_WORD dest = getRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm)));
    SIGNED_WORD result = doImul(dest, src);
    SIGNED_DWORD largeResult = doImul(dest, src);

    setRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm)), result);

    setCF(result != largeResult);
    setOF(result != largeResult);
}

void VCpu::_IMUL_reg32_RM32()
{
    BYTE rm = fetchOpcodeByte();
    SIGNED_DWORD src = readModRM32(rm);
    SIGNED_DWORD dest = getRegister32(static_cast<VCpu::RegisterIndex32>(vomit_modRMRegisterPart(rm)));

    SIGNED_DWORD result = doImul(dest, src);
    SIGNED_QWORD largeResult = doImul(dest, src);

    setRegister32(static_cast<VCpu::RegisterIndex32>(vomit_modRMRegisterPart(rm)), result);

    setCF(result != largeResult);
    setOF(result != largeResult);
}

void VCpu::_IMUL_reg16_RM16_imm8()
{
    BYTE rm = fetchOpcodeByte();
    SIGNED_BYTE imm = fetchOpcodeByte();
    SIGNED_WORD value = readModRM16(rm);
    SIGNED_WORD result = doImul(value, static_cast<SIGNED_WORD>(imm));
    SIGNED_DWORD largeResult = doImul(value, static_cast<SIGNED_WORD>(imm));

    setRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm)), result);

    setCF(result != largeResult);
    setOF(result != largeResult);
}

void VCpu::_IMUL_RM16()
{
    SIGNED_WORD value = readModRM16(rmbyte);
    SIGNED_DWORD result = doImul(static_cast<SIGNED_WORD>(getAX()), value);
    regs.W.AX = result;
    regs.W.DX = result >> 16;

    if (regs.W.DX == 0x0000 || regs.W.DX == 0xFFFF) {
        setCF(0);
        setOF(0);
    } else {
        setCF(1);
        setOF(1);
    }
}

void VCpu::_DIV_RM8()
{
    BYTE value = readModRM8(rmbyte);
    WORD tAX = regs.W.AX;

    if (value == 0) {
        exception(0);
        return;
    }

    regs.B.AL = (BYTE)(tAX / value); // Quote
    regs.B.AH = (BYTE)(tAX % value); // Remainder
}

void VCpu::_DIV_RM16()
{
    WORD value = readModRM16(rmbyte);
    DWORD tDXAX = regs.W.AX + (regs.W.DX << 16);

    if (value == 0) {
        exception(0);
        return;
    }

    regs.W.AX = (WORD)(tDXAX / value); // Quote
    regs.W.DX = (WORD)(tDXAX % value); // Remainder
}

void VCpu::_DIV_RM32()
{
    DWORD value = readModRM32(rmbyte);
    QWORD tEDXEAX = getEAX() | (getEDX() << 16);

    if (value == 0) {
        exception(0);
        return;
    }

    setEAX(tEDXEAX / value); // Quote
    setEDX(tEDXEAX % value); // Remainder
}

void VCpu::_IDIV_RM8()
{
    SIGNED_BYTE value = (SIGNED_BYTE)readModRM8(rmbyte);
    SIGNED_WORD tAX = (SIGNED_WORD)regs.W.AX;

    if (value == 0) {
        exception(0);
        return;
    }

    regs.B.AL = (SIGNED_BYTE)(tAX / value); // Quote
    regs.B.AH = (SIGNED_BYTE)(tAX % value); // Remainder
}

void VCpu::_IDIV_RM16()
{
    SIGNED_WORD value = readModRM16(rmbyte);
    SIGNED_DWORD tDXAX = (regs.W.AX + (regs.W.DX << 16));

    if (value == 0) {
        exception(0);
        return;
    }

    regs.W.AX = (SIGNED_WORD)(tDXAX / value); // Quote
    regs.W.DX = (SIGNED_WORD)(tDXAX % value); // Remainder
}
