#include <insn-types.h>
#include <disasm.h>

const insn_t insn_table[256] = {

	{ "ADD",		OP_RM8_reg8		},			/* 00 */
	{ "ADD",		OP_RM16_reg16	},			/* 01 */
	{ "ADD",		OP_reg8_RM8		},			/* 02 */
	{ "ADD",		OP_reg16_RM16	},			/* 03 */
	{ "ADD",		OP_AL_imm8		},			/* 04 */
	{ "ADD",		OP_AX_imm16		},			/* 05 */
	{ "PUSH",		OP_ES			},			/* 06 */
	{ "POP",		OP_ES			},			/* 07 */
	{ "OR",			OP_RM8_reg8		},			/* 08 */
	{ "OR",			OP_RM16_reg16	},			/* 09 */
	{ "OR",			OP_reg8_RM8		},			/* 0A */
	{ "OR",			OP_reg16_RM16	},			/* 0B */
	{ "OR",			OP_AL_imm8		},			/* 0C */
	{ "OR",			OP_AX_imm16		},			/* 0D */
	{ "PUSH",		OP_CS			},			/* 0E */
	{ "POP",		OP_CS			},			/* 0F */

	{ "ADC",		OP_RM8_reg8		},			/* 10 */
	{ "ADC",		OP_RM16_reg16	},			/* 11 */
	{ "ADC",		OP_reg8_RM8		},			/* 12 */
	{ "ADC",		OP_reg16_RM16	},			/* 13 */
	{ "ADC",		OP_AL_imm8		},			/* 14 */
	{ "ADC",		OP_AX_imm16		},			/* 15 */
	{ "PUSH",		OP_SS			},			/* 16 */
	{ "POP",		OP_SS			},			/* 17 */
	{ "SBB",		OP_RM8_reg8		},			/* 18 */
	{ "SBB",		OP_RM16_reg16	},			/* 19 */
	{ "SBB",		OP_reg8_RM8		},			/* 1A */
	{ "SBB",		OP_reg16_RM16	},			/* 1B */
	{ "SBB",		OP_AL_imm8		},			/* 1C */
	{ "SBB",		OP_AX_imm16		},			/* 1D */
	{ "PUSH",		OP_DS			},			/* 1E */
	{ "POP",		OP_DS			},			/* 1F */

	{ "AND",		OP_RM8_reg8		},			/* 20 */
	{ "AND",		OP_RM16_reg16	},			/* 21 */
	{ "AND",		OP_reg8_RM8		},			/* 22 */
	{ "AND",		OP_reg16_RM16	},			/* 23 */
	{ "AND",		OP_AL_imm8		},			/* 24 */
	{ "AND",		OP_AX_imm16		},			/* 25 */
	{ "ES:",		OP				},			/* 26 */
	{ "DAA",		OP				},			/* 27 */
	{ "SUB",		OP_RM8_reg8		},			/* 28 */
	{ "SUB",		OP_RM16_reg16	},			/* 29 */
	{ "SUB",		OP_reg8_RM8		},			/* 2A */
	{ "SUB",		OP_reg16_RM16	},			/* 2B */
	{ "SUB",		OP_AL_imm8		},			/* 2C */
	{ "SUB",		OP_AX_imm16		},			/* 2D */
	{ "CS:",		OP				},			/* 2E */
	{ "DAS",		OP				},			/* 2F */

	{ "XOR",		OP_RM8_reg8		},			/* 30 */
	{ "XOR",		OP_RM16_reg16	},			/* 31 */
	{ "XOR",		OP_reg8_RM8		},			/* 32 */
	{ "XOR",		OP_reg16_RM16	},			/* 33 */
	{ "XOR",		OP_AL_imm8		},			/* 34 */
	{ "XOR",		OP_AX_imm16		},			/* 35 */
	{ "SS:",		OP				},			/* 36 */
	{ "AAA",		OP				},			/* 37 */
	{ "CMP",		OP_RM8_reg8		},			/* 38 */
	{ "CMP",		OP_RM16_reg16	},			/* 39 */
	{ "CMP",		OP_reg8_RM8		},			/* 3A */
	{ "CMP",		OP_reg16_RM16	},			/* 3B */
	{ "CMP",		OP_AL_imm8		},			/* 3C */
	{ "CMP",		OP_AX_imm16		},			/* 3D */
	{ "DS:",		OP				},			/* 3E */
	{ "AAS",		OP				},			/* 3F */

	{ "INC",		OP_reg16		},			/* 40 */
	{ "INC",		OP_reg16		},			/* 41 */
	{ "INC",		OP_reg16		},			/* 42 */
	{ "INC",		OP_reg16		},			/* 43 */
	{ "INC",		OP_reg16		},			/* 44 */
	{ "INC",		OP_reg16		},			/* 45 */
	{ "INC",		OP_reg16		},			/* 46 */
	{ "INC",		OP_reg16		},			/* 47 */
	{ "DEC",		OP_reg16		},			/* 48 */
	{ "DEC",		OP_reg16		},			/* 49 */
	{ "DEC",		OP_reg16		},			/* 4A */
	{ "DEC",		OP_reg16		},			/* 4B */
	{ "DEC",		OP_reg16		},			/* 4C */
	{ "DEC",		OP_reg16		},			/* 4D */
	{ "DEC",		OP_reg16		},			/* 4E */
	{ "DEC",		OP_reg16		},			/* 4F */

	{ "PUSH",		OP_reg16		},			/* 50 */
	{ "PUSH",		OP_reg16		},			/* 51 */
	{ "PUSH",		OP_reg16		},			/* 52 */
	{ "PUSH",		OP_reg16		},			/* 53 */
	{ "PUSH",		OP_reg16		},			/* 54 */
	{ "PUSH",		OP_reg16		},			/* 55 */
	{ "PUSH",		OP_reg16		},			/* 56 */
	{ "PUSH",		OP_reg16		},			/* 57 */
	{ "POP",		OP_reg16		},			/* 58 */
	{ "POP",		OP_reg16		},			/* 59 */
	{ "POP",		OP_reg16		},			/* 5A */
	{ "POP",		OP_reg16		},			/* 5B */
	{ "POP",		OP_reg16		},			/* 5C */
	{ "POP",		OP_reg16		},			/* 5D */
	{ "POP",		OP_reg16		},			/* 5E */
	{ "POP",		OP_reg16		},			/* 5F */

	{ "PUSHA",		OP				},			/* 60 */
	{ "POPA",		OP				},			/* 61 */
	{ "BOUND",		OP				},			/* 62 */
	{ "???",		OP				},			/* 63 */
	{ "???",		OP				},			/* 64 */
	{ "???",		OP				},			/* 65 */
	{ "386",		OP				},			/* 66 */
	{ "???",		OP				},			/* 67 */
	{ "PUSH",		OP_imm16		},			/* 68 */
	{ "???",		OP				},			/* 69 */
	{ "PUSH",		OP_imm8			},			/* 6A */
	{ "???",		OP				},			/* 6B */
	{ "???",		OP				},			/* 6C */
	{ "???",		OP				},			/* 6D */
	{ "???",		OP				},			/* 6E */
	{ "???",		OP				},			/* 6F */

	{ "JO",			OP_short_imm8	},			/* 70 */
	{ "JNO",		OP_short_imm8	},			/* 71 */
	{ "JC",			OP_short_imm8	},			/* 72 */
	{ "JNC",		OP_short_imm8	},			/* 73 */
	{ "JZ",			OP_short_imm8	},			/* 74 */
	{ "JNZ",		OP_short_imm8	},			/* 75 */
	{ "JBE",		OP_short_imm8	},			/* 76 */
	{ "JNBE",		OP_short_imm8	},			/* 77 */
	{ "JS",			OP_short_imm8	},			/* 78 */
	{ "JNS",		OP_short_imm8	},			/* 79 */
	{ "JP",			OP_short_imm8	},			/* 7A */
	{ "JNP",		OP_short_imm8	},			/* 7B */
	{ "JL",			OP_short_imm8	},			/* 7C */
	{ "JNL",		OP_short_imm8	},			/* 7D */
	{ "JNG",		OP_short_imm8	},			/* 7E */
	{ "JG",			OP_short_imm8	},			/* 7F */

	{ "wrap",		WRAP			},			/* 80 */
	{ "wrap",		WRAP			},			/* 81 */
	{ "???",		OP				},			/* 82 */
	{ "wrap",		WRAP			},			/* 83 */
	{ "TEST",		OP_RM8_reg8		},			/* 84 */
	{ "TEST",		OP_RM16_reg16	},			/* 85 */
	{ "XCHG",		OP_reg8_RM8		},			/* 86 */
	{ "XCHG",		OP_reg16_RM16	},			/* 87 */
	{ "MOV",		OP_RM8_reg8		},			/* 88 */
	{ "MOV",		OP_RM16_reg16	},			/* 89 */
	{ "MOV",		OP_reg8_RM8		},			/* 8A */
	{ "MOV",		OP_reg16_RM16	},			/* 8B */
	{ "MOV",		OP_RM16_seg		},			/* 8C */
	{ "LEA",		OP_reg16_RM16	},			/* 8D */
	{ "MOV",		OP_seg_RM16		},			/* 8E */
	{ "wrap",		WRAP			},			/* 8F */

	{ "NOP",		OP				},			/* 90 */
	{ "XCHG",		OP_AX_reg16		},			/* 91 */
	{ "XCHG",		OP_AX_reg16		},			/* 92 */
	{ "XCHG",		OP_AX_reg16		},			/* 93 */
	{ "XCHG",		OP_AX_reg16		},			/* 94 */
	{ "XCHG",		OP_AX_reg16		},			/* 95 */
	{ "XCHG",		OP_AX_reg16		},			/* 96 */
	{ "XCHG",		OP_AX_reg16		},			/* 97 */
	{ "CBW",		OP				},			/* 98 */
	{ "CWD",		OP				},			/* 99 */
	{ "CALL",		OP_imm16_imm16	},			/* 9A */
	{ "WAIT",		OP				},			/* 9B */
	{ "PUSHF",		OP				},			/* 9C */
	{ "POPF",		OP				},			/* 9D */
	{ "SAHF",		OP				},			/* 9E */
	{ "LAHF",		OP				},			/* 9F */

	{ "MOV",		OP_AL_moff8		},			/* A0 */
	{ "MOV",		OP_AX_moff16	},			/* A1 */
	{ "MOV",		OP_moff8_AL		},			/* A2 */
	{ "MOV",		OP_moff16_AX	},			/* A3 */
	{ "MOVSB",		OP				},			/* A4 */
	{ "MOVSW",		OP				},			/* A5 */
	{ "CMPSB",		OP				},			/* A6 */
	{ "CMPSW",		OP				},			/* A7 */
	{ "TEST",		OP_AL_imm8		},			/* A8 */
	{ "TEST",		OP_AX_imm16		},			/* A9 */
	{ "STOSB",		OP				},			/* AA */
	{ "STOSW",		OP				},			/* AB */
	{ "LODSB",		OP				},			/* AC */
	{ "LODSW",		OP				},			/* AD */
	{ "SCASB",		OP				},			/* AE */
	{ "SCASW",		OP				},			/* AF */

	{ "MOV",		OP_reg8_imm8	},			/* B0 */
	{ "MOV",		OP_reg8_imm8	},			/* B1 */
	{ "MOV",		OP_reg8_imm8	},			/* B2 */
	{ "MOV",		OP_reg8_imm8	},			/* B3 */
	{ "MOV",		OP_reg8_imm8	},			/* B4 */
	{ "MOV",		OP_reg8_imm8	},			/* B5 */
	{ "MOV",		OP_reg8_imm8	},			/* B6 */
	{ "MOV",		OP_reg8_imm8	},			/* B7 */
	{ "MOV",		OP_reg16_imm16	},			/* B8 */
	{ "MOV",		OP_reg16_imm16	},			/* B9 */
	{ "MOV",		OP_reg16_imm16	},			/* BA */
	{ "MOV",		OP_reg16_imm16	},			/* BB */
	{ "MOV",		OP_reg16_imm16	},			/* BC */
	{ "MOV",		OP_reg16_imm16	},			/* BD */
	{ "MOV",		OP_reg16_imm16	},			/* BE */
	{ "MOV",		OP_reg16_imm16	},			/* BF */

	{ "wrap",		WRAP			},			/* C0 */
	{ "wrap",		WRAP			},			/* C1 */
	{ "RET",		OP_imm16		},			/* C2 */
	{ "RET",		OP				},			/* C3 */
	{ "LES",		OP_reg16_RM16	},			/* C4 */
	{ "LDS",		OP_reg16_RM16	},			/* C5 */
	{ "MOV",		OP_RM8_imm8		},			/* C6 */
	{ "MOV",		OP_RM16_imm16	},			/* C7 */
	{ "ENTER",		OP				},			/* C8 */
	{ "LEAVE",		OP				},			/* C9 */
	{ "RETF",		OP_imm16		},			/* CA */
	{ "RETF",		OP				},			/* CB */
	{ "INT",		OP_3			},			/* CC */
	{ "INT",		OP_imm8			},			/* CD */
	{ "???",		OP				},			/* CE */
	{ "IRET",		OP				},			/* CF */

	{ "wrap",		WRAP			},			/* D0 */
	{ "wrap",		WRAP			},			/* D1 */
	{ "wrap",		WRAP			},			/* D2 */
	{ "wrap",		WRAP			},			/* D3 */
	{ "AAM",		OP				},			/* D4 */
	{ "AAD",		OP				},			/* D5 */
	{ "???",		OP				},			/* D6 */
	{ "XLAT",		OP				},			/* D7 */
	{ "???",		OP				},			/* D8 */
	{ "???",		OP				},			/* D9 */
	{ "???",		OP				},			/* DA */
	{ "187",		OP				},			/* DB */
	{ "???",		OP				},			/* DC */
	{ "???",		OP				},			/* DD */
	{ "???",		OP				},			/* DE */
	{ "???",		OP				},			/* DF */

	{ "LOOPNE",		OP_imm8			},			/* E0 */
	{ "LOOPE",		OP_imm8			},			/* E1 */
	{ "LOOP",		OP_imm8			},			/* E2 */
	{ "JCXZ",		OP_imm8			},			/* E3 */
	{ "IN",			OP_AL_imm8		},			/* E4 */
	{ "IN",			OP_AX_imm16		},			/* E5 */
	{ "OUT",		OP_imm8_AL		},			/* E6 */
	{ "OUT",		OP_imm8_AX		},			/* E7 */
	{ "CALL",		OP_relimm16		},			/* E8 */
	{ "JMP",		OP_imm16		},			/* E9 */
	{ "JMP",		OP_imm16_imm16	},			/* EA */
	{ "JMP",		OP_short_imm8	},			/* EB */
	{ "IN",			OP_AL_DX		},			/* EC */
	{ "???",		OP				},			/* ED */
	{ "OUT",		OP_DX_AL		},			/* EE */
	{ "OUT",		OP_DX_AX		},			/* EF */

	{ "???",		OP				},			/* F0 */
	{ "???",		OP				},			/* F1 */
	{ "REPNE",		OP_OP			},			/* F2 */
	{ "REP",		OP_OP			},			/* F3 */
	{ "HLT",		OP				},			/* F4 */
	{ "CMC",		OP				},			/* F5 */
	{ "wrap",		WRAP			},			/* F6 */
	{ "wrap",		WRAP			},			/* F7 */
	{ "CLC",		OP				},			/* F8 */
	{ "STC",		OP				},			/* F9 */
	{ "CLI",		OP				},			/* FA */
	{ "STI",		OP				},			/* FB */
	{ "CLD",		OP				},			/* FC */
	{ "STD",		OP				},			/* FD */
	{ "wrap",		WRAP			},			/* FE */
	{ "wrap",		WRAP			}			/* FF */
};

