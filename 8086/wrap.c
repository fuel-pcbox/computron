/* 8086/wrap.c
 * ModR/M Instruction Wrappers
 *
 */

#include <stdio.h>
#include "vomit.h"
#include "debug.h"

void
_wrap_0x80() {
	cpu_rmbyte = cpu_pfq_getbyte();
	switch(rmreg(cpu_rmbyte)) {
		case 0: _ADD_RM8_imm8(); break;
		case 1:  _OR_RM8_imm8(); break;
		case 2: _ADC_RM8_imm8(); break;
		case 3: _SBB_RM8_imm8(); break;
		case 4: _AND_RM8_imm8(); break;
		case 5: _SUB_RM8_imm8(); break;
		case 6: _XOR_RM8_imm8(); break;
		case 7: _CMP_RM8_imm8(); break;
	}
}

void
_wrap_0x81() {
	cpu_rmbyte = cpu_pfq_getbyte();
	switch(rmreg(cpu_rmbyte)) {
		case 0: _ADD_RM16_imm16(); break;
		case 1:  _OR_RM16_imm16(); break;
		case 2: _ADC_RM16_imm16(); break;
		case 3: _SBB_RM16_imm16(); break;
		case 4: _AND_RM16_imm16(); break;
		case 5: _SUB_RM16_imm16(); break;
		case 6: _XOR_RM16_imm16(); break;
		case 7: _CMP_RM16_imm16(); break;
	}
}

void
_wrap_0x83() {
	cpu_rmbyte = cpu_pfq_getbyte();
	switch(rmreg(cpu_rmbyte)) {
		case 0: _ADD_RM16_imm8(); break;
		case 1:  _OR_RM16_imm8(); break;
		case 2: _ADC_RM16_imm8(); break;
		case 3: _SBB_RM16_imm8(); break;
		case 4: _AND_RM16_imm8(); break;
		case 5: _SUB_RM16_imm8(); break;
		case 6: _XOR_RM16_imm8(); break;
		case 7: _CMP_RM16_imm8(); break;
	}
}

void
_wrap_0x8F() {
	cpu_rmbyte = cpu_pfq_getbyte();
	switch(rmreg(cpu_rmbyte)) {
		case 0: _POP_RM16(); break;
		default: vlog( VM_ALERT, "8F /%d not wrapped", rmreg( cpu_rmbyte ));
	}
}

void
_wrap_0xC0() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	byte imm = cpu_pfq_getbyte();
	switch(rmreg(rm)) {
		case 0: *p = cpu_rol(*p, imm, 8); break;
		case 1: *p = cpu_ror(*p, imm, 8); break;
		case 2: *p = cpu_rcl(*p, imm, 8); break;
		case 3: *p = cpu_rcr(*p, imm, 8); break;
		case 4: *p = cpu_shl(*p, imm, 8); break;
		case 5: *p = cpu_shr(*p, imm, 8); break;
		case 7: *p = cpu_sar(*p, imm, 8); break;
		default: vlog( VM_ALERT, "C0 /%d not wrapped", rmreg( cpu_rmbyte ));
	}
}

void
_wrap_0xC1() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	byte imm = cpu_pfq_getbyte();
	switch(rmreg(rm)) {
		case 0: *p = cpu_rol(*p, imm, 16); break;
		case 1: *p = cpu_ror(*p, imm, 16); break;
		case 2: *p = cpu_rcl(*p, imm, 16); break;
		case 3: *p = cpu_rcr(*p, imm, 16); break;
		case 4: *p = cpu_shl(*p, imm, 16); break;
		case 5: *p = cpu_shr(*p, imm, 16); break;
		case 7: *p = cpu_sar(*p, imm, 16); break;
		default: vlog( VM_ALERT, "C1 /%d not wrapped", rmreg( cpu_rmbyte ));
	}
}

void
_wrap_0xD0() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	switch(rmreg(rm)) {
		case 0: *p = cpu_rol(*p, 1, 8); break;
		case 1: *p = cpu_ror(*p, 1, 8); break;
		case 2: *p = cpu_rcl(*p, 1, 8); break;
		case 3: *p = cpu_rcr(*p, 1, 8); break;
		case 4: *p = cpu_shl(*p, 1, 8); break;
		case 5: *p = cpu_shr(*p, 1, 8); break;
		case 7: *p = cpu_sar(*p, 1, 8); break;
		default: vlog( VM_ALERT, "D0 /%d not wrapped", rmreg( cpu_rmbyte ));
	}
}

void
_wrap_0xD1() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	switch(rmreg(rm)) {
		case 0: *p = cpu_rol(*p, 1, 16); break;
		case 1: *p = cpu_ror(*p, 1, 16); break;
		case 2: *p = cpu_rcl(*p, 1, 16); break;
		case 3: *p = cpu_rcr(*p, 1, 16); break;
		case 4: *p = cpu_shl(*p, 1, 16); break;
		case 5: *p = cpu_shr(*p, 1, 16); break;
		case 7: *p = cpu_sar(*p, 1, 16); break;
		default: vlog( VM_ALERT, "D1 /%d not wrapped", rmreg( cpu_rmbyte ));
	}
}

