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

#ifndef VGA_MEMORY_H
#define VGA_MEMORY_H

#include "types.h"
#include <QtCore/QRect>

class QImage;
class VCpu;

class VGAMemory
{
public:
    VGAMemory(VCpu*);
    ~VGAMemory();

    void write8(DWORD address, BYTE value);
    void write16(DWORD address, WORD value);
    BYTE read8(DWORD address);
    WORD read16(DWORD address);

    /*!
        Ask the VGAMemory object to synchronize its internal
        palette against the 6845 palette.
     */
    void syncPalette();

    /*!
        Returns the specified pixel plane.
	Valid indices are 0 through 3.
     */
    BYTE *plane(int index) const;

    QImage *modeImage(BYTE mode) const;

    bool isDirty() const;

    QRect dirtyRect() const;

    void clearDirty();

private:
    struct Private;
    Private *d;
};

#endif
