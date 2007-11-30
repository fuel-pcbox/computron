#include "insn-types.h"
#include "disasm.h"
#include <stdio.h>
#include <assert.h>

/* int insn_width ( BYTE *data )
 *
 * Calculates the width in bytes of the instruction starting
 * at data[0]
 *
 */

int insn_width(BYTE *p) {

	int ret, wrapped = 0;
	const insn_t *i;

	assert(p != NULL);

	ret = 0;
	i = &insn_table[p[0]];

	if (i->type == WRAP) {
		wrapped = 1;
		i = &wrapped_insn_table[p[0]][MODRM_REG(p[1])];
	}

	if (i->type == OP_OP) {
		i = &insn_table[p[1]];
		ret++;
	}

	ret += insn_base_width[i->type];
	if (is_modrm_insn(i)) {
		ret += modrm_width(p[1]);
	}

	return ret;
}

/* bool disassemble ( BYTE *data, char *buffer, int buffer_size )
 *
 * Disassembles the instruction starting at data[0] and writes
 * a string representation into a buffer.
 *
 * Returns false if the instruction couldn't be deciphered.
 *
 */

bool disassemble(BYTE *p, long unsigned int offset, char *buf, int len) {

	const insn_t *i;
	char *ptr;
	WORD w;
	int base_width = 0;

	assert(p != NULL);
	assert(buf != NULL);
	assert(len != 0);

	ptr = buf;

	i = &insn_table[p[0]];

	if (i->type == WRAP) {
		i = &wrapped_insn_table[p[0]][MODRM_REG(p[1])];
		base_width++;
	}
	if (i->type == OP_OP) {
		ptr += snprintf(ptr, len, "%s ", i->name);
		len -= (ptr - buf);
		i = &insn_table[p[1]];
		base_width++;
	}
	base_width += insn_base_width[i->type];

	ptr += snprintf(ptr, len, "%s ", i->name);
	len -= (ptr - buf);

	assert(len > 1);

	switch (i->type) {
	case OP: break;
	case OP_CS: snprintf(ptr, len, "CS"); break;
	case OP_DS: snprintf(ptr, len, "DS"); break;
	case OP_ES: snprintf(ptr, len, "ES"); break;
	case OP_SS: snprintf(ptr, len, "SS"); break;
	case OP_imm8: snprintf(ptr, len, "%02X", p[1]); break;

	/* Single IP-relative immediate byte operand */
	case OP_short_imm8:
		if (p[1] > 0x7f)
			snprintf(ptr, len, "%04lX",
				offset - (0x100 - p[1] - base_width));
		else
			snprintf(ptr, len, "%04lX",
				offset + p[1] + base_width);
		break;

	/* Single IP-relative immediate word operand */
	case OP_relimm16:
		w = MAKEWORD(p[1], p[2]);
		if (w > 0x7fff)
			snprintf(ptr, len, "%04lX",
				offset - (0x10000 - w - base_width));
		else
			snprintf(ptr, len, "%04lX",
				offset + w + base_width);
		break;

	case OP_DX_AL:
		snprintf(ptr, len, "DX, AL");
		break;
	case OP_AL_DX:
		snprintf(ptr, len, "AL, DX");
		break;
	case OP_AL_moff8:
		snprintf(ptr, len, "AL, [%04X]", MAKEWORD(p[1], p[2]));
		break;
	case OP_moff8_AL:
		snprintf(ptr, len, "[%04X], AL", MAKEWORD(p[1], p[2]));
		break;
	case OP_moff16_AX:
		snprintf(ptr, len, "[%04X], AX", MAKEWORD(p[1], p[2]));
		break;
	case OP_AX_moff16:
		snprintf(ptr, len, "AX, [%04X]", MAKEWORD(p[1], p[2]));
		break;
	case OP_RM8:
		snprintf(ptr, len, "%s", modrm_string(&p[1], 8));
		break;
	case OP_RM16:
		snprintf(ptr, len, "%s", modrm_string(&p[1], 16));
		break;
	case OP_RM8_imm8:
		snprintf(ptr, len, "%s, %02X", modrm_string(&p[1], 8), p[modrm_width(p[1]) + 1]);
		break;
	case OP_RM16_imm8:
		{
			int w = modrm_width( p[1] );
			snprintf(ptr, len, "%s, %02X", modrm_string(&p[1], 16), p[w+1]);
		}
		break;
	case OP_RM16_imm16:
		{
			int w = modrm_width( p[1] );
			snprintf(ptr, len, "%s, %04X", modrm_string(&p[1], 16), MAKEWORD(p[w+2], p[w+3]));
		}
		break;
	case OP_imm16:
		snprintf(ptr, len, "%04X", MAKEWORD(p[1], p[2]));
		break;
	case OP_imm16_imm16:
		snprintf(ptr, len, "%04X:%04X", MAKEWORD(p[3], p[4]), MAKEWORD(p[1], p[2]));
		break;
	case OP_reg16:
		snprintf(ptr, len, "%s", R16(*p));
		break;
	case OP_AL_imm8:
		snprintf(ptr, len, "AL, %02X", p[1]);
		break;
	case OP_AX_imm16:
		snprintf(ptr, len, "AX, %04X", MAKEWORD(p[1], p[2]));
		break;
	case OP_imm8_AL:
		snprintf(ptr, len, "%02X, AL", p[1]);
		break;
	case OP_imm8_AX:
		snprintf(ptr, len, "%02X, AX", p[1]);
		break;
	case OP_AX_reg16:
		snprintf(ptr, len, "AX, %s", R16(*p));
		break;
	case OP_reg8_imm8:
		snprintf(ptr, len, "%s, %02X", R8(*p), p[1]);
		break;
	case OP_reg16_imm16:
		snprintf(ptr, len, "%s, %04X", R16(*p), MAKEWORD(p[1], p[2]));
		break;
	case OP_RM8_reg8:
		snprintf(ptr, len, "%s, %s", modrm_string(p+1, 8), R8( MODRM_REG(*(p+1)) ));
		break;
	case OP_seg_RM16:
		++p;
		snprintf(ptr, len, "%s, %s", SEG(MODRM_REG(*p)), modrm_string(p, 16));
		break;
	case OP_reg8_RM8:
		snprintf(ptr, len, "%s, %s", R8( MODRM_REG(*(p+1)) ), modrm_string(p+1, 8));
		break;
	case OP_reg16_RM16:
		snprintf(ptr, len, "%s, %s", R16( MODRM_REG(*(p+1)) ), modrm_string(p+1, 16));
		break;
	case OP_RM16_reg16:
		snprintf(ptr, len, "%s, %s", modrm_string(p+1, 16), R16( MODRM_REG(*(p+1)) ));
		break;
	case OP_RM16_seg:
		++p;
		snprintf(ptr, len, "%s, %s", modrm_string(p, 16), SEG(MODRM_REG(*p)));
		break;
	default:
		snprintf(ptr, len, "???");
		return false;
	}
	return true;
}

