/* 8086/cpu.c
 * 8086 emulation
 *
 * 031101:	Changed all multiplications to shifts.
 *			I want to run this on my 200MHz box too ;-)
 *
 */

#include <stdlib.h>
#include <string.h>
#include "vomit.h"
#include "debug.h"

byte cpu_state, cpu_type;
dword cpu_ips, cpu_ii;

byte cpu_opcode; /* Opcodes are no longer passed as handler arguments!! */
byte cpu_rmbyte; /* Me neither. */

#ifndef VM_NOPFQ
byte *cpu_pfq;
byte cpu_pfq_current;
byte CPU_PFQ_SIZE;
#endif

#ifdef VM_DEBUG
word g_last_nonbios_CS;
word g_last_nonbios_IP;
#endif

word *treg16[8]; byte *treg8[8]; word *tseg[8];
word *CurrentSegment, SegmentPrefix; word CS, DS, ES, SS;
word AX, BX, CX, DX, BasePointer, StackPointer, SI, DI, IP;
byte CF, DF, TF, PF, AF, ZF, SF, IF, OF;

tfunctab cpu_optable[0x100];

word segment_dummy; /* The black hole of 386 segment selectors. */

void
_OpOverride()
{
	vlog( VM_LOGMSG, "Operation size override detected!" );
}

void
_FNINIT()
{
	vlog( VM_LOGMSG, "FPU initalization attempt detected!" );
	/* skip second stuffing */
	cpu_pfq_getbyte();
}

void
cpu_init()
{
	treg16[REG_AX] = &AX; treg16[REG_BX] = &BX;
	treg16[REG_CX] = &CX; treg16[REG_DX] = &DX;
	treg16[REG_StackPointer] = &StackPointer; treg16[REG_BasePointer] = &BasePointer;
	treg16[REG_SI] = &SI; treg16[REG_DI] = &DI;

	treg8[REG_AH] = (byte *)&AX+1; treg8[REG_BH] = (byte *)&BX+1;
	treg8[REG_CH] = (byte *)&CX+1; treg8[REG_DH] = (byte *)&DX+1;
	treg8[REG_AL] = (byte *)&AX; treg8[REG_BL] = (byte *)&BX;
	treg8[REG_CL] = (byte *)&CX; treg8[REG_DL] = (byte *)&DX;

	tseg[REG_CS] = &CS; tseg[REG_DS] = &DS;
	tseg[REG_ES] = &ES; tseg[REG_SS] = &SS;
	tseg[4] = &segment_dummy;
	tseg[5] = &segment_dummy;
	tseg[6] = &segment_dummy;
	tseg[7] = &segment_dummy;

	AX = 0; BX = 0; CX = 0; DX = 0;
	BasePointer = 0; StackPointer = 0; SI = 0; DI = 0;
	DS = 0; ES = 0; SS = 0;
	CS = 0xF000; IP = 0x0000;

    CurrentSegment = &DS;

    cpu_setflags(0x0200 | CPU_STATIC_FLAGS);
	cpu_modrm_init();
	cpu_flags_init();

	cpu_ips = 40000;	/* FUCK WITH CARE */
	cpu_ii = 0;

}

