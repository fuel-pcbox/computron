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

#pragma once

#define DEFAULT_RM8_reg8(helper, name) \
    void CPU::name(Instruction& insn) { \
        auto& modrm = insn.modrm(); \
        modrm.write8(helper(modrm.read8(), insn.reg8())); \
	}

#define DEFAULT_RM16_reg16(helper, name) \
    void CPU::name(Instruction& insn) { \
        auto& modrm = insn.modrm(); \
        modrm.write16(helper(modrm.read16(), insn.reg16())); \
	}

#define DEFAULT_reg8_RM8(helper, name) \
    void CPU::name(Instruction& insn) { \
        insn.reg8() = helper(insn.reg8(), insn.modrm().read8()); \
	}

#define DEFAULT_reg16_RM16(helper, name) \
    void CPU::name(Instruction& insn) { \
        insn.reg16() = helper(insn.reg16(), insn.modrm().read16()); \
	}

#define DEFAULT_reg32_RM32(helper, name) \
    void CPU::name(Instruction& insn) { \
        insn.reg32() = helper(insn.reg32(), insn.modrm().read32()); \
    }

#define DEFAULT_RM8_imm8(helper, name) \
    void CPU::name(Instruction& insn) { \
        auto& modrm = insn.modrm(); \
        modrm.write8(helper(modrm.read8(), insn.imm8())); \
	}

#define DEFAULT_RM16_imm16(helper, name) \
    void CPU::name(Instruction& insn) { \
        auto& modrm = insn.modrm(); \
        modrm.write16(helper(modrm.read16(), insn.imm16())); \
	}

#define DEFAULT_RM32_imm32(helper, name) \
    void CPU::name(Instruction& insn) { \
        auto& modrm = insn.modrm(); \
        modrm.write32(helper(modrm.read32(), insn.imm32())); \
	}

#define DEFAULT_RM16_imm8(helper, name) \
    void CPU::name(Instruction& insn) { \
        auto& modrm = insn.modrm(); \
        modrm.write16(helper(modrm.read16(), signExtend<WORD>(insn.imm8()))); \
	}

#define DEFAULT_RM32_imm8(helper, name) \
    void CPU::name(Instruction& insn) { \
        auto& modrm = insn.modrm(); \
        modrm.write32(helper(modrm.read32(), signExtend<DWORD>(insn.imm8()))); \
    }

#define DEFAULT_AL_imm8(helper, name) \
    void CPU::name(Instruction& insn) { \
        regs.B.AL = helper(getAL(), insn.imm8()); \
	}

#define DEFAULT_AX_imm16(helper, name) \
    void CPU::name(Instruction& insn) { \
        regs.W.AX = helper(getAX(), insn.imm16()); \
	}

#define DEFAULT_EAX_imm32(helper, name) \
    void CPU::name(Instruction& insn) { \
        regs.D.EAX = helper(getEAX(), insn.imm32()); \
    }

#define READONLY_RM8_reg8(helper, name) \
    void CPU::name(Instruction& insn) { \
        helper(insn.modrm().read8(), insn.reg8()); \
	}

#define READONLY_RM16_reg16(helper, name) \
    void CPU::name(Instruction& insn) { \
        helper(insn.modrm().read16(), insn.reg16()); \
	}

#define READONLY_RM32_reg32(helper, name) \
    void CPU::name(Instruction& insn) { \
        helper(insn.modrm().read32(), insn.reg32()); \
    }

#define READONLY_reg8_RM8(helper, name) \
    void CPU::name(Instruction& insn) { \
        helper(insn.reg8(), insn.modrm().read8()); \
	}

#define READONLY_reg16_RM16(helper, name) \
    void CPU::name(Instruction& insn) { \
        helper(insn.reg16(), insn.modrm().read16()); \
    }

#define READONLY_reg32_RM32(helper, name) \
    void CPU::name(Instruction& insn) { \
        helper(insn.reg32(), insn.modrm().read32()); \
    }

#define READONLY_RM8_imm8(helper, name) \
    void CPU::name(Instruction& insn) { \
        helper(insn.modrm().read8(), insn.imm8()); \
	}

#define READONLY_RM16_imm16( helper, name ) \
    void CPU::name(Instruction& insn) { \
        helper(insn.modrm().read16(), insn.imm16()); \
    }

#define READONLY_RM32_imm8( helper, name ) \
    void CPU::name(Instruction& insn) { \
        helper(insn.modrm().read32(), signExtend<DWORD>(insn.imm8())); \
    }

#define READONLY_RM32_imm32( helper, name ) \
    void CPU::name(Instruction& insn) { \
        helper(insn.modrm().read32(), insn.imm32()); \
	}

#define READONLY_RM16_imm8(helper, name) \
    void CPU::name(Instruction& insn) { \
        helper(insn.modrm().read16(), signExtend<WORD>(insn.imm8())); \
	}

#define READONLY_AL_imm8(helper, name) \
    void CPU::name(Instruction& insn) { \
        helper(getAL(), insn.imm8()); \
	}

#define READONLY_AX_imm16(helper, name) \
    void CPU::name(Instruction& insn) { \
        helper(getAX(), insn.imm16()); \
	}

#define READONLY_EAX_imm32(helper, name) \
    void CPU::name(Instruction& insn) { \
        helper(getEAX(), insn.imm32()); \
    }

#define DEFAULT_RM32_reg32(helper, name) \
    void CPU::name(Instruction& insn) { \
        auto& modrm = insn.modrm(); \
        modrm.write32(helper(modrm.read32(), insn.reg32())); \
    }

#define DEFAULT_RM8_1(helper, name) \
    void CPU::name(Instruction& insn) { \
        auto& modrm = insn.modrm(); \
        modrm.write8(helper(modrm.read8(), 1)); \
    }

#define DEFAULT_RM16_1(helper, name) \
    void CPU::name(Instruction& insn) { \
        auto& modrm = insn.modrm(); \
        modrm.write16(helper(modrm.read16(), 1)); \
    }

#define DEFAULT_RM32_1(helper, name) \
    void CPU::name(Instruction& insn) { \
        auto& modrm = insn.modrm(); \
        modrm.write32(helper(modrm.read32(), 1)); \
    }

#define DEFAULT_RM8_CL(helper, name) \
    void CPU::name(Instruction& insn) { \
        auto& modrm = insn.modrm(); \
        modrm.write8(helper(modrm.read8(), getCL())); \
    }

#define DEFAULT_RM16_CL(helper, name) \
    void CPU::name(Instruction& insn) { \
        auto& modrm = insn.modrm(); \
        modrm.write16(helper(modrm.read16(), getCL())); \
    }

#define DEFAULT_RM32_CL(helper, name) \
    void CPU::name(Instruction& insn) { \
        auto& modrm = insn.modrm(); \
        modrm.write32(helper(modrm.read32(), getCL())); \
    }
