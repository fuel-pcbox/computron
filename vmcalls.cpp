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
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

static void vm_handleE6(CPU& cpu);

void vm_call8(CPU& cpu, WORD port, BYTE data) {
    switch (port) {
    case 0xE0:
        vlog(LogAlert, "Interrupt %02X, function %04X requested", cpu.getBL(), cpu.getAX());
        if (cpu.getBL() == 0x15 && cpu.getAH() == 0x87) {
            vlog(LogAlert, "MoveBlock GDT{ %04X:%04X } x %04X", cpu.getES(), cpu.getSI(), cpu.getCX());
        }
        break;
    case 0xE6:
        vm_handleE6(cpu);
        break;
    case 0xE2:
        bios_disk_call(cpu, ReadSectors);
        break;
    case 0xE3:
        bios_disk_call(cpu, WriteSectors);
        break;
    case 0xE4:
        bios_disk_call(cpu, VerifySectors);
        break;
    default:
        vlog(LogAlert, "vm_call8: Unhandled write, %02X -> %04X", data, port);
        hard_exit(0);
        break;
    }
}

static DiskDrive* diskDriveForBIOSIndex(Machine& machine, BYTE index)
{
    switch (index) {
    case 0x00: return &machine.floppy0(); break;
    case 0x01: return &machine.floppy1(); break;
    case 0x80: return &machine.fixed0(); break;
    case 0x81: return &machine.fixed1(); break;
    }
    return nullptr;
}

void vm_handleE6(CPU& cpu)
{
    extern WORD kbd_hit();
    extern WORD kbd_getc();

    struct    tm *t;
    time_t    curtime;
    struct    timeval timv;
    DWORD tick_count;
    DiskDrive* drive;

    switch (cpu.getAX()) {
    case 0x1601:
        if (kbd_hit()) {
            cpu.setAX(kbd_hit());
            cpu.setZF(0);
        } else {
            cpu.setAX(0);
            cpu.setZF(1);
        }
        break;
    case 0x1A00:
        // Interrupt 1A, 00: Get RTC tick count
        cpu.setAL(0); // Midnight flag.
        curtime = time(nullptr);
        t = localtime(&curtime);
        tick_count = ((t->tm_hour*3600) + (t->tm_min*60) + (t->tm_sec)) * 18.206; // yuck..
        gettimeofday(&timv, NULL);
        tick_count += timv.tv_usec / 54926.9471602768;
#ifdef CT_DETERMINISTIC
        tick_count = 0x12345678;
#endif
        cpu.setCX(tick_count >> 16);
        cpu.setDX(tick_count & 0xFFFF);
        cpu.writeUnmappedMemory16(0x046C, tick_count & 0xFFFF);
        cpu.writeUnmappedMemory16(0x046D, tick_count >> 16);
        break;
    case 0x1300:
        drive = diskDriveForBIOSIndex(cpu.machine(), cpu.getDL());
        if (drive && drive->present()) {
            cpu.setAH(FD_NO_ERROR);
            cpu.setCF(0);
        } else {
            cpu.setAH(FD_CHANGED_OR_REMOVED);
            cpu.setCF(1);
        }
        cpu.writeUnmappedMemory8(cpu.getDL() < 2 ? 0x0441 : 0x0474, cpu.getAH());
        break;
    case 0x1308:
        drive = diskDriveForBIOSIndex(cpu.machine(), cpu.getDL());
        if (drive && drive->present()) {
            cpu.setAL(0);
            cpu.setAH(FD_NO_ERROR);
            cpu.setBL(drive->floppyTypeForCMOS());
            cpu.setBH(0);
            cpu.setCH(drive->cylinders() & 0xFF); // Tracks.
            cpu.setCL(((drive->cylinders() >> 2) & 0xC0) | (drive->sectorsPerTrack() & 0x3F)); // Sectors per track.
            cpu.setDH(drive->heads() - 1); // Sides.

            if (cpu.getDL() < 2) {
                cpu.setDL(cpu.machine().floppy0().present() + cpu.machine().floppy1().present());
            } else {
                cpu.setDL(cpu.machine().fixed0().present() + cpu.machine().fixed1().present());
            }

            vlog(LogDisk, "Reporting %s geometry: %u tracks, %u spt, %u heads", qPrintable(drive->name()), drive->cylinders(), drive->sectorsPerTrack(), drive->heads());

            // FIXME: ES:DI should points to some wacky Disk Base Table.
            cpu.setCF(0);
        } else {
            if (cpu.getDL() < 2)
                cpu.setAH(FD_CHANGED_OR_REMOVED);
            else if (cpu.getDL() > 1)
                cpu.setAH(FD_FIXED_NOT_READY);
            cpu.setCF(1);
        }

        break;
    case 0x1315:
        drive = diskDriveForBIOSIndex(cpu.machine(), cpu.getDL());
        if (drive && drive->present()) {
            cpu.setAH(0x01); // Diskette, no change detection present.
            if (cpu.getDL() > 1) {
                cpu.setAH(0x03); // FIXED DISK :-)
                // If fixed disk, CX:DX = sectors.
                cpu.setDX(drive->sectors());
                cpu.setCX(drive->sectors() >> 16);
            }
            cpu.setCF(0);
        } else {
            // Drive not present.
            cpu.setAH(0);
            cpu.setCF(1);
        }
        break;
    case 0x1318:
        drive = diskDriveForBIOSIndex(cpu.machine(), cpu.getDL());
        if (drive && drive->present()) {
            vlog(LogDisk, "Setting media type for %s:", qPrintable(drive->name()));
            vlog(LogDisk, "%d sectors per track", cpu.getCL());
            vlog(LogDisk, "%d tracks", cpu.getCH());

            // FIXME: ES:DI should point to a Wacky DBT
            cpu.setAH(0);
            cpu.setCF(0);
        } else {
            cpu.setCF(1);
            cpu.setAH(0x80); // No media in drive
        }
        break;

    case 0x1600:
        cpu.setAX(kbd_getc());
        break;

    case 0x1700:
        // Interrupt 17, 00: Print character on LPT
        {
            char tmp[80];
            sprintf(tmp, "prn%d.txt", cpu.getDX());
            FILE* fpdrv = fopen(tmp, "a");
            fputc(cpu.getCL(), fpdrv);
            fclose(fpdrv);
        }
        break;

    case 0x1A01:
        // Interrupt 1A, 01: Set RTC tick count
        vlog(LogAlert, "INT 1A,01: Attempt to set tick counter to %lu", (DWORD)(cpu.getCX() << 16) | cpu.getDX());
        break;

    case 0x1A05:
        // Interrupt 1A, 05: Set BIOS date
        vlog(LogAlert, "INT 1A,05: Attempt to set BIOS date to %02X-%02X-%04X", cpu.getDH(), cpu.getDL(), cpu.getCX());
        break;

    /* 0x3333: Is Drive Present?
     * DL = Drive
     *
     * Returns:
     * AL = 0x01 if drive is present
     *    = 0x00 if not
     *
     * CF = !AL
     */
    case 0x3333:
        drive = diskDriveForBIOSIndex(cpu.machine(), cpu.getDL());
        if (drive && drive->present()) {
            cpu.setAL(1);
            cpu.setCF(0);
        } else {
            cpu.setAL(0);
            cpu.setCF(1);
        }
        break;

    default:
        vlog(LogAlert, "Unknown VM call %04X received!!", cpu.getAX());
        //hard_exit(0);
        break;
    }
}
