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

#include "Common.h"
#include "CPU.h"
#include "floppy.h"
#include "vga.h"
#include "debug.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

static void vga_scrollup(BYTE x1, BYTE y1, BYTE x2, BYTE y2, BYTE num, BYTE attr);
static void vga_scrolldown(BYTE x1, BYTE y1, BYTE x2, BYTE y2, BYTE num, BYTE attr);
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
        bios_disk_call(ReadSectors);
        break;
    case 0xE3:
        bios_disk_call(WriteSectors);
        break;
    case 0xE4:
        bios_disk_call(VerifySectors);
        break;
    case 0xE7:
        vga_scrollup(cpu.getCL(), cpu.getCH(), cpu.getDL(), cpu.getDH(), cpu.getAL(), cpu.getBH());
        break;
    case 0xE8:
        vga_scrolldown(cpu.getCL(), cpu.getCH(), cpu.getDL(), cpu.getDH(), cpu.getAL(), cpu.getBH());
        break;
    default:
        vlog(LogAlert, "vm_call8: Unhandled write, %02X -> %04X", data, port);
        hard_exit(0);
        break;
    }
}

void vm_handleE6(CPU& cpu)
{
    extern WORD kbd_hit();
    extern WORD kbd_getc();

    struct    tm *t;
    time_t    curtime;
    struct    timeval timv;
    DWORD    tick_count, tracks;
    BYTE    drive;
    FILE    *fpdrv;

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
        drive = cpu.getDL();
        if (drive >= 0x80)
            drive = drive - 0x80 + 2;
        if (drv_status[drive] != 0) {
            cpu.setAH(FD_NO_ERROR);
            cpu.setCF(0);
        } else {
            cpu.setAH(FD_CHANGED_OR_REMOVED);
            cpu.setCF(1);
        }
        cpu.writeUnmappedMemory8(drive < 2 ? 0x0441 : 0x0474, cpu.getAH());
        break;
    case 0x1308:
        drive = cpu.getDL();
        if(drive>=0x80) drive = drive - 0x80 + 2;
        if(drv_status[drive]!=0) {
            tracks = (drv_sectors[drive] / drv_spt[drive] / drv_heads[drive]) - 2;
            cpu.setAL(0);
            cpu.setAH(FD_NO_ERROR);
            cpu.setBL(drv_type[drive]);
            cpu.setBH(0);
            cpu.setCH(tracks & 0xFF); // Tracks.
            cpu.setCL(((tracks >> 2) & 0xC0) | (drv_spt[drive] & 0x3F)); // Sectors per track.
            cpu.setDH(drv_heads[drive] - 1); // Sides.

            if (drive < 2)
                cpu.setDL(drv_status[0] + drv_status[1]);
            else
                cpu.setDL(drv_status[2] + drv_status[3]);

            vlog(LogDisk, "Reporting disk%d geo: %d tracks, %d spt, %d sides", drive, tracks, drv_spt[drive], drv_heads[drive]);

            cpu.setCH(tracks & 0xFF);
            cpu.setCL(cpu.getCL() | (tracks & 0x00030000) >> 10);

            // FIXME: ES:DI should points to some wacky Disk Base Table.
            cpu.setCF(0);
        } else {
            if (drive < 2)
                cpu.setAH(FD_CHANGED_OR_REMOVED);
            else if(drive > 1)
                cpu.setAH(FD_FIXED_NOT_READY);
            cpu.setCF(1);
        }

        break;
    case 0x1315:
        drive = cpu.getDL();
        if (drive >= 0x80)
            drive = drive - 0x80 + 2;
        if (drv_status[drive] != 0) {
            cpu.setAH(0x01); // Diskette, no change detection present.
            if (drive > 1) {
                cpu.setAH(0x03); // FIXED DISK :-)
                // If fixed disk, CX:DX = sectors.
                cpu.setDX(drv_sectors[drive]);
                cpu.setCX((drv_sectors[drive] << 16));
            }
            cpu.setCF(0);
        } else {
            // Drive not present.
            cpu.setAH(0);
            cpu.setCF(1);
        }
        break;
    case 0x1318:
        drive = cpu.getDL();
        if (drive >= 0x80)
            drive = drive - 0x80 + 2;
        if (drv_status[drive]) {
            vlog(LogDisk, "Setting media type for drive %d:", drive);
            vlog(LogDisk, "%d sectors per track", cpu.getCL());
            vlog(LogDisk, "%d tracks", cpu.getCH());

            /* Wacky DBT. */
            cpu.setES(0x820E);
            cpu.setDI(0x0503);
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
            fpdrv = fopen(tmp, "a");
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
        drive = cpu.getDL();
        if (drive >= 0x80)
            drive = drive - 0x80 + 2;
        if (drv_status[drive] != 0) {
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

void vga_scrollup(BYTE x1, BYTE y1, BYTE x2, BYTE y2, BYTE num, BYTE attr)
{
    BYTE *videoMemory = g_cpu->unmappedMemoryPointer(0xb8000);

    // TODO: Scroll graphics when in graphics mode (using text coordinates)

    vlog(LogVGA, "vga_scrollup(%d, %d, %d, %d, %d)", x1, y1, x2, y2, num);
    if ((num == 0) || (num > g_cpu->readUnmappedMemory8(0x484) + 1)) {
        for (BYTE y = y1; y <= y2; ++y) {
            for (BYTE x = x1; x < x2; ++x) {
                videoMemory[(y * 160 + x * 2) + 0] = 0x20;
                videoMemory[(y * 160 + x * 2) + 1] = attr;
            }
        }
        return;
    }
    for (BYTE i = 0; i < num; ++i) {
        for (BYTE y = y1; y < y2; ++y) {
            for (BYTE x = x1; x < x2; ++x) {
                videoMemory[(y * 160 + x * 2) + 0] = videoMemory[(((y+1)*160)+x*2)+0];
                videoMemory[(y * 160 + x * 2) + 1] = videoMemory[(((y+1)*160)+x*2)+1];
            }
        }
        for (BYTE x = x1; x < x2; ++x) {
            videoMemory[(y2 * 160 + x * 2) + 0] = 0x20;
            videoMemory[(y2 * 160 + x * 2) + 1] = attr;
        }
        y2--;
    }
}

void vga_scrolldown(BYTE x1, BYTE y1, BYTE x2, BYTE y2, BYTE num, BYTE attr)
{
    BYTE *videoMemory = g_cpu->unmappedMemoryPointer(0xb8000);

    // TODO: Scroll graphics when in graphics mode (using text coordinates)

    vlog(LogVGA, "vga_scrolldown(%d, %d, %d, %d, %d)", x1, y1, x2, y2, num);
    if ((num == 0) || (num > g_cpu->readUnmappedMemory8(0x484) + 1)) {
        for (BYTE y = y1; y <= y2; ++y) {
            for (BYTE x = x1; x < x2; ++x) {
                videoMemory[(y * 160 + x * 2) + 0] = 0x20;
                videoMemory[(y * 160 + x * 2) + 1] = attr;
            }
        }
        return;
    }
    for (BYTE i = 0; i < num; ++i) {
        for (BYTE y = y1; y < y2; ++y) {
            for (BYTE x = x1; x < x2; ++x) {
                videoMemory[(y * 160 + x * 2) + 0] = videoMemory[(((y+1)*160)+x*2)+0];
                videoMemory[(y * 160 + x * 2) + 1] = videoMemory[(((y+1)*160)+x*2)+1];
            }
        }
        for (BYTE x = x1; x < x2; ++x) {
            videoMemory[(y2 * 160 + x * 2) + 0] = 0x20;
            videoMemory[(y2 * 160 + x * 2) + 1] = attr;
        }
        y2--;
    }
}