void
cpu_genmap()
{
#ifndef VM_NOPFQ
	CPU_PFQ_SIZE = 4;
	cpu_pfq = malloc(CPU_PFQ_SIZE*sizeof(byte));
	cpu_pfq_current = 0;
#endif

	cpu_addinstruction( 0x00, 0xFF, _UNSUPP           );
    cpu_addinstruction( 0x00, 0x00, _ADD_RM8_reg8     );
    cpu_addinstruction( 0x01, 0x01, _ADD_RM16_reg16   );
    cpu_addinstruction( 0x02, 0x02, _ADD_reg8_RM8     );
    cpu_addinstruction( 0x03, 0x03, _ADD_reg16_RM16   );
    cpu_addinstruction( 0x04, 0x04, _ADD_AL_imm8      );
    cpu_addinstruction( 0x05, 0x05, _ADD_AX_imm16     );
	cpu_addinstruction( 0x06, 0x06, _PUSH_ES          );
	cpu_addinstruction( 0x07, 0x07, _POP_ES           );
	cpu_addinstruction( 0x08, 0x08, _OR_RM8_reg8      );
	cpu_addinstruction( 0x09, 0x09, _OR_RM16_reg16    );
	cpu_addinstruction( 0x0A, 0x0A, _OR_reg8_RM8      );
	cpu_addinstruction( 0x0B, 0x0B, _OR_reg16_RM16    );
	cpu_addinstruction( 0x0C, 0x0C, _OR_AL_imm8       );
	cpu_addinstruction( 0x0D, 0x0D, _OR_AX_imm16      );
	cpu_addinstruction( 0x0E, 0x0E, _PUSH_CS          );
	cpu_addinstruction( 0x0F, 0x0F, _POP_CS           );
	cpu_addinstruction( 0x10, 0x10, _ADC_RM8_reg8     );
    cpu_addinstruction( 0x11, 0x11, _ADC_RM16_reg16   );
    cpu_addinstruction( 0x12, 0x12, _ADC_reg8_RM8     );
    cpu_addinstruction( 0x13, 0x13, _ADC_reg16_RM16   );
    cpu_addinstruction( 0x14, 0x14, _ADC_AL_imm8      );
    cpu_addinstruction( 0x15, 0x15, _ADC_AX_imm16     );
	cpu_addinstruction( 0x16, 0x16, _PUSH_SS          );
	cpu_addinstruction( 0x17, 0x17, _POP_SS           );
	cpu_addinstruction( 0x18, 0x18, _SBB_RM8_reg8     );
	cpu_addinstruction( 0x19, 0x19, _SBB_RM16_reg16   );
    cpu_addinstruction( 0x1A, 0x1A, _SBB_reg8_RM8     );
    cpu_addinstruction( 0x1B, 0x1B, _SBB_reg16_RM16   );
    cpu_addinstruction( 0x1C, 0x1C, _SBB_AL_imm8      );
    cpu_addinstruction( 0x1D, 0x1D, _SBB_AX_imm16     );
	cpu_addinstruction( 0x1E, 0x1E, _PUSH_DS          );
	cpu_addinstruction( 0x1F, 0x1F, _POP_DS           );
	cpu_addinstruction( 0x20, 0x20, _AND_RM8_reg8     );
	cpu_addinstruction( 0x21, 0x21, _AND_RM16_reg16   );
	cpu_addinstruction( 0x22, 0x22, _AND_reg8_RM8     );
	cpu_addinstruction( 0x23, 0x23, _AND_reg16_RM16   );
	cpu_addinstruction( 0x24, 0x24, _AND_AL_imm8      );
	cpu_addinstruction( 0x25, 0x25, _AND_AX_imm16     );
	cpu_addinstruction( 0x26, 0x26, _ES               );
	cpu_addinstruction( 0x27, 0x27, _DAA              );
	cpu_addinstruction( 0x28, 0x28, _SUB_RM8_reg8     );
    cpu_addinstruction( 0x29, 0x29, _SUB_RM16_reg16   );
    cpu_addinstruction( 0x2A, 0x2A, _SUB_reg8_RM8     );
    cpu_addinstruction( 0x2B, 0x2B, _SUB_reg16_RM16   );
    cpu_addinstruction( 0x2C, 0x2C, _SUB_AL_imm8      );
    cpu_addinstruction( 0x2D, 0x2D, _SUB_AX_imm16     );
	cpu_addinstruction( 0x2E, 0x2E, _CS               );
	cpu_addinstruction( 0x2F, 0x2F, _DAS              );
	cpu_addinstruction( 0x30, 0x30, _XOR_RM8_reg8     );
	cpu_addinstruction( 0x31, 0x31, _XOR_RM16_reg16   );
	cpu_addinstruction( 0x32, 0x32, _XOR_reg8_RM8     );
	cpu_addinstruction( 0x33, 0x33, _XOR_reg16_RM16   );
	cpu_addinstruction( 0x34, 0x34, _XOR_AL_imm8      );
	cpu_addinstruction( 0x35, 0x35, _XOR_AX_imm16     );
	cpu_addinstruction( 0x36, 0x36, _SS               );
	cpu_addinstruction( 0x37, 0x37, _AAA              );
	cpu_addinstruction( 0x38, 0x38, _CMP_RM8_reg8     );
    cpu_addinstruction( 0x39, 0x39, _CMP_RM16_reg16   );
    cpu_addinstruction( 0x3A, 0x3A, _CMP_reg8_RM8     );
    cpu_addinstruction( 0x3B, 0x3B, _CMP_reg16_RM16   );
    cpu_addinstruction( 0x3C, 0x3C, _CMP_AL_imm8      );
    cpu_addinstruction( 0x3D, 0x3D, _CMP_AX_imm16     );
	cpu_addinstruction( 0x3E, 0x3E, _DS               );
	cpu_addinstruction( 0x3F, 0x3F, _AAS              );
	cpu_addinstruction( 0x40, 0x47, _INC_reg16        );
	cpu_addinstruction( 0x48, 0x4F, _DEC_reg16        );
	cpu_addinstruction( 0x50, 0x57, _PUSH_reg16       );
	cpu_addinstruction( 0x58, 0x5F, _POP_reg16        );
	cpu_addinstruction( 0x70, 0x7F, _Jcc_imm8         );
	cpu_addinstruction( 0x80, 0x80, _wrap_0x80        );
	cpu_addinstruction( 0x81, 0x81, _wrap_0x81        );
	cpu_addinstruction( 0x83, 0x83, _wrap_0x83        );
	cpu_addinstruction( 0x84, 0x84, _TEST_RM8_reg8    );
	cpu_addinstruction( 0x85, 0x85, _TEST_RM16_reg16  );
	cpu_addinstruction( 0x86, 0x86, _XCHG_reg8_RM8    );
	cpu_addinstruction( 0x87, 0x87, _XCHG_reg16_RM16  );
	cpu_addinstruction( 0x88, 0x88, _MOV_RM8_reg8     );
	cpu_addinstruction( 0x89, 0x89, _MOV_RM16_reg16   );
	cpu_addinstruction( 0x8A, 0x8A, _MOV_reg8_RM8     );
	cpu_addinstruction( 0x8B, 0x8B, _MOV_reg16_RM16   );
	cpu_addinstruction( 0x8C, 0x8C, _MOV_RM16_seg     );
	cpu_addinstruction( 0x8D, 0x8D, _LEA_reg16_mem16  );
	cpu_addinstruction( 0x8E, 0x8E, _MOV_seg_RM16     );
	cpu_addinstruction( 0x8F, 0x8F, _wrap_0x8F        );
	cpu_addinstruction( 0x90, 0x90, _NOP              );
	cpu_addinstruction( 0x91, 0x97, _XCHG_AX_reg16    );
	cpu_addinstruction( 0x98, 0x98, _CBW              );
	cpu_addinstruction( 0x99, 0x99, _CWD              );
	cpu_addinstruction( 0x9A, 0x9A, _CALL_imm16_imm16 );
	cpu_addinstruction( 0x9C, 0x9C, _PUSHF            );
	cpu_addinstruction( 0x9D, 0x9D, _POPF             );
	cpu_addinstruction( 0x9E, 0x9E, _SAHF             );
	cpu_addinstruction( 0x9F, 0x9F, _LAHF             );
	cpu_addinstruction( 0xA0, 0xA0, _MOV_AL_moff8     );
	cpu_addinstruction( 0xA1, 0xA1, _MOV_AX_moff16    );
	cpu_addinstruction( 0xA2, 0xA2, _MOV_moff8_AL     );
	cpu_addinstruction( 0xA3, 0xA3, _MOV_moff16_AX    );
	cpu_addinstruction( 0xA4, 0xA4, _MOVSB            );
	cpu_addinstruction( 0xA5, 0xA5, _MOVSW            );
	cpu_addinstruction( 0xA6, 0xA6, _CMPSB            );
	cpu_addinstruction( 0xA7, 0xA7, _CMPSW            );
	cpu_addinstruction( 0xA8, 0xA8, _TEST_AL_imm8     );
	cpu_addinstruction( 0xA9, 0xA9, _TEST_AX_imm16    );
	cpu_addinstruction( 0xAA, 0xAA, _STOSB            );
	cpu_addinstruction( 0xAB, 0xAB, _STOSW            );
	cpu_addinstruction( 0xAC, 0xAC, _LODSB            );
	cpu_addinstruction( 0xAD, 0xAD, _LODSW            );
	cpu_addinstruction( 0xAE, 0xAE, _SCASB            );
	cpu_addinstruction( 0xAF, 0xAF, _SCASW            );
	cpu_addinstruction( 0xB0, 0xB7, _MOV_reg8_imm8    );
	cpu_addinstruction( 0xB8, 0xBF, _MOV_reg16_imm16  );
	cpu_addinstruction( 0xC2, 0xC2, _RET_imm16        );
	cpu_addinstruction( 0xC3, 0xC3, _RET              );
	cpu_addinstruction( 0xC4, 0xC4, _LES_reg16_mem16  );
	cpu_addinstruction( 0xC5, 0xC5, _LDS_reg16_mem16  );
	cpu_addinstruction( 0xC6, 0xC6, _MOV_RM8_imm8     );
	cpu_addinstruction( 0xC7, 0xC7, _MOV_RM16_imm16   );
	cpu_addinstruction( 0xCA, 0xCA, _RETF_imm16       );
	cpu_addinstruction( 0xCB, 0xCB, _RETF             );
	cpu_addinstruction( 0xCC, 0xCC, _INT3             );
	cpu_addinstruction( 0xCD, 0xCD, _INT_imm8         );
	cpu_addinstruction( 0xCF, 0xCF, _IRET             );
	cpu_addinstruction( 0xD0, 0xD0, _wrap_0xD0        );
	cpu_addinstruction( 0xD1, 0xD1, _wrap_0xD1        );
	cpu_addinstruction( 0xD2, 0xD2, _wrap_0xD2        );
	cpu_addinstruction( 0xD3, 0xD3, _wrap_0xD3        );
	cpu_addinstruction( 0xD4, 0xD4, _AAM              );
	cpu_addinstruction( 0xD5, 0xD5, _AAD              );
	cpu_addinstruction( 0xD6, 0xD6, _SALC             );
	cpu_addinstruction( 0xD7, 0xD7, _XLAT             );
	cpu_addinstruction( 0xD9, 0xD9, _wrap_0xD9        );
	cpu_addinstruction( 0xE0, 0xE0, _LOOPNE_imm8      );
	cpu_addinstruction( 0xE1, 0xE1, _LOOPE_imm8       );
	cpu_addinstruction( 0xE2, 0xE2, _LOOP_imm8        );
	cpu_addinstruction( 0xE3, 0xE3, _JCXZ_imm8        );
	cpu_addinstruction( 0xE4, 0xE4, _IN_AL_imm8       );
	cpu_addinstruction( 0xE5, 0xE5, _IN_AX_imm8       );
	cpu_addinstruction( 0xE6, 0xE6, _OUT_imm8_AL      );
	cpu_addinstruction( 0xE7, 0xE7, _OUT_imm8_AX      );
	cpu_addinstruction( 0xE8, 0xE8, _CALL_imm16       );
	cpu_addinstruction( 0xE9, 0xE9, _JMP_imm16        );
	cpu_addinstruction( 0xEA, 0xEA, _JMP_imm16_imm16  );
	cpu_addinstruction( 0xEB, 0xEB, _JMP_short_imm8   );
	cpu_addinstruction( 0xEC, 0xEC, _IN_AL_DX         );
	cpu_addinstruction( 0xEE, 0xEE, _OUT_DX_AL        );
	cpu_addinstruction( 0xEF, 0xEF, _OUT_DX_AX        );
	cpu_addinstruction( 0xF2, 0xF2, _REPNE            );
	cpu_addinstruction( 0xF3, 0xF3, _REP              );
	cpu_addinstruction( 0xF4, 0xF4, _HLT              );
	cpu_addinstruction( 0xF5, 0xF5, _CMC              );
	cpu_addinstruction( 0xF6, 0xF6, _wrap_0xF6        );
	cpu_addinstruction( 0xF7, 0xF7, _wrap_0xF7        );
	cpu_addinstruction( 0xF8, 0xF8, _CLC              );
	cpu_addinstruction( 0xF9, 0xF9, _STC              );
	cpu_addinstruction( 0xFA, 0xFA, _CLI              );
	cpu_addinstruction( 0xFB, 0xFB, _STI              );
	cpu_addinstruction( 0xFC, 0xFC, _CLD              );
	cpu_addinstruction( 0xFD, 0xFD, _STD              );
	cpu_addinstruction( 0xFE, 0xFE, _wrap_0xFE        );
	cpu_addinstruction( 0xFF, 0xFF, _wrap_0xFF        );

	if( cpu_type >= INTEL_80186 )
	{
		cpu_addinstruction( 0x0F, 0x0F, _wrap_0x0F  );
		cpu_addinstruction( 0x60, 0x60, _PUSHA      );
		cpu_addinstruction( 0x61, 0x61, _POPA       );
		cpu_addinstruction( 0x62, 0x62, _BOUND      );
		cpu_addinstruction( 0x68, 0x68, _PUSH_imm16 );
		cpu_addinstruction( 0x6A, 0x6A, _PUSH_imm8  );
		cpu_addinstruction( 0xC0, 0xC0, _wrap_0xC0  );
		cpu_addinstruction( 0xC1, 0xC1, _wrap_0xC1  );
		cpu_addinstruction( 0xC8, 0xC8, _ENTER      );
		cpu_addinstruction( 0xC9, 0xC9, _LEAVE      );
	}

	/* Some cheap solutions. */
	cpu_addinstruction( 0x9B, 0x9B, _WAIT       );
	cpu_addinstruction( 0x66, 0x66, _OpOverride );
	cpu_addinstruction( 0xDB, 0xDB, _FNINIT     );
}

