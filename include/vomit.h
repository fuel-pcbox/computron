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

#ifndef VOMIT_H
#define VOMIT_H

#include "types.h"
#include <string>

#define MAX_FILESIZE	524288		/* 512kB is max "loadfile" size */
#define MAX_FN_LENGTH	128

void vomit_exit(int exitCode);

template<typename T> struct BitSizeOfType { static const int bits = sizeof(T) * 8; };

template<typename T> struct SignedTypeMaker { };
template<> struct SignedTypeMaker<BYTE> { typedef SIGNED_BYTE SignedType; };
template<> struct SignedTypeMaker<WORD> { typedef SIGNED_WORD SignedType; };
template<> struct SignedTypeMaker<DWORD> { typedef SIGNED_DWORD SignedType; };
template<> struct SignedTypeMaker<QWORD> { typedef SIGNED_QWORD SignedType; };

struct VomitOptions {
    VomitOptions() { }
    bool trace { false };
    bool disklog { false };
    bool trapint { false };
    bool iopeek { false };
    bool start_in_debug { false };
    bool memdebug { false };
    bool vgadebug { false };
    bool novlog { false };
    bool pedebug { false };
    std::string file_to_run;
};

extern VomitOptions options;

#define ASSERT_VALID_SEGMENT_INDEX(segmentIndex) VM_ASSERT(static_cast<int>(segmentIndex) >= 0 && static_cast<int>(segmentIndex) <= 5)

inline DWORD vomit_toFlatAddress(WORD segment, DWORD offset)
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

inline void CRASH()
{
    WORD* p = reinterpret_cast<WORD*>(0xBB0DB00F);
    *p = 0xDEAD;
}

template<typename T>
inline T vomit_signExtend(BYTE value)
{
    if (!(value & 0x80))
        return value;
    if (BitSizeOfType<T>::bits == 16)
        return value | 0xFF00;
    if (BitSizeOfType<T>::bits == 32)
        return value | 0xFFFFFF00;
    if (BitSizeOfType<T>::bits == 64)
        return value | 0xFFFFFFFFFFFFFF00;
}

template<typename T>
inline T vomit_signExtend(WORD value)
{
    if (!(value & 0x8000))
        return value;
    if (BitSizeOfType<T>::bits == 32)
        return value | 0xFFFF0000;
    if (BitSizeOfType<T>::bits == 64)
        return value | 0xFFFFFFFFFFFF0000;
}

inline int vomit_modRMRegisterPart(int rmbyte)
{
    return (rmbyte >> 3) & 7;
}

inline WORD vomit_MSW(DWORD d)
{
    return (d >> 16) & 0xFFFF;
}

inline WORD vomit_LSW(DWORD d)
{
    return d & 0xFFFF;
}

inline WORD vomit_MSB(WORD w)
{
    return (w >> 8) & 0xFF;
}

inline WORD vomit_LSB(WORD w)
{
    return w & 0xFF;
}

inline WORD vomit_MAKEWORD(BYTE msb, BYTE lsb)
{
    return (msb << 8) | lsb;
}

inline DWORD vomit_MAKEDWORD(WORD msw, WORD lsw)
{
    return (msw << 16) | lsw;
}

inline QWORD makeQWORD(DWORD msw, DWORD lsw)
{
    return ((QWORD)msw << 32) | lsw;
}

#endif
