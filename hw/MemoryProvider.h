// Computron x86 PC Emulator
// Copyright (C) 2003-2018 Andreas Kling <awesomekling@gmail.com>
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY ANDREAS KLING ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANDREAS KLING OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#pragma once

#include "debug.h"
#include "types.h"

class MemoryProvider {
public:
    MemoryProvider() { }
    virtual ~MemoryProvider() { }

    // pls no use :(
    virtual BYTE* memoryPointer(DWORD address);

    virtual BYTE read8(DWORD address);
    virtual WORD read16(DWORD address);
    virtual DWORD read32(DWORD address);
    virtual void write8(DWORD address, BYTE);
    virtual void write16(DWORD address, WORD);
    virtual void write32(DWORD address, DWORD);

    template<typename T> T read(DWORD address);
    template<typename T> void write(DWORD address, T);
};

template<typename T> inline T MemoryProvider::read(DWORD address)
{
    if (sizeof(T) == 1)
        return read8(address);
    if (sizeof(T) == 2)
        return read16(address);
    ASSERT(sizeof(T) == 4);
    return read32(address);
}

template<typename T> inline void MemoryProvider::write(DWORD address, T data)
{
    if (sizeof(T) == 1)
        return write8(address, data);
    if (sizeof(T) == 2)
        return write16(address, data);
    ASSERT(sizeof(T) == 4);
    return write32(address, data);
}