void
cpu_kill()
{
#ifndef VM_NOPFQ
	free( cpu_pfq );
#endif
}

void
cpu_main()
{
#ifndef VM_NOPFQ
	cpu_pfq_flush();
#endif
	for(;;) {
		cpu_opcode = cpu_pfq_getbyte();
		cpu_optable[cpu_opcode]();	/* Call instruction handler. */
#ifdef VM_DEBUG
		BCS = CS; BIP = IP - 1;
		if ( BCS != 0xF000 ) {
			g_last_nonbios_CS = BCS;
			g_last_nonbios_IP = BIP;
		}
		if ( g_debug_step ) {
			ui_kill();
			vm_debug();
			if ( g_debug_step )
				continue;
			ui_show();
		}
#endif
#ifdef VM_BREAK
		if ( g_break_pressed ) {
			g_command_mode = !g_command_mode;
			g_break_pressed = false;
			/* TODO: int_call( 9 ); */
			continue;
		}

		if( g_command_mode )
		{
			ui_command_mode();
		}
#endif

		if( TF )
		{
			/* The Trap Flag is set, so we'll execute one instruction and
			 * call INT 1 as soon as it's finished. */
			cpu_opcode = cpu_pfq_getbyte();
			cpu_optable[cpu_opcode]();

			/* NOTE: The PIT ISR won't be called below, since int_call()
			 *       clears IF. Phew. */
			int_call( 1 );
		}

		if( ++cpu_ii == cpu_ips ) {
			cpu_ii = 0;
			ui_sync();

			if( IF == 1 )
			{
				/* Call the PIT ISR. This is ugly, to say the least, and I'm sorry. */
				int_call( 8 );
			}
		}
    }
}

