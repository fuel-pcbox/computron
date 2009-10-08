/* 8086/bitwise.cpp
 * Bitwise instructions
 *
 */

#define MASK_STEPS_IF_80286 if( cpu->type >= INTEL_80186 ) { steps &= 0x1F; }

#include "vomit.h"
#include "templates.h"

DEFAULT_RM8_reg8( cpu_and, _AND_RM8_reg8 )
DEFAULT_RM16_reg16( cpu_and, _AND_RM16_reg16 )
DEFAULT_reg8_RM8( cpu_and, _AND_reg8_RM8 )
DEFAULT_reg16_RM16( cpu_and, _AND_reg16_RM16 )
DEFAULT_RM8_imm8( cpu_and, _AND_RM8_imm8 )
DEFAULT_RM16_imm16( cpu_and, _AND_RM16_imm16 )
DEFAULT_RM16_imm8( cpu_and, _AND_RM16_imm8 )
DEFAULT_AL_imm8( cpu_and, _AND_AL_imm8 )
DEFAULT_AX_imm16( cpu_and, _AND_AX_imm16 )

DEFAULT_RM8_reg8( cpu_xor, _XOR_RM8_reg8 )
DEFAULT_RM16_reg16( cpu_xor, _XOR_RM16_reg16 )
DEFAULT_reg8_RM8( cpu_xor, _XOR_reg8_RM8 )
DEFAULT_reg16_RM16( cpu_xor, _XOR_reg16_RM16 )
DEFAULT_RM8_imm8( cpu_xor, _XOR_RM8_imm8 )
DEFAULT_RM16_imm16( cpu_xor, _XOR_RM16_imm16 )
DEFAULT_RM16_imm8( cpu_xor, _XOR_RM16_imm8 )
DEFAULT_AL_imm8( cpu_xor, _XOR_AL_imm8 )
DEFAULT_AX_imm16( cpu_xor, _XOR_AX_imm16 )

DEFAULT_RM8_reg8( cpu_or, _OR_RM8_reg8 )
DEFAULT_RM16_reg16( cpu_or, _OR_RM16_reg16 )
DEFAULT_reg8_RM8( cpu_or, _OR_reg8_RM8 )
DEFAULT_reg16_RM16( cpu_or, _OR_reg16_RM16 )
DEFAULT_RM8_imm8( cpu_or, _OR_RM8_imm8 )
DEFAULT_RM16_imm16( cpu_or, _OR_RM16_imm16 )
DEFAULT_RM16_imm8( cpu_or, _OR_RM16_imm8 )
DEFAULT_AL_imm8( cpu_or, _OR_AL_imm8 )
DEFAULT_AX_imm16( cpu_or, _OR_AX_imm16 )

READONLY_RM8_reg8( cpu_and, _TEST_RM8_reg8 )
READONLY_RM16_reg16( cpu_and, _TEST_RM16_reg16 )
READONLY_reg8_RM8( cpu_and, _TEST_reg8_RM8 )
READONLY_reg16_RM16( cpu_and, _TEST_reg16_RM16 )
READONLY_RM8_imm8( cpu_and, _TEST_RM8_imm8 )
READONLY_RM16_imm16( cpu_and, _TEST_RM16_imm16 )
READONLY_RM16_imm8( cpu_and, _TEST_RM16_imm8 )
READONLY_AL_imm8( cpu_and, _TEST_AL_imm8 )
READONLY_AX_imm16( cpu_and, _TEST_AX_imm16 )

void _CBW(vomit_cpu_t *cpu)
{
    cpu->regs.W.AX = signext(cpu->regs.B.AL);
}

void _CWD(vomit_cpu_t *cpu)
{
    if (cpu->regs.B.AH & 0x80)
        cpu->regs.W.DX = 0xFFFF;
    else
        cpu->regs.W.DX = 0x0000;
}

void _SALC(vomit_cpu_t *cpu)
{
    cpu->regs.B.AL = cpu->CF ? 0xFF : 0x00;
}

