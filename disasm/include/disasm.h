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

#ifndef DISASM_H
#define DISASM_H

#include "types.h"

/* Fetch the name of a register based on a register id# */
#define R8(d)			(reg8name[(d) & 7])
#define R16(d)			(reg16name[(d) & 7])

/* Fetch the name of a segment register based on id#.
 * We AND with 3 to keep the index within bounds.
 */
#define SEG(d)			(segname[(d)])

struct insn_t {
    const char* name;
    int type;
};

extern int insn_base_width[];
extern const insn_t insn_table[];
extern insn_t wrapped_insn_table[256][8];
extern const char reg16name[8][3];
extern const char reg8name[8][3];
extern const char segname[6][3];

extern char * modrm_string(BYTE *, int);
extern bool disassemble(BYTE *, long unsigned int, char *, int);
extern int insn_width(BYTE *);
extern int modrm_width(BYTE);
extern bool is_modrm_insn(const insn_t *);

#endif
