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
_wrap_0xC0()
{
	byte rm = cpu_pfq_getbyte();
	byte value = modrm_read8( rm );
	byte imm = cpu_pfq_getbyte();

	switch( rmreg(rm) )
	{
		case 0: modrm_update8( cpu_rol( value, imm, 8 )); break;
		case 1: modrm_update8( cpu_ror( value, imm, 8 )); break;
		case 2: modrm_update8( cpu_rcl( value, imm, 8 )); break;
		case 3: modrm_update8( cpu_rcr( value, imm, 8 )); break;
		case 4: modrm_update8( cpu_shl( value, imm, 8 )); break;
		case 5: modrm_update8( cpu_shr( value, imm, 8 )); break;
		case 7: modrm_update8( cpu_sar( value, imm, 8 )); break;
		default: vlog( VM_ALERT, "C0 /%d not wrapped", rmreg( rm ));
	}
}

void
_wrap_0xC1()
{
	byte rm = cpu_pfq_getbyte();
	word value = modrm_read16( rm );
	byte imm = cpu_pfq_getbyte();

	switch( rmreg(rm) )
	{
		case 0: modrm_update16( cpu_rol( value, imm, 16)); break;
		case 1: modrm_update16( cpu_ror( value, imm, 16)); break;
		case 2: modrm_update16( cpu_rcl( value, imm, 16)); break;
		case 3: modrm_update16( cpu_rcr( value, imm, 16)); break;
		case 4: modrm_update16( cpu_shl( value, imm, 16)); break;
		case 5: modrm_update16( cpu_shr( value, imm, 16)); break;
		case 7: modrm_update16( cpu_sar( value, imm, 16)); break;
		default: vlog( VM_ALERT, "C1 /%d not wrapped", rmreg( rm ));
	}
}

void
_wrap_0xD0()
{
	byte rm = cpu_pfq_getbyte();
	byte value = modrm_read8( rm );

	switch( rmreg(rm) )
	{
		case 0: modrm_update8( cpu_rol( value, 1, 8 )); break;
		case 1: modrm_update8( cpu_ror( value, 1, 8 )); break;
		case 2: modrm_update8( cpu_rcl( value, 1, 8 )); break;
		case 3: modrm_update8( cpu_rcr( value, 1, 8 )); break;
		case 4: modrm_update8( cpu_shl( value, 1, 8 )); break;
		case 5: modrm_update8( cpu_shr( value, 1, 8 )); break;
		case 7: modrm_update8( cpu_sar( value, 1, 8 )); break;
		default: vlog( VM_ALERT, "D0 /%d not wrapped", rmreg( rm ));
	}
}

void
_wrap_0xD1()
{
	byte rm = cpu_pfq_getbyte();
	word value = modrm_read16( rm );

	switch( rmreg(rm) )
	{
		case 0: modrm_update16( cpu_rol( value, 1, 16 )); break;
		case 1: modrm_update16( cpu_ror( value, 1, 16 )); break;
		case 2: modrm_update16( cpu_rcl( value, 1, 16 )); break;
		case 3: modrm_update16( cpu_rcr( value, 1, 16 )); break;
		case 4: modrm_update16( cpu_shl( value, 1, 16 )); break;
		case 5: modrm_update16( cpu_shr( value, 1, 16 )); break;
		case 7: modrm_update16( cpu_sar( value, 1, 16 )); break;
		default: vlog( VM_ALERT, "D1 /%d not wrapped", rmreg( rm ));
	}
}

void
_wrap_0xD2()
{
	byte rm = cpu_pfq_getbyte();
	byte value = modrm_read8( rm );

	switch( rmreg(rm) )
	{
		case 0: modrm_update8( cpu_rol( value, cpu.regs.B.CL, 8 )); break;
		case 1: modrm_update8( cpu_ror( value, cpu.regs.B.CL, 8 )); break;
		case 2: modrm_update8( cpu_rcl( value, cpu.regs.B.CL, 8 )); break;
		case 3: modrm_update8( cpu_rcr( value, cpu.regs.B.CL, 8 )); break;
		case 4: modrm_update8( cpu_shl( value, cpu.regs.B.CL, 8 )); break;
		case 5: modrm_update8( cpu_shr( value, cpu.regs.B.CL, 8 )); break;
		case 7: modrm_update8( cpu_sar( value, cpu.regs.B.CL, 8 )); break;
		default: vlog( VM_ALERT, "D2 /%d not wrapped", rmreg( rm ));
	}
}

void
_wrap_0xD3()
{
	byte rm = cpu_pfq_getbyte();
	word value = modrm_read16( rm );

	switch( rmreg(rm) )
	{
		case 0: modrm_update16( cpu_rol( value, cpu.regs.B.CL, 16 )); break;
		case 1: modrm_update16( cpu_ror( value, cpu.regs.B.CL, 16 )); break;
		case 2: modrm_update16( cpu_rcl( value, cpu.regs.B.CL, 16 )); break;
		case 3: modrm_update16( cpu_rcr( value, cpu.regs.B.CL, 16 )); break;
		case 4: modrm_update16( cpu_shl( value, cpu.regs.B.CL, 16 )); break;
		case 5: modrm_update16( cpu_shr( value, cpu.regs.B.CL, 16 )); break;
		case 7: modrm_update16( cpu_sar( value, cpu.regs.B.CL, 16 )); break;
		default: vlog( VM_ALERT, "D3 /%d not wrapped", rmreg( rm ));
	}
}

void
_wrap_0xF6()
{
	cpu_rmbyte = cpu_pfq_getbyte();

	switch( rmreg(cpu_rmbyte) )
	{
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
_wrap_0xF7()
{
	cpu_rmbyte = cpu_pfq_getbyte();

	switch( rmreg(cpu_rmbyte) )
	{
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


void
_wrap_0xFE()
{
	/* TODO: Is this really correct? Really? */
	byte rm = cpu_pfq_getbyte();
	byte value = modrm_read8( rm );

	word i = value;

	switch( rmreg(rm) )
	{
		case 0:
			cpu.OF = value == 0xFF;
			i++;
			cpu_setAF( i, value, 1 );
			cpu_updflags(i, 8);
			modrm_update8( value + 1 );
			break;
		case 1:
			cpu.OF = i == 0;
			i--;
			cpu_setAF( i, value, 1 );
			cpu_updflags(i, 8);
			modrm_update8( value - 1 );
			break;
		default:
			vlog( VM_ALERT, "FE /%d not wrapped", rmreg( rm ));
			return;
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
