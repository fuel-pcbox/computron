/*
 * Copyright (C) 2003-2011 Andreas Kling <kling@webkit.org>
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

#ifndef __vomit_h__
#define __vomit_h__

#include "types.h"

#define MAX_FILESIZE	524288		/* 512kB is max "loadfile" size */
#define MAX_FN_LENGTH	128

void vm_exit(int);
void vm_kill();

WORD kbd_hit();
WORD kbd_getc();
BYTE kbd_pop_raw();

void vomit_init();

struct vomit_options_t {
    bool trace;
    bool disklog;
    bool trapint;
    bool iopeek;
    bool start_in_debug;
};

extern vomit_options_t options;

bool vomit_in_vretrace();

#define ASSERT_VALID_SEGMENT_INDEX(segmentIndex) VM_ASSERT(static_cast<int>(segmentIndex) >= 0 && static_cast<int>(segmentIndex) <= 5)

inline DWORD vomit_toFlatAddress(WORD segment, WORD offset)
{
    return (segment << 4) + offset;
}

inline void vomit_write16ToPointer(WORD* pointer, WORD value)
{
#ifdef VOMIT_BIG_ENDIAN
    *pointer = V_BYTESWAP(value);
#else
    *pointer = value;
#endif
}

inline DWORD vomit_read32FromPointer(DWORD* pointer)
{
#ifdef VOMIT_BIG_ENDIAN
    return V_BYTESWAP(*pointer);
#else
    return *pointer;
#endif
}

inline void vomit_write32ToPointer(DWORD* pointer, DWORD value)
{
#ifdef VOMIT_BIG_ENDIAN
#error IMPLEMENT ME
#else
    *pointer = value;
#endif
}

inline WORD vomit_read16FromPointer(WORD* pointer)
{
#ifdef VOMIT_BIG_ENDIAN
    return V_BYTESWAP(*pointer);
#else
    return *pointer;
#endif
}

inline WORD vomit_signExtend(BYTE b)
{
    if (b & 0x80)
        return b | 0xFF00;
    else
        return b;
}

inline int vomit_modRMRegisterPart(int rmbyte)
{
    return (rmbyte >> 3) & 7;
}

/* Construct a 16-bit word from two 8-bit bytes */
#define MAKEWORD(l, m)	(((m) << 8) | (l))

#define LSW(d) ((d)&0xFFFF)
#define MSW(d) (((d)&0xFFFF0000)>>16)

#define LSB(w) ((w)&0xFF)
#define MSB(w) (((w)&0xFF00)>>8)

#endif