void
_wrap_0xD2() {
	byte rm = cpu_pfq_getbyte();
	byte *p = cpu_rmptr(rm, 8);
	switch(rmreg(rm)) {
		case 0: *p = cpu_rol(*p, cpu.regs.B.CL, 8); break;
		case 1: *p = cpu_ror(*p, cpu.regs.B.CL, 8); break;
		case 2: *p = cpu_rcl(*p, cpu.regs.B.CL, 8); break;
		case 3: *p = cpu_rcr(*p, cpu.regs.B.CL, 8); break;
		case 4: *p = cpu_shl(*p, cpu.regs.B.CL, 8); break;
		case 5: *p = cpu_shr(*p, cpu.regs.B.CL, 8); break;
		case 7: *p = cpu_sar(*p, cpu.regs.B.CL, 8); break;
		default: vlog( VM_ALERT, "D2 /%d not wrapped", rmreg( cpu_rmbyte ));
	}
}

void
_wrap_0xD3() {
	byte rm = cpu_pfq_getbyte();
	word *p = cpu_rmptr(rm, 16);
	switch(rmreg(rm)) {
		case 0: *p = cpu_rol(*p, cpu.regs.B.CL, 16); break;
		case 1: *p = cpu_ror(*p, cpu.regs.B.CL, 16); break;
		case 2: *p = cpu_rcl(*p, cpu.regs.B.CL, 16); break;
		case 3: *p = cpu_rcr(*p, cpu.regs.B.CL, 16); break;
		case 4: *p = cpu_shl(*p, cpu.regs.B.CL, 16); break;
		case 5: *p = cpu_shr(*p, cpu.regs.B.CL, 16); break;
		case 7: *p = cpu_sar(*p, cpu.regs.B.CL, 16); break;
		default: vlog( VM_ALERT, "D3 /%d not wrapped", rmreg( cpu_rmbyte ));
	}
}

void
_wrap_0xF6() {
	cpu_rmbyte = cpu_pfq_getbyte();
	switch(rmreg(cpu_rmbyte)) {
		case 0: _TEST_RM8_imm8(); break;
		case 2: _NOT_RM8(); break;
		case 3: _NEG_RM8(); break;
		case 4: _MUL_RM8(); break;
		case 5: _IMUL_RM8(); break;
		case 6: _DIV_RM8(); break;
		case 7: _IDIV_RM8(); break;
		default: vlog( VM_ALERT, "F6 /%d not wrapped", rmreg( cpu_rmbyte ));
	}
}

void
_wrap_0xF7() {
	cpu_rmbyte = cpu_pfq_getbyte();
	switch(rmreg(cpu_rmbyte)) {
		case 0: _TEST_RM16_imm16(); break;
		case 2: _NOT_RM16(); break;
		case 3: _NEG_RM16(); break;
		case 4: _MUL_RM16(); break;
		case 5: _IMUL_RM16(); break;
		case 6: _DIV_RM16(); break;
		case 7: _IDIV_RM16(); break;
		default: vlog( VM_ALERT, "F7 /%d not wrapped", rmreg( cpu_rmbyte ));
	}
}


/* 0xFE: _________WRAP ME_________ GOD DAMN IT */

void
_wrap_0xFE() {
	byte *p, rm = cpu_pfq_getbyte();
	word i;
	p = cpu_rmptr(rm, 8);
	i = *p;
	switch(rmreg(rm)) {
		case 0:
			cpu.OF = i != 255 ? 0 : 1;
			i++;
			cpu_setAF(i,*p,1);
			cpu_updflags(i, 8);
			*p = i;
			break;
		case 1:
			cpu.OF = i != 0 ? 0 : 1;
			i--;
			cpu_setAF(i,*p,1);
			cpu_updflags(i, 8);
			*p = i;
			break;
		default:
			vlog( VM_ALERT, "FE /%d not wrapped", rmreg( cpu_rmbyte ));
	}
}

void
_wrap_0xFF() {
	cpu_rmbyte = cpu_pfq_getbyte();
	switch(rmreg(cpu_rmbyte)) {
		case 0: _INC_RM16();			break;
		case 1: _DEC_RM16();			break;
		case 2: _CALL_RM16();			break;
		case 3: _CALL_FAR_mem16();		break;
		case 4: _JMP_RM16();			break;
		case 5: _JMP_FAR_mem16();		break;
		case 6: _PUSH_RM16();			break;
		case 7: vlog( VM_ALERT, "FF /%d not wrapped", rmreg( cpu_rmbyte ));
	}
}
