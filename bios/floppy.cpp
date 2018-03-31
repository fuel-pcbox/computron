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

#include "Common.h"
#include "CPU.h"
#include "floppy.h"
#include "debug.h"
#include "machine.h"
#include "DiskDrive.h"

static void bios_disk_read(FILE* fp, DiskDrive& drive, WORD cylinder, WORD head, WORD sector, WORD count, WORD segment, WORD offset)
{
    auto lba = drive.toLBA(cylinder, head, sector);

    if (options.disklog)
        vlog(LogDisk, "%s reading %u sectors at %u/%u/%u (LBA %u) to %04x:%04x", qPrintable(drive.name()), count, cylinder, head, sector, lba, segment, offset);

    void* destination = g_cpu->memoryPointer(segment, offset);
    fread(destination, drive.bytesPerSector(), count, fp);
}

static void bios_disk_write(FILE* fp, DiskDrive& drive, WORD cylinder, WORD head, WORD sector, WORD count, WORD segment, WORD offset)
{
    auto lba = drive.toLBA(cylinder, head, sector);

    if (options.disklog)
        vlog(LogDisk, "%s writing %u sectors at %u/%u/%u (LBA %u) from %04x:%04x", qPrintable(drive.name()), count, cylinder, head, sector, lba, segment, offset);

    void* source = g_cpu->memoryPointer(segment, offset);
    fwrite(source, drive.bytesPerSector(), count, fp);
}

static void bios_disk_verify(FILE* fp, DiskDrive& drive, WORD cylinder, WORD head, WORD sector, WORD count, WORD segment, WORD offset)
{
    auto lba = drive.toLBA(cylinder, head, sector);

    if (options.disklog)
        vlog(LogDisk, "%s verifying %u sectors at %u/%u/%u (LBA %u)", qPrintable(drive.name()), count, cylinder, head, sector, lba);

    BYTE dummy[count * drive.bytesPerSector()];
    WORD veri = fread(dummy, drive.bytesPerSector(), count, fp);
    if (veri != count)
        vlog(LogAlert, "veri != count, something went wrong");

    // FIXME: Actually compare something..
    Q_UNUSED(segment);
    Q_UNUSED(offset);
}

void bios_disk_call(CPU& cpu, DiskCallFunction function)
{
    // This is a hack to support the custom Computron BIOS. We should not be here in PE=1 mode.
    ASSERT(!cpu.getPE());

    WORD cylinder = g_cpu->getCH() | ((g_cpu->getCL() & 0xc0) << 2);
    WORD sector = g_cpu->getCL() & 0x3f;
    BYTE driveIndex = g_cpu->getDL();
    BYTE head = g_cpu->getDH();
    BYTE sectorCount = g_cpu->getAL();
    FILE* fp;
    DWORD lba;

    DiskDrive* drive { nullptr };
    switch (driveIndex) {
    case 0x00: drive = &cpu.machine().floppy0(); break;
    case 0x01: drive = &cpu.machine().floppy1(); break;
    case 0x80: drive = &cpu.machine().fixed0(); break;
    case 0x81: drive = &cpu.machine().fixed1(); break;
    }

    BYTE error = FD_NO_ERROR;

    if (!drive || !drive->present()) {
        if (options.disklog)
            vlog(LogDisk, "Drive %02X not ready", drive);
        if (!(driveIndex & 0x80))
            error = FD_CHANGED_OR_REMOVED;
        else
            error = FD_TIMEOUT;
        goto epilogue;
    }

    lba = drive->toLBA(cylinder, head, sector);
    if (lba > drive->sectors()) {
        if (options.disklog)
            vlog(LogDisk, "%s bogus sector request (LBA %u)", qPrintable(drive->name()), lba);
        error = FD_TIMEOUT;
        goto epilogue;
    }

    if ((sector > drive->sectorsPerTrack()) || (head >= drive->heads())) {
        if (options.disklog)
            vlog(LogDisk, "%s request out of geometrical bounds (%u/%u/%u)", qPrintable(drive->name()), cylinder, head, sector);
        error = FD_TIMEOUT;
        goto epilogue;
    }

    fp = fopen(qPrintable(drive->imagePath()), function == WriteSectors ? "rb+" : "rb");
    if (!fp) {
        vlog(LogDisk, "PANIC: Could not access drive %d image!", drive);
        hard_exit(1);
    }

    fseek(fp, lba * drive->bytesPerSector(), SEEK_SET);

    switch (function) {
    case ReadSectors:
        bios_disk_read(fp, *drive, cylinder, head, sector, sectorCount, cpu.getES(), cpu.getBX());
        break;
    case WriteSectors:
        bios_disk_write(fp, *drive, cylinder, head, sector, sectorCount, cpu.getES(), cpu.getBX());
        break;
    case VerifySectors:
        bios_disk_verify(fp, *drive, cylinder, head, sector, sectorCount, cpu.getES(), cpu.getBX());
        break;
    }

    error = FD_NO_ERROR;
    fclose(fp);

epilogue:
    if (error == FD_NO_ERROR)
        cpu.setCF(0);
    else {
        cpu.setCF(1);
        cpu.setAL(0);
    }

    cpu.setAH(error);
    cpu.writeUnmappedMemory8(0x441, error);
}
