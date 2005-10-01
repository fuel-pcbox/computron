#ifndef __templates_h__
#define __templates_h__

#define DEFAULT_RM8_reg8( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		byte *p = cpu_rmptr( rm, 8 ); \
		*p = helper( *p, *treg8[rmreg( rm )], 8 ); \
	}

#define DEFAULT_RM16_reg16( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		word *p = cpu_rmptr( rm, 16 ); \
		*p = helper( *p, *treg16[rmreg( rm )], 16 ); \
	}

#define DEFAULT_reg8_RM8( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		byte *p = cpu_rmptr( rm, 8 ); \
		*treg8[rmreg( rm )] = helper( *treg8[rmreg( rm )], *p, 8 ); \
	}

#define DEFAULT_reg16_RM16( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		word *p = cpu_rmptr( rm, 16 ); \
		*treg16[rmreg( rm )] = helper( *treg16[rmreg( rm )], *p, 16 ); \
	}

#define DEFAULT_RM8_imm8( helper, name ) \
	void name() { \
		byte *p = cpu_rmptr( cpu_rmbyte, 8 ); \
		*p = helper( *p, cpu_pfq_getbyte(), 8 ); \
	}

#define DEFAULT_RM16_imm16( helper, name ) \
	void name() { \
		word *p = cpu_rmptr( cpu_rmbyte, 16 ); \
		*p = helper( *p, cpu_pfq_getword(), 16 ); \
	}

#define DEFAULT_RM16_imm8( helper, name ) \
	void name() { \
		word *p = cpu_rmptr( cpu_rmbyte, 16 ); \
		*p = helper( *p, signext( cpu_pfq_getbyte() ), 16 ); \
	}

#define DEFAULT_AL_imm8( helper, name ) \
	void name() { \
		*treg8[REG_AL] = helper( *treg8[REG_AL], cpu_pfq_getbyte(), 8 ); \
	}

#define DEFAULT_AX_imm16( helper, name ) \
	void name() { \
		AX = helper( AX, cpu_pfq_getword(), 16 ); \
	}

#define READONLY_RM8_reg8( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		byte *p = cpu_rmptr( rm, 8 ); \
		helper( *p, *treg8[rmreg( rm )], 8 ); \
	}

#define READONLY_RM16_reg16( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		word *p = cpu_rmptr( rm, 16 ); \
		helper( *p, *treg16[rmreg( rm )], 16 ); \
	}

#define READONLY_reg8_RM8( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		byte *p = cpu_rmptr( rm, 8 ); \
		helper( *treg8[rmreg( rm )], *p, 8 ); \
	}

#define READONLY_reg16_RM16( helper, name ) \
	void name() { \
		byte rm = cpu_pfq_getbyte(); \
		word *p = cpu_rmptr( rm, 16 ); \
		helper( *treg16[rmreg( rm )], *p, 16 ); \
	}

#define READONLY_RM8_imm8( helper, name ) \
	void name() { \
		byte *p = cpu_rmptr( cpu_rmbyte, 8 ); \
		helper( *p, cpu_pfq_getbyte(), 8 ); \
	}

#define READONLY_RM16_imm16( helper, name ) \
	void name() { \
		word *p = cpu_rmptr( cpu_rmbyte, 16 ); \
		helper( *p, cpu_pfq_getword(), 16 ); \
	}

#define READONLY_RM16_imm8( helper, name ) \
	void name() { \
		word *p = cpu_rmptr( cpu_rmbyte, 16 ); \
		helper( *p, signext( cpu_pfq_getbyte() ), 16 ); \
	}

#define READONLY_AL_imm8( helper, name ) \
	void name() { \
		helper( *treg8[REG_AL], cpu_pfq_getbyte(), 8 ); \
	}

#define READONLY_AX_imm16( helper, name ) \
	void name() { \
		helper( AX, cpu_pfq_getword(), 16 ); \
	}

#endif /* __templates_h__ */
