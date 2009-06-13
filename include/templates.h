#ifndef __templates_h__
#define __templates_h__

#define DEFAULT_RM8_reg8( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		byte value = modrm_read8( rm ); \
		modrm_update8( helper ## 8 ( value, *treg8[rmreg( rm )] )); \
	}

#define DEFAULT_RM16_reg16( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		word value = modrm_read16( rm ); \
		modrm_update16( helper ## 16 ( value, *treg16[rmreg( rm )] )); \
	}

#define DEFAULT_reg8_RM8( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		byte value = modrm_read8( rm ); \
		*treg8[rmreg( rm )] = helper ## 8 ( *treg8[rmreg( rm )], value ); \
	}

#define DEFAULT_reg16_RM16( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		word value = modrm_read16( rm ); \
		*treg16[rmreg( rm )] = helper ## 16 ( *treg16[rmreg( rm )], value ); \
	}

#define DEFAULT_RM8_imm8( helper, name ) \
	void name() { \
		byte value = modrm_read8( cpu.rmbyte ); \
		modrm_update8( helper ## 8 ( value, cpu_pfq_getbyte() )); \
	}

#define DEFAULT_RM16_imm16( helper, name ) \
	void name() { \
		word value = modrm_read16( cpu.rmbyte ); \
		modrm_update16( helper ## 16 ( value, cpu_pfq_getword() )); \
	}

#define DEFAULT_RM16_imm8( helper, name ) \
	void name() { \
		word value = modrm_read16( cpu.rmbyte ); \
		modrm_update16( helper ## 16 ( value, signext( cpu_pfq_getbyte() ) )); \
	}

#define DEFAULT_AL_imm8( helper, name ) \
	void name() { \
		cpu.regs.B.AL = helper ## 8 ( cpu.regs.B.AL, cpu_pfq_getbyte() ); \
	}

#define DEFAULT_AX_imm16( helper, name ) \
	void name() { \
		cpu.regs.W.AX = helper ## 16 ( cpu.regs.W.AX, cpu_pfq_getword() ); \
	}

#define READONLY_RM8_reg8( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		helper ## 8 ( modrm_read8( rm ), *treg8[rmreg( rm )] ); \
	}

#define READONLY_RM16_reg16( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		helper ## 16 ( modrm_read16( rm ), *treg16[rmreg( rm )] ); \
	}

#define READONLY_reg8_RM8( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		helper ## 8 ( *treg8[rmreg( rm )], modrm_read8( rm ) ); \
	}

#define READONLY_reg16_RM16( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		helper ## 16 ( *treg16[rmreg( rm )], modrm_read16( rm ) ); \
	}

#define READONLY_RM8_imm8( helper, name ) \
	void name() { \
		byte value = modrm_read8( cpu.rmbyte ); \
		helper ## 8 ( value, cpu_pfq_getbyte() ); \
	}

#define READONLY_RM16_imm16( helper, name ) \
	void name() { \
		word value = modrm_read16( cpu.rmbyte ); \
		helper ## 16 ( value, cpu_pfq_getword() ); \
	}

#define READONLY_RM16_imm8( helper, name ) \
	void name() { \
		word value = modrm_read16( cpu.rmbyte ); \
		helper ## 16 ( value, signext( cpu_pfq_getbyte() ) ); \
	}

#define READONLY_AL_imm8( helper, name ) \
	void name() { \
		helper ## 8 ( cpu.regs.B.AL, cpu_pfq_getbyte() ); \
	}

#define READONLY_AX_imm16( helper, name ) \
	void name() { \
		helper ## 16 ( cpu.regs.W.AX, cpu_pfq_getword() ); \
	}

#endif /* __templates_h__ */