byte cpu_pfq_getbyte() { /* Get byte from prefetch queue and let new ops in. Modifies IP+1 */
#ifdef VM_NOPFQ
	return mem_getbyte(CS, IP++);
#else
	byte b = cpu_pfq[cpu_pfq_current];
	cpu_pfq[cpu_pfq_current] = mem_space[(CS<<4) + IP + CPU_PFQ_SIZE];
	if(++cpu_pfq_current==CPU_PFQ_SIZE) cpu_pfq_current=0;
	++IP;
	return b;
#endif
}

word cpu_pfq_getword() { /* Get word from prefetch queue... same as above, but word and IP+2 */
#ifdef VM_NOPFQ
	word w = mem_getword(CS, IP);
	IP += 2;
	return w;
#else
	word w = (word)cpu_pfq[cpu_pfq_current];
	cpu_pfq[cpu_pfq_current] = mem_space[(CS<<4) + IP + CPU_PFQ_SIZE];
	if(++cpu_pfq_current==CPU_PFQ_SIZE) cpu_pfq_current=0;
	w += (word)(cpu_pfq[cpu_pfq_current]) << 8;
	cpu_pfq[cpu_pfq_current] = mem_space[(CS<<4) + (++IP) + CPU_PFQ_SIZE];
	if(++cpu_pfq_current==CPU_PFQ_SIZE) cpu_pfq_current=0;
	++IP;
	return w;
#endif
}

