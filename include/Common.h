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

#pragma once

#include "types.h"
#include <QString>

#define ALWAYS_INLINE __attribute__ ((always_inline))

#define MAX_FN_LENGTH	128

void hard_exit(int exitCode);

struct RuntimeOptions {
    bool trace { false };
    bool disklog { false };
    bool trapint { false };
    bool iopeek { false };
    bool start_in_debug { false };
    bool memdebug { false };
    bool vgadebug { false };
    bool novlog { false };
    bool pedebug { false };
    bool vlogcycle { false };
    QString autotestPath;
    QString configPath;
};

extern RuntimeOptions options;

inline DWORD realModeAddressToPhysicalAddress(WORD segment, DWORD offset)
{
    return (segment << 4) + offset;
}

inline void write16ToPointer(WORD* pointer, WORD value)
{
#ifdef CT_BIG_ENDIAN
    *pointer = V_BYTESWAP(value);
#else
    *pointer = value;
#endif
}

inline DWORD read32FromPointer(DWORD* pointer)
{
#ifdef CT_BIG_ENDIAN
    return V_BYTESWAP(*pointer);
#else
    return *pointer;
#endif
}

inline WORD read16FromPointer(WORD* pointer)
{
#ifdef CT_BIG_ENDIAN
    return V_BYTESWAP(*pointer);
#else
    return *pointer;
#endif
}

inline void CRASH()
{
    WORD* p = reinterpret_cast<WORD*>(0xBB0DB00F);
    *p = 0xDEAD;
}
