/*
 * Copyright (C) 2003-2018 Andreas Kling <awesomekling@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ANDREAS KLING ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANDREAS KLING OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef INSN_TYPES_H
#define INSN_TYPES_H

#define OP_UNASSIGNED			0

#define OP_RM16_reg16			1
#define OP_reg8_RM8				2
#define OP_reg16_RM16			3
#define OP_AL_imm8				4
#define OP_AX_imm16				5
#define OP_CS					6
#define OP_DS					7
#define OP_ES					8
#define OP_SS					9
#define OP						10
#define OP_reg16				11
#define OP_imm16				12
#define OP_imm8					13
#define WRAP					14
#define	OP_RM16_seg				15
#define OP_reg16_mem16			16
#define OP_imm16_imm16			17
#define OP_AX_reg16				18
#define OP_AL_moff8				19
#define OP_AX_moff16			20
#define OP_moff8_AL				21
#define OP_moff16_AX			22
#define OP_reg8_imm8			23
#define OP_reg16_imm16			24
#define OP_RM8_imm8				25
#define OP_RM16_imm16			26
#define OP_3					27
#define OP_AX_imm8				28
#define OP_short_imm8			29
#define OP_AL_DX				30
#define OP_DX_AL				31
#define OP_DX_AX				32
#define OP_OP					33
#define OP_seg_RM16				34
#define OP_imm8_AL				35
#define OP_imm8_AX				36
#define OP_relimm16				37
#define OP_RM16_imm8			38
#define OP_RM8					39
#define OP_RM16					40
#define OP_RM8_reg8				41
#define OP_reg8_CL              42
#define OP_dummy_mem16          43

#endif