#ifndef VM_NOPFQ
void
cpu_pfq_flush()
{
	/* Flush the prefetch queue. MUST be done after all jumps/calls. */
	int i;
	cpu_pfq_current = 0;
	for ( i = 0; i < CPU_PFQ_SIZE; ++i )
		cpu_pfq[i] = mem_space[( CS << 4 ) + ( IP + i )];
}
#endif

void cpu_jump(word seg, word off) { /* Jump to specified location. */
	CS = seg;
	IP = off;
#ifndef VM_NOPFQ
	cpu_pfq_flush();
#endif
}

void cpu_setflags(word flags) {		/* probably optimizeable */
	CF = (flags & 0x0001) > 0;
	PF = (flags & 0x0004) > 0;
	AF = (flags & 0x0010) > 0;
	ZF = (flags & 0x0040) > 0;
	SF = (flags & 0x0080) > 0;
	TF = (flags & 0x0100) > 0;
	IF = (flags & 0x0200) > 0;
	DF = (flags & 0x0400) > 0;
	OF = (flags & 0x0800) > 0;
}

word cpu_getflags() {
	word r=	(CF) | (PF << 2) | (AF << 4) | (ZF << 6) | (SF << 7) |
			(TF << 8) | (IF << 9) | (DF << 10) | (OF << 11) | CPU_STATIC_FLAGS;
	return r;
}

