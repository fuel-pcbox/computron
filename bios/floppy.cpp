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

#include "vomit.h"
#include "vcpu.h"
#include "floppy.h"
#include "debug.h"

char drv_imgfile[4][MAX_FN_LENGTH];
char drv_title[4][MAX_DRV_TITLESIZE+1];
BYTE drv_status[4], drv_type[4];
DWORD drv_spt[4], drv_heads[4], drv_sectors[4], drv_sectsize[4];

void vomit_set_drive_image(int drive_id, const char* filename)
{
    strcpy(drv_imgfile[drive_id], filename);
    vlog(VM_DISKLOG, "Drive %u image changed to %s", drive_id, filename);
}

static WORD chs2lba(BYTE drive, WORD cyl, BYTE head, WORD sector)
{
    return (sector - 1) +
           (head * drv_spt[drive]) +
           (cyl * drv_spt[drive] * drv_heads[drive]);
}

static void floppy_read(FILE* fp, BYTE drive, WORD cylinder, WORD head, WORD sector, WORD count, WORD segment, WORD offset)
{
    WORD lba = chs2lba(drive, cylinder, head, sector);

    if (options.disklog)
        vlog(VM_DISKLOG, "Drive %d reading %d sectors at %d/%d/%d (LBA %d) to %04X:%04X [offset=0x%x]", drive, count, cylinder, head, sector, lba, segment, offset, lba*drv_sectsize[drive]);

    void* destination = g_cpu->memoryPointer(segment, offset);
    ssize_t bytesRead = fread(destination, drv_sectsize[drive], count, fp);
#ifdef VOMIT_DETECT_UNINITIALIZED_ACCESS
    for (DWORD address = segment * 16 + offset; address < segment * 16 + offset + BYTEsRead; ++address)
        g_cpu->markDirty(address);
#else
    Q_UNUSED(bytesRead);
#endif
}

static void floppy_write(FILE* fp, BYTE drive, WORD cylinder, WORD head, WORD sector, WORD count, WORD segment, WORD offset)
{
    WORD lba = chs2lba(drive, cylinder, head, sector);

    if (options.disklog)
        vlog(VM_DISKLOG, "Drive %d writing %d sectors at %d/%d/%d (LBA %d) from %04X:%04X", drive, count, cylinder, head, sector, lba, segment, offset);

    void* source = g_cpu->memoryPointer(segment, offset);
    fwrite(source, drv_sectsize[drive], count, fp);
}

static void floppy_verify(FILE* fp, BYTE drive, WORD cylinder, WORD head, WORD sector, WORD count, WORD segment, WORD offset)
{
    WORD lba = chs2lba(drive, cylinder, head, sector);

    if (options.disklog)
        vlog(VM_DISKLOG, "Drive %d verifying %d sectors at %d/%d/%d (LBA %d)", drive, count, cylinder, head, sector, lba);

    BYTE dummy[count * drv_sectsize[drive]];
    WORD veri = fread(dummy, drv_sectsize[drive], count, fp);
    if (veri != count)
        vlog(VM_ALERT, "veri != count, something went wrong");

    // FIXME: Actually compare something..
}

void bios_disk_call(DiskCallFunction function)
{
    FILE* fp;
    WORD lba;

    WORD cylinder = g_cpu->regs.B.CH;
    WORD sector = g_cpu->regs.B.CL;
    BYTE drive = g_cpu->regs.B.DL;
    BYTE head = g_cpu->regs.B.DH;
    BYTE sectorCount = g_cpu->regs.B.AL;

    if (drive >= 0x80)
        drive = drive - 0x80 + 2;

    BYTE error;

    if (!drv_status[drive]) {
        if (options.disklog)
            vlog(VM_DISKLOG, "Drive %02X not ready", drive);
        if (drive < 2)
            error = FD_CHANGED_OR_REMOVED;
        else if (drive > 1)
            error = FD_TIMEOUT;
        goto epilogue;
    }

    lba = chs2lba(drive, cylinder, head, sector);
    if (lba > drv_sectors[drive]) {
        if (options.disklog)
            vlog(VM_DISKLOG, "Drive %d bogus sector request (LBA %d)", drive, lba);
        error = FD_TIMEOUT;
        goto epilogue;
    }

    if ((sector > drv_spt[drive]) || (head >= drv_heads[drive])) {
        if (options.disklog)
            vlog(VM_DISKLOG, "%04X:%04X Drive %d request out of geometrical bounds (%d/%d/%d)", g_cpu->getCS(), g_cpu->getIP(), drive, cylinder, head, sector);
        error = FD_TIMEOUT;
        goto epilogue;
    }

    fp = fopen(drv_imgfile[drive], function == WriteSectors ? "rb+" : "rb");
    if (!fp) {
        vlog(VM_DISKLOG, "PANIC: Could not access drive %d image!", drive);
        vm_exit(1);
    }

    fseek(fp, lba * drv_sectsize[drive], SEEK_SET);

    switch (function) {
    case ReadSectors:
        floppy_read(fp, drive, cylinder, head, sector, sectorCount, g_cpu->ES, g_cpu->regs.W.BX);
        break;
    case WriteSectors:
        floppy_write(fp, drive, cylinder, head, sector, sectorCount, g_cpu->ES, g_cpu->regs.W.BX);
        break;
    case VerifySectors:
        floppy_verify(fp, drive, cylinder, head, sector, sectorCount, g_cpu->ES, g_cpu->regs.W.BX);
        break;
    }

    error = FD_NO_ERROR;
    fclose(fp);

epilogue:
    if (error == FD_NO_ERROR)
        g_cpu->setCF(0);
    else {
        g_cpu->setCF(1);
        g_cpu->regs.B.AL = 0x00;
    }

    g_cpu->regs.B.AH = error;

    BYTE* bda_fd_status = g_cpu->memoryPointer(0x0000, 0x0441);
    *bda_fd_status = error;
}
