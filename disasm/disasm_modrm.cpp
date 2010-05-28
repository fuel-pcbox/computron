#include "insn-types.h"
#include "disasm.h"
#include <stdio.h>
#include <string.h>

/* Table of *printf() format strings for the 24 non-register ModR/M address
 * specifications.
 */

const char *modrm_fmt[24] = {
    "[BX+SI]",
    "[BX+DI]",
    "[BP+SI]",
    "[BP+DI]",
    "[SI]",
    "[DI]",
    "[%04X]",
    "[BX]",

    "[BX+SI+%02X]",
    "[BX+DI+%02X]",
    "[BP+SI+%02X]",
    "[BP+DI+%02X]",
    "[SI+%02X]",
    "[DI+%02X]",
    "[BP+%02X]",
    "[BX+%02X]",

    "[BX+SI+%04X]",
    "[BX+DI+%04X]",
    "[BP+SI+%04X]",
    "[BP+DI+%04X]",
    "[SI+%04X]",
    "[DI+%04X]",
    "[BP+%04X]",
    "[BX+%04X]"
};

/* int modrm_width (BYTE rm)
 *
 * Returns the width in bytes of a ModR/M address specifier, based on
 * the first byte in the ModR/M sequence.
 *
 */

int modrm_width(BYTE rm)
{
    switch (rm & 0xC0) {
    case 0x00:
        switch (rm & 7) {
        case 6: return 3;
        default: return 1;
        }
    case 0x40:
        return 2;
    case 0x80:
        return 3;
    default:
        return 1;
    }
}

/* bool is_modrm_insn (const insn_t* i)
 *
 * Returns true if the instruction type specified handles
 * ModR/M data (and therefore needs additional width calculation)
 *
 */

bool is_modrm_insn(const insn_t* i)
{
    switch (i->type) {
    case OP_RM8:
    case OP_RM16:
    case OP_RM8_reg8:
    case OP_RM16_reg16:
    case OP_reg8_RM8:
    case OP_reg16_RM16:
    case OP_RM16_seg:
    case OP_RM8_imm8:
    case OP_RM16_imm16:
    case OP_seg_RM16:
        return true;
    }
    return false;
}

/* char* modrm_string (BYTE *code, int bits)
 *
 * Generates a string representation of a ModR/M address specifyer
 * and returns a pointer to the result.
 *
 */

char* modrm_string(BYTE *p, int bits)
{
    BYTE rm;
    static char ret[16];
    rm = *p;
    switch (rm & 0xC0) {
    case 0x00:
        if ((rm & 7) == 6)
            sprintf(ret, modrm_fmt[rm & 7], (*(p+1) | (*(p+2) << 8)));
        else
            sprintf(ret, "%s", modrm_fmt[rm & 7]);
        break;
    case 0x40:
        sprintf(ret, modrm_fmt[8 + (rm & 7)], *(p+1));
        break;
    case 0x80:
        sprintf(ret, modrm_fmt[16 + (rm & 7)], (*(p+1) | (*(p+2) << 8)));
        break;
    case 0xC0:
        if (bits == 8) {
            switch (rm & 7) {
            case 0x00: strcpy(ret, "AL"); break;
            case 0x01: strcpy(ret, "CL"); break;
            case 0x02: strcpy(ret, "DL"); break;
            case 0x03: strcpy(ret, "BL"); break;
            case 0x04: strcpy(ret, "AH"); break;
            case 0x05: strcpy(ret, "CH"); break;
            case 0x06: strcpy(ret, "DH"); break;
            case 0x07: strcpy(ret, "BH"); break;
            }
        } else {
            switch (rm & 7) {
            case 0x00: strcpy(ret, "AX"); break;
            case 0x01: strcpy(ret, "CX"); break;
            case 0x02: strcpy(ret, "DX"); break;
            case 0x03: strcpy(ret, "BX"); break;
            case 0x04: strcpy(ret, "SP"); break;
            case 0x05: strcpy(ret, "BP"); break;
            case 0x06: strcpy(ret, "SI"); break;
            case 0x07: strcpy(ret, "DI"); break;
            }
        }
        break;
    }
    return ret;
}
