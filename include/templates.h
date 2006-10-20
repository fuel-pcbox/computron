#ifndef __templates_h__
#define __templates_h__

#define DEFAULT_RM8_reg8( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		byte value = modrm_read8( rm ); \
		modrm_update8( helper( value, *treg8[rmreg( rm )], 8 )); \
	}

#define DEFAULT_RM16_reg16( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		word value = modrm_read16( rm ); \
		modrm_update16( helper( value, *treg16[rmreg( rm )], 16 )); \
	}

#define DEFAULT_reg8_RM8( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		byte value = modrm_read8( rm ); \
		*treg8[rmreg( rm )] = helper( *treg8[rmreg( rm )], value, 8 ); \
	}

#define DEFAULT_reg16_RM16( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		word value = modrm_read16( rm ); \
		*treg16[rmreg( rm )] = helper( *treg16[rmreg( rm )], value, 16 ); \
	}

#define DEFAULT_RM8_imm8( helper, name ) \
	void name() { \
		byte value = modrm_read8( cpu_rmbyte ); \
		modrm_update8( helper( value, cpu_pfq_getbyte(), 8 )); \
	}

#define DEFAULT_RM16_imm16( helper, name ) \
	void name() { \
		word value = modrm_read16( cpu_rmbyte ); \
		modrm_update16( helper( value, cpu_pfq_getword(), 16 )); \
	}

#define DEFAULT_RM16_imm8( helper, name ) \
	void name() { \
		word value = modrm_read16( cpu_rmbyte ); \
		modrm_update16( helper( value, signext( cpu_pfq_getbyte() ), 16 )); \
	}

#define DEFAULT_AL_imm8( helper, name ) \
	void name() { \
		cpu.regs.B.AL = helper( cpu.regs.B.AL, cpu_pfq_getbyte(), 8 ); \
	}

#define DEFAULT_AX_imm16( helper, name ) \
	void name() { \
		cpu.regs.W.AX = helper( cpu.regs.W.AX, cpu_pfq_getword(), 16 ); \
	}

#define READONLY_RM8_reg8( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		helper( modrm_read8( rm ), *treg8[rmreg( rm )], 8 ); \
	}

#define READONLY_RM16_reg16( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		helper( modrm_read16( rm ), *treg16[rmreg( rm )], 16 ); \
	}

#define READONLY_reg8_RM8( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		helper( *treg8[rmreg( rm )], modrm_read8( rm ), 8 ); \
	}

#define READONLY_reg16_RM16( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		helper( *treg16[rmreg( rm )], modrm_read16( rm ), 16 ); \
	}

#define READONLY_RM8_imm8( helper, name ) \
	void name() { \
		byte value = modrm_read8( cpu_rmbyte ); \
		helper( value, cpu_pfq_getbyte(), 8 ); \
	}

#define READONLY_RM16_imm16( helper, name ) \
	void name() { \
		word value = modrm_read16( cpu_rmbyte ); \
		helper( value, cpu_pfq_getword(), 16 ); \
	}

#define READONLY_RM16_imm8( helper, name ) \
	void name() { \
		word value = modrm_read16( cpu_rmbyte ); \
		helper( value, signext( cpu_pfq_getbyte() ), 16 ); \
	}

#define READONLY_AL_imm8( helper, name ) \
	void name() { \
		helper( cpu.regs.B.AL, cpu_pfq_getbyte(), 8 ); \
	}

#define READONLY_AX_imm16( helper, name ) \
	void name() { \
		helper( cpu.regs.W.AX, cpu_pfq_getword(), 16 ); \
	}

#endif /* __templates_h__ */