void cpu_addint(byte n, word segment, word offset) {
	mem_setword(0x0000, (n<<2), offset);
	mem_setword(0x0000, (n<<2)+2, segment);
}

void cpu_addinstruction( byte opcode_range_start, byte opcode_range_end, void (*handler)() )
{
	word i;
	for( i = opcode_range_start; i <= opcode_range_end; ++i )
	{
		cpu_optable[i] = handler;
	}
}

bool
cpu_evaluate( byte condition )
{
	switch ( condition ) {
		case  0: return OF;                       /* O          */
		case  1: return !OF;                      /* NO         */
		case  2: return CF;                       /* B, C, NAE  */
		case  3: return !CF;                      /* NB, NC, AE */
		case  4: return ZF;                       /* E, Z       */
		case  5: return !ZF;                      /* NE, NZ     */
		case  6: return ( CF | ZF );              /* BE, NA     */
		case  7: return !( CF | ZF );             /* NBE, A     */
		case  8: return SF;                       /* S          */
		case  9: return !SF;                      /* NS         */
		case 10: return PF;                       /* P, PE      */
		case 11: return !PF;                      /* NP, PO     */
		case 12: return ( SF ^ OF );              /* L, NGE     */
		case 13: return !( SF ^ OF );             /* NL, GE     */
		case 14: return ( SF ^ OF ) | ZF;         /* LE, NG     */
		case 15: return !( ( SF ^ OF ) | ZF );    /* NLE, G     */
	}
	return 0;
}

void
_WAIT()
{
	/* Do nothing? */
}

void
_UNSUPP()
{
	ui_kill();
	vlog( VM_ALERT, "%04X:%04X: Opcode %02X not supported", CS, IP, cpu_opcode );
	if( g_try_run )
	{
		dump_try();
		vm_exit( 1 );
	}
#ifdef VM_DEBUG
	vm_debug();
	ui_show();
#endif
	vm_exit( 1 );
}