insn_t wrapped_insn_table[256][8];

/* Base width of instruction types, a work in progress. */
int insn_base_width[43];

void
vomit_disasm_init_tables()
{
wrapped_insn_table[0x80][0] = (insn_t){ "ADD", OP_RM8_imm8 };
wrapped_insn_table[0x80][1] = (insn_t){ "OR",  OP_RM8_imm8 };
wrapped_insn_table[0x80][2] = (insn_t){ "ADC", OP_RM8_imm8 };
wrapped_insn_table[0x80][3] = (insn_t){ "SBB", OP_RM8_imm8 };
wrapped_insn_table[0x80][4] = (insn_t){ "AND", OP_RM8_imm8 };
wrapped_insn_table[0x80][5] = (insn_t){ "SUB", OP_RM8_imm8 };
wrapped_insn_table[0x80][6] = (insn_t){ "XOR", OP_RM8_imm8 };
wrapped_insn_table[0x80][7] = (insn_t){ "CMP", OP_RM8_imm8 };

wrapped_insn_table[0x81][0] = (insn_t){ "ADD", OP_RM16_imm16 };
wrapped_insn_table[0x81][1] = (insn_t){ "OR",  OP_RM16_imm16 };
wrapped_insn_table[0x81][2] = (insn_t){ "ADC", OP_RM16_imm16 };
wrapped_insn_table[0x81][3] = (insn_t){ "SBB", OP_RM16_imm16 };
wrapped_insn_table[0x81][4] = (insn_t){ "AND", OP_RM16_imm16 };
wrapped_insn_table[0x81][5] = (insn_t){ "SUB", OP_RM16_imm16 };
wrapped_insn_table[0x81][6] = (insn_t){ "XOR", OP_RM16_imm16 };
wrapped_insn_table[0x81][7] = (insn_t){ "CMP", OP_RM16_imm16 };

wrapped_insn_table[0x83][0] = (insn_t){ "ADD", OP_RM16_imm8 };
wrapped_insn_table[0x83][1] = (insn_t){ "OR",  OP_RM16_imm8 };
wrapped_insn_table[0x83][2] = (insn_t){ "ADC", OP_RM16_imm8 };
wrapped_insn_table[0x83][3] = (insn_t){ "SBB", OP_RM16_imm8 };
wrapped_insn_table[0x83][4] = (insn_t){ "AND", OP_RM16_imm8 };
wrapped_insn_table[0x83][5] = (insn_t){ "SUB", OP_RM16_imm8 };
wrapped_insn_table[0x83][6] = (insn_t){ "XOR", OP_RM16_imm8 };
wrapped_insn_table[0x83][7] = (insn_t){ "CMP", OP_RM16_imm8 };

wrapped_insn_table[0xF6][0] = (insn_t){ "TEST", OP_RM8_imm8 };
wrapped_insn_table[0xF6][1] = (insn_t){ 0, OP_UNASSIGNED };
wrapped_insn_table[0xF6][2] = (insn_t){ 0, OP_UNASSIGNED };
wrapped_insn_table[0xF6][3] = (insn_t){ 0, OP_UNASSIGNED };
wrapped_insn_table[0xF6][4] = (insn_t){ "MUL", OP_RM8 };
wrapped_insn_table[0xF6][6] = (insn_t){ "DIV", OP_RM8 };
wrapped_insn_table[0xF6][7] = (insn_t){ 0, OP_UNASSIGNED };

wrapped_insn_table[0xF7][0] = (insn_t){ "TEST", OP_RM16_imm16 };
wrapped_insn_table[0xF7][1] = (insn_t){ 0, OP_UNASSIGNED };
wrapped_insn_table[0xF7][2] = (insn_t){ 0, OP_UNASSIGNED };
wrapped_insn_table[0xF7][3] = (insn_t){ 0, OP_UNASSIGNED };
wrapped_insn_table[0xF7][4] = (insn_t){ "MUL", OP_RM16 };
wrapped_insn_table[0xF7][6] = (insn_t){ "DIV", OP_RM16 };
wrapped_insn_table[0xF7][7] = (insn_t){ 0, OP_UNASSIGNED };

wrapped_insn_table[0xD0][0] = (insn_t){ "ROL", OP_RM8 };
wrapped_insn_table[0xD0][1] = (insn_t){ "ROR", OP_RM8 };
wrapped_insn_table[0xD0][2] = (insn_t){ "RCL", OP_RM8 };
wrapped_insn_table[0xD0][3] = (insn_t){ "RCR", OP_RM8 };
wrapped_insn_table[0xD0][4] = (insn_t){ "SHL", OP_RM8 };
wrapped_insn_table[0xD0][5] = (insn_t){ "SHR", OP_RM8 };
wrapped_insn_table[0xD0][6] = (insn_t){ 0, OP_UNASSIGNED };
wrapped_insn_table[0xD0][7] = (insn_t){ "SAR", OP_RM8 };

wrapped_insn_table[0xD1][0] = (insn_t){ "ROL", OP_RM16 };
wrapped_insn_table[0xD1][1] = (insn_t){ "ROR", OP_RM16 };
wrapped_insn_table[0xD1][2] = (insn_t){ "RCL", OP_RM16 };
wrapped_insn_table[0xD1][3] = (insn_t){ "RCR", OP_RM16 };
wrapped_insn_table[0xD1][4] = (insn_t){ "SHL", OP_RM16 };
wrapped_insn_table[0xD1][5] = (insn_t){ "SHR", OP_RM16 };
wrapped_insn_table[0xD1][6] = (insn_t){ 0, OP_UNASSIGNED };
wrapped_insn_table[0xD1][7] = (insn_t){ "SAR", OP_RM16 };

wrapped_insn_table[0xD3][0] = (insn_t){ "ROL", OP_reg8_CL };
wrapped_insn_table[0xD3][1] = (insn_t){ "ROR", OP_reg8_CL };
wrapped_insn_table[0xD3][2] = (insn_t){ "RCL", OP_reg8_CL };
wrapped_insn_table[0xD3][3] = (insn_t){ "RCR", OP_reg8_CL };
wrapped_insn_table[0xD3][4] = (insn_t){ "SHL", OP_reg8_CL };
wrapped_insn_table[0xD3][5] = (insn_t){ "SHR", OP_reg8_CL };
wrapped_insn_table[0xD3][6] = (insn_t){ 0, OP_UNASSIGNED };
wrapped_insn_table[0xD3][7] = (insn_t){ "SAR", OP_reg8_CL };

wrapped_insn_table[0xFF][0] = (insn_t){ "INC", OP_RM16 };
wrapped_insn_table[0xFF][1] = (insn_t){ "DEC", OP_RM16 };
wrapped_insn_table[0xFF][2] = (insn_t){ "CALL", OP_RM16 };
wrapped_insn_table[0xFF][3] = (insn_t){ "CALL", OP_RM16 };
wrapped_insn_table[0xFF][4] = (insn_t){ "JMP", OP_RM16 };
wrapped_insn_table[0xFF][5] = (insn_t){ 0, OP_UNASSIGNED };
wrapped_insn_table[0xFF][6] = (insn_t){ "PUSH", OP_RM16 };
wrapped_insn_table[0xFF][7] = (insn_t){ 0, OP_UNASSIGNED };

wrapped_insn_table[0xFE][0] = (insn_t){ "INC", OP_RM8 };
wrapped_insn_table[0xFE][1] = (insn_t){ "DEC", OP_RM8 };
wrapped_insn_table[0xFE][2] = (insn_t){ 0, OP_UNASSIGNED };
wrapped_insn_table[0xFE][3] = (insn_t){ 0, OP_UNASSIGNED };
wrapped_insn_table[0xFE][4] = (insn_t){ 0, OP_UNASSIGNED };
wrapped_insn_table[0xFE][5] = (insn_t){ 0, OP_UNASSIGNED };
wrapped_insn_table[0xFE][6] = (insn_t){ 0, OP_UNASSIGNED };
wrapped_insn_table[0xFE][7] = (insn_t){ 0, OP_UNASSIGNED };
	insn_base_width[OP_RM8_reg8] = 1;
	insn_base_width[OP_RM16_reg16] = 1;
	insn_base_width[OP_reg8_RM8] = 1;
	insn_base_width[OP_reg16_RM16] = 1;
	insn_base_width[OP_AL_imm8] = 2;
	insn_base_width[OP_AX_imm16] = 3;
	insn_base_width[OP_CS] = 1;
	insn_base_width[OP_DS] = 1;
	insn_base_width[OP_ES] = 1;
	insn_base_width[OP_SS] = 1;
	insn_base_width[OP] = 1;
	insn_base_width[OP_reg16] = 1;
	insn_base_width[OP_imm16] = 3;
	insn_base_width[OP_relimm16] = 3;
	insn_base_width[OP_imm8] = 2;
	insn_base_width[WRAP] = 1;
	insn_base_width[OP_RM16_seg] = 1;
	insn_base_width[OP_reg16_mem16] = -1;
	insn_base_width[OP_imm16_imm16] = 5;
	insn_base_width[OP_AX_reg16] = 1;
	insn_base_width[OP_AL_moff8] = 3;
	insn_base_width[OP_AX_moff16] = 3;
	insn_base_width[OP_moff8_AL] = 3;
	insn_base_width[OP_moff16_AX] = 3;
	insn_base_width[OP_reg8_imm8] = 2;
	insn_base_width[OP_reg16_imm16] = 3;
	insn_base_width[OP_RM8_imm8] = 2;
	insn_base_width[OP_RM16_imm16] = 3;
	insn_base_width[OP_RM16_imm8] = 3;
	insn_base_width[OP_3] = 1;
	insn_base_width[OP_AX_imm8] = 2;
	insn_base_width[OP_short_imm8] = 2;
	insn_base_width[OP_AL_DX] = 1;
	insn_base_width[OP_DX_AL] = 1;
	insn_base_width[OP_DX_AX] = 1;
	insn_base_width[OP_OP] = 1;
	insn_base_width[OP_seg_RM16] = 1;
	insn_base_width[OP_imm8_AL] = 2;
	insn_base_width[OP_imm8_AX] = 2;
	insn_base_width[OP_RM8] = 1;
	insn_base_width[OP_RM16] = 1;
	insn_base_width[OP_reg8_CL] = 2;
}

const char segname[4][3] = {
	"ES", "CS", "SS", "DS"
};

const char reg16name[8][3] = {
	"AX", "CX", "DX", "BX",
	"SP", "BP", "SI", "DI",
};

const char reg8name[8][3] = {
	"AL", "CL", "DL", "BL",
	"AH", "CH", "DH", "BH",
};

