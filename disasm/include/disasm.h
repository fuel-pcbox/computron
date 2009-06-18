#ifndef __DISASM_H__
#define __DISASM_H__

#include <stdint.h>
#include <stdbool.h>

/* Create some handy type definitions */
typedef uint16_t WORD;
typedef uint8_t BYTE;

/* Extract the register id# from a ModR/M byte */
#define MODRM_REG(rm)	(((rm) >> 3) & 7)

/* Fetch the name of a register based on a register id# */
#define R8(d)			(reg8name[(d) & 7])
#define R16(d)			(reg16name[(d) & 7])

/* Fetch the name of a segment register based on id#.
 * We AND with 3 to keep the index within bounds.
 */
#define SEG(d)			(segname[(d) & 3])

/* Construct a 16-bit word from two 8-bit bytes */
#define MAKEWORD(l, m)	(((m) << 8) | (l))

typedef struct {
	const char *name;
	int type;
} insn_t;

extern int insn_base_width[];
extern const insn_t insn_table[];
extern insn_t wrapped_insn_table[256][8];
extern const char reg16name[8][3];
extern const char reg8name[8][3];
extern const char segname[4][3];

extern char * modrm_string(BYTE *, int);
extern bool disassemble(BYTE *, long unsigned int, char *, int);
extern int insn_width(BYTE *);
extern int modrm_width(BYTE);
extern bool is_modrm_insn(const insn_t *);

#endif