BYTE cpu_or8(vomit_cpu_t *cpu, BYTE dest, BYTE src)
{
    BYTE result = dest | src;
    vomit_cpu_update_flags8(cpu, result);
    cpu->OF = 0;
    cpu->CF = 0;
    return result;
}

WORD cpu_or16(vomit_cpu_t *cpu, WORD dest, WORD src)
{
    WORD result = dest | src;
    vomit_cpu_update_flags16(cpu, result);
    cpu->OF = 0;
    cpu->CF = 0;
    return result;
}

BYTE cpu_xor8(vomit_cpu_t *cpu, BYTE dest, BYTE src)
{
    BYTE result = dest ^ src;
    vomit_cpu_update_flags8(cpu, result);
    cpu->OF = 0;
    cpu->CF = 0;
    return result;
}

WORD cpu_xor16(vomit_cpu_t *cpu, WORD dest, WORD src)
{
    WORD result = dest ^ src;
    vomit_cpu_update_flags16(cpu, result);
    cpu->OF = 0;
    cpu->CF = 0;
    return result;
}

BYTE cpu_and8(vomit_cpu_t *cpu, BYTE dest, BYTE src)
{
    BYTE result = dest & src;
    vomit_cpu_update_flags8(cpu, result);
    cpu->OF = 0;
    cpu->CF = 0;
    return result;
}

WORD cpu_and16(vomit_cpu_t *cpu, WORD dest, WORD src)
{
    WORD result = dest & src;
    vomit_cpu_update_flags16(cpu, result);
    cpu->OF = 0;
    cpu->CF = 0;
    return result;
}

DWORD cpu_shl(vomit_cpu_t *cpu, WORD data, BYTE steps, BYTE bits)
{
    DWORD result = (DWORD)data;

    MASK_STEPS_IF_80286;

    if (bits == 8) {
        for (BYTE i = 0; i < steps; ++i) {
            cpu->CF = (result >> 7) & 1;
            result <<= 1;
        }
    } else {
        for (BYTE i = 0; i < steps; ++i) {
            cpu->CF = (result >> 15) & 1;
            result <<= 1;
        }
    }

    if (steps == 1) {
        cpu->OF = (data >> (bits - 1)) ^ cpu->CF;
    }

    vomit_cpu_update_flags(cpu, result, bits);
    return result;
}

DWORD cpu_shr(vomit_cpu_t *cpu, WORD data, BYTE steps, BYTE bits)
{
    DWORD result = (DWORD)data;

    MASK_STEPS_IF_80286;

    if (bits == 8) {
        for (BYTE i = 0; i < steps; ++i) {
            cpu->CF = result & 1;
            result >>= 1;
        }
    } else {
        for (BYTE i = 0; i < steps; ++i) {
            cpu->CF = result & 1;
            result >>= 1;
        }
    }

    if (steps == 1) {
        cpu->OF = (data >> (bits - 1)) & 1;
    }

    vomit_cpu_update_flags(cpu, result, bits);
    return result;
}

DWORD cpu_sar(vomit_cpu_t *cpu, WORD data, BYTE steps, BYTE bits)
{
    DWORD result = (DWORD)data;
    word n;

    MASK_STEPS_IF_80286;

    if( bits == 8 ) {
        for (BYTE i = 0; i < steps; ++i) {
            n = result;
            result = (result>>1) | (n&0x80);
            cpu->CF = n & 1;
        }
    } else {
        for (BYTE i = 0; i < steps; ++i) {
            n = result;
            result = (result>>1) | (n&0x8000);
            cpu->CF = n & 1;
        }
    }

    if (steps == 1) {
        cpu->OF = 0;
    }

    vomit_cpu_update_flags(cpu, result, bits);
    return result;
}

DWORD cpu_rol(vomit_cpu_t *cpu, WORD data, BYTE steps, BYTE bits)
{
    DWORD result = (DWORD)data;

    MASK_STEPS_IF_80286;

    if (bits == 8) {
        for (BYTE i = 0; i < steps; ++i) {
            cpu->CF = (result>>7) & 1;
            result = (result<<1) | cpu->CF;
        }
    } else {
        for (BYTE i = 0; i < steps; ++i) {
            cpu->CF = (result>>15) & 1;
            result = (result<<1) | cpu->CF;
        }
    }

    if (steps == 1) {
        cpu->OF = ((result >> (bits - 1)) & 1) ^ cpu->CF;
    }

    return result;
}

