#ifndef __templates_h__
#define __templates_h__

#define DEFAULT_RM8_reg8(helper, name) \
	void name(vomit_cpu_t *cpu) { \
		BYTE rm = vomit_cpu_pfq_getbyte(cpu); \
		BYTE value = vomit_cpu_modrm_read8(cpu, rm); \
		vomit_cpu_modrm_update8(cpu, helper ## 8(cpu, value, *cpu->treg8[rmreg(rm)])); \
	}

#define DEFAULT_RM16_reg16(helper, name) \
	void name(vomit_cpu_t *cpu) { \
		BYTE rm = vomit_cpu_pfq_getbyte(cpu); \
		WORD value = vomit_cpu_modrm_read16(cpu, rm); \
		vomit_cpu_modrm_update16(cpu, helper ## 16(cpu, value, *cpu->treg16[rmreg(rm)])); \
	}

#define DEFAULT_reg8_RM8(helper, name) \
	void name(vomit_cpu_t *cpu) { \
		BYTE rm = vomit_cpu_pfq_getbyte(cpu); \
		BYTE value = vomit_cpu_modrm_read8(cpu, rm); \
		*cpu->treg8[rmreg(rm)] = helper ## 8(cpu, *cpu->treg8[rmreg(rm)], value); \
	}

#define DEFAULT_reg16_RM16(helper, name) \
	void name(vomit_cpu_t *cpu) { \
		BYTE rm = vomit_cpu_pfq_getbyte(cpu); \
		WORD value = vomit_cpu_modrm_read16(cpu, rm); \
		*cpu->treg16[rmreg(rm)] = helper ## 16(cpu, *cpu->treg16[rmreg(rm)], value); \
	}

#define DEFAULT_RM8_imm8(helper, name) \
	void name(vomit_cpu_t *cpu) { \
		BYTE value = vomit_cpu_modrm_read8(cpu, cpu->rmbyte); \
		vomit_cpu_modrm_update8(cpu, helper ## 8(cpu, value, vomit_cpu_pfq_getbyte(cpu))); \
	}

#define DEFAULT_RM16_imm16(helper, name) \
	void name(vomit_cpu_t *cpu) { \
		WORD value = vomit_cpu_modrm_read16(cpu, cpu->rmbyte); \
		vomit_cpu_modrm_update16(cpu, helper ## 16(cpu, value, vomit_cpu_pfq_getword(cpu))); \
	}

#define DEFAULT_RM16_imm8(helper, name) \
	void name(vomit_cpu_t *cpu) { \
		WORD value = vomit_cpu_modrm_read16(cpu, cpu->rmbyte); \
		vomit_cpu_modrm_update16(cpu, helper ## 16(cpu, value, signext(vomit_cpu_pfq_getbyte(cpu)))); \
	}

#define DEFAULT_AL_imm8(helper, name) \
	void name(vomit_cpu_t *cpu) { \
		cpu->regs.B.AL = helper ## 8(cpu, cpu->regs.B.AL, vomit_cpu_pfq_getbyte(cpu)); \
	}

#define DEFAULT_AX_imm16(helper, name) \
	void name(vomit_cpu_t *cpu) { \
		cpu->regs.W.AX = helper ## 16(cpu, cpu->regs.W.AX, vomit_cpu_pfq_getword(cpu)); \
	}

#define READONLY_RM8_reg8(helper, name) \
	void name(vomit_cpu_t *cpu) { \
		BYTE rm = vomit_cpu_pfq_getbyte(cpu); \
		helper ## 8(cpu, vomit_cpu_modrm_read8(cpu, rm), *cpu->treg8[rmreg(rm)]); \
	}

#define READONLY_RM16_reg16(helper, name) \
	void name(vomit_cpu_t *cpu) { \
		BYTE rm = vomit_cpu_pfq_getbyte(cpu); \
		helper ## 16(cpu, vomit_cpu_modrm_read16(cpu, rm), *cpu->treg16[rmreg(rm)]); \
	}

#define READONLY_reg8_RM8(helper, name) \
	void name(vomit_cpu_t *cpu) { \
		BYTE rm = vomit_cpu_pfq_getbyte(cpu); \
		helper ## 8(cpu, *cpu->treg8[rmreg(rm)], vomit_cpu_modrm_read8(cpu, rm)); \
	}

#define READONLY_reg16_RM16(helper, name) \
	void name(vomit_cpu_t *cpu) { \
		BYTE rm = vomit_cpu_pfq_getbyte(cpu); \
		helper ## 16(cpu, *cpu->treg16[rmreg(rm)], vomit_cpu_modrm_read16(cpu, rm)); \
	}

#define READONLY_RM8_imm8(helper, name) \
	void name(vomit_cpu_t *cpu) { \
		BYTE value = vomit_cpu_modrm_read8(cpu, cpu->rmbyte); \
		helper ## 8(cpu, value, vomit_cpu_pfq_getbyte(cpu)); \
	}

#define READONLY_RM16_imm16( helper, name ) \
	void name(vomit_cpu_t *cpu) { \
		WORD value = vomit_cpu_modrm_read16(cpu, cpu->rmbyte); \
		helper ## 16(cpu, value, vomit_cpu_pfq_getword(cpu)); \
	}

#define READONLY_RM16_imm8(helper, name) \
	void name(vomit_cpu_t *cpu) { \
		WORD value = vomit_cpu_modrm_read16(cpu, cpu->rmbyte); \
		helper ## 16(cpu, value, signext(vomit_cpu_pfq_getbyte(cpu))); \
	}

#define READONLY_AL_imm8(helper, name) \
	void name(vomit_cpu_t *cpu) { \
		helper ## 8(cpu, cpu->regs.B.AL, vomit_cpu_pfq_getbyte(cpu)); \
	}

#define READONLY_AX_imm16(helper, name) \
	void name(vomit_cpu_t *cpu) { \
		helper ## 16(cpu, cpu->regs.W.AX, vomit_cpu_pfq_getword(cpu)); \
	}

#endif /* __templates_h__ */
