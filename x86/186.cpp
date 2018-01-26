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

#include "vomit.h"
#include "vcpu.h"
#include "debug.h"

void VCpu::_wrap_0x0F()
{
    BYTE op = fetchOpcodeByte();
    switch (op) {
    case 0x01:
    {
        this->rmbyte = fetchOpcodeByte();
        switch (vomit_modRMRegisterPart(this->rmbyte)) {
        case 0: _SGDT(); return;
        }
        (void) readModRM16(this->rmbyte);
        vlog(LogAlert, "Sliding by 0F 01 /%d\n", vomit_modRMRegisterPart(this->rmbyte));
        break;
    }
    case 0xFF: // UD0
    case 0xB9: // UD1
    case 0x0B: // UD2
        vlog(LogAlert, "Undefined opcode 0F %02X", op);
        exception(6);
        break;
    default:
        vlog(LogCPU, "_wrap_0x0F passing opcode to VCpu::decodeNext(): 0F %02X", op);
        setEIP(getEIP() - 2);
        decodeNext();
        break;
    }
}

void VCpu::_BOUND()
{
    BYTE rm = fetchOpcodeByte();
    WORD* ptr = static_cast<WORD*>(resolveModRM8(rm));
    WORD index = getRegister16(static_cast<VCpu::RegisterIndex16>(vomit_modRMRegisterPart(rm)));

    if (index < ptr[0] || index > ptr[1]) {
        /* Raise BR exception */
        exception(5);
    }
}

void VCpu::_PUSH_imm8()
{
    if (o32())
        push32(vomit_signExtend<DWORD>(fetchOpcodeByte()));
    else
        push(vomit_signExtend<WORD>(fetchOpcodeByte()));
}

void VCpu::_PUSH_imm16()
{
    push(fetchOpcodeWord());
}

void VCpu::_ENTER()
{
    WORD Size = fetchOpcodeWord();
    BYTE NestingLevel = fetchOpcodeByte() % 32;
    push(getBP());
    WORD FrameTemp = getSP();
    if (NestingLevel != 0) {
        for (WORD i = 1; i <= (NestingLevel - 1); ++i) {
            setBP(getBP() - 2);
            push(readMemory16(getSS(), getBP()));
        }
    }
    push(FrameTemp);
    setBP(FrameTemp);
    setSP(getBP() - Size);
}

void VCpu::_LEAVE()
{
    setSP(getBP());
    setBP(pop());
}

void VCpu::_PUSHA()
{
    WORD oldSP = getSP();
    push(getAX());
    push(getCX());
    push(getDX());
    push(getBX());
    push(oldSP);
    push(getBP());
    push(getSI());
    push(getDI());
}

void VCpu::_PUSHAD()
{
    DWORD oldESP = getESP();
    push32(getEAX());
    push32(getECX());
    push32(getEDX());
    push32(getEBX());
    push32(oldESP);
    push32(getEBP());
    push32(getESI());
    push32(getEDI());
}

void VCpu::_POPA()
{
    setDI(pop());
    setSI(pop());
    setBP(pop());
    (void) pop();
    setBX(pop());
    setDX(pop());
    setCX(pop());
    setAX(pop());
}

void VCpu::_POPAD()
{
    setEDI(pop32());
    setESI(pop32());
    setEBP(pop32());
    (void) pop32();
    setEBX(pop32());
    setEDX(pop32());
    setECX(pop32());
    setEAX(pop32());
}