DWORD cpu_ror(vomit_cpu_t *cpu, WORD data, BYTE steps, BYTE bits)
{
    DWORD result = (DWORD)data;

    MASK_STEPS_IF_80286;

    if (bits == 8) {
        for (BYTE i = 0; i < steps; ++i) {
            cpu->CF = result & 1;
            result = (result>>1) | (cpu->CF<<7);
        }
    } else {
        for (BYTE i = 0; i < steps; ++i) {
            cpu->CF = result & 1;
            result = (result>>1) | (cpu->CF<<15);
        }
    }

    if (steps == 1) {
        cpu->OF = (result >> (bits - 1)) ^ ((result >> (bits - 2) & 1));
    }

    return result;
}

DWORD cpu_rcl(vomit_cpu_t *cpu, WORD data, BYTE steps, BYTE bits)
{
    DWORD result = (DWORD)data;
    WORD n;

    MASK_STEPS_IF_80286;

    if (bits == 8) {
        for (BYTE i = 0; i < steps; ++i) {
            n = result;
            result = ((result<<1) & 0xFF) | cpu->CF;
            cpu->CF = (n>>7) & 1;
        }
    } else {
        for (BYTE i = 0; i < steps; ++i) {
            n = result;
            result = ((result<<1) & 0xFFFF) | cpu->CF;
            cpu->CF = (n>>15) & 1;
        }
    }

    if (steps == 1) {
        cpu->OF = (result >> (bits - 1)) ^ cpu->CF;
    }

    return result;
}

DWORD cpu_rcr(vomit_cpu_t *cpu, WORD data, BYTE steps, BYTE bits)
{
    DWORD result = (DWORD)data;
    word n;

    MASK_STEPS_IF_80286;

    if (bits == 8) {
        for (BYTE i = 0; i < steps; ++i) {
            n = result;
            result = (result>>1) | (cpu->CF<<7);
            cpu->CF = n & 1;
        }
    } else {
        for (BYTE i = 0; i < steps; ++i) {
            n = result;
            result = (result>>1) | (cpu->CF<<15);
            cpu->CF = n & 1;
        }
    }

    if (steps == 1) {
        cpu->OF = (result >> (bits - 1)) ^ ((result >> (bits - 2) & 1));
    }

    return result;
}

void _NOT_RM8(vomit_cpu_t *cpu)
{
    BYTE value = vomit_cpu_modrm_read8(cpu, cpu->rmbyte);
    vomit_cpu_modrm_update8(cpu, ~value);
}

void _NOT_RM16(vomit_cpu_t *cpu)
{
    word value = vomit_cpu_modrm_read16(cpu, cpu->rmbyte);
    vomit_cpu_modrm_update16(cpu, ~value);
}

void _NEG_RM8(vomit_cpu_t *cpu)
{
    BYTE value = vomit_cpu_modrm_read8(cpu, cpu->rmbyte);
    BYTE old = value;
    value = -value;
    vomit_cpu_modrm_update8(cpu, value);
    cpu->CF = old != 0;
    vomit_cpu_update_flags8(cpu, value);
    cpu->OF = ((
        ((0)^(old)) &
        ((0)^(value))
        )>>(7))&1;
    vomit_cpu_setAF(cpu, value, 0, old);
}

void _NEG_RM16(vomit_cpu_t *cpu)
{
    WORD value = vomit_cpu_modrm_read16(cpu, cpu->rmbyte);
    WORD old = value;
    value = -value;
    vomit_cpu_modrm_update16(cpu, value);
    cpu->CF = ( old != 0 );
    vomit_cpu_update_flags16(cpu, value);
    cpu->OF = ((
        ((0)^(old)) &
        ((0)^(value))
        )>>(15))&1;
    vomit_cpu_setAF(cpu, value, 0, old);
}
