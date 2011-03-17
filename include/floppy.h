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

#ifndef __floppy_h__
#define __floppy_h__

#include "vomit.h"

#define FD_NO_ERROR             0x00
#define FD_BAD_COMMAND          0x01
#define FD_BAD_ADDRESS_MARK     0x02
#define FD_WRITE_PROTECT_ERROR  0x03
#define FD_SECTOR_NOT_FOUND     0x04
#define FD_FIXED_RESET_FAIL     0x05
#define FD_CHANGED_OR_REMOVED   0x06
#define FD_SEEK_FAIL            0x40
#define FD_TIMEOUT              0x80
#define FD_FIXED_NOT_READY      0xAA

enum DiskCallFunction { ReadSectors, WriteSectors, VerifySectors };
void bios_disk_call(DiskCallFunction);

extern BYTE drv_status[];
extern char drv_imgfile[][MAX_FN_LENGTH];
extern DWORD drv_spt[];
extern DWORD drv_heads[];
extern DWORD drv_sectors[];
extern DWORD drv_sectsize[];
extern BYTE drv_type[];

#endif /* __floppy_h__ */
