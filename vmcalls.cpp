// VM Calls
//
// This thing should eventually go away.
// It is a stopgap measure to get things running without having to implement all I/O devices.

#include "vomit.h"
#include "vcpu.h"
#include "floppy.h"
#include "vga.h"
#include "debug.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

static void vga_scrollup(BYTE x1, BYTE y1, BYTE x2, BYTE y2, BYTE num, BYTE attr);
static void vm_handleE6(VCpu* cpu);

void vm_call8(VCpu* cpu, WORD port, BYTE data) {
    switch (port) {
    case 0xE0:
        vlog(VM_ALERT, "Interrupt %02X, function %04X requested", cpu->regs.B.BL, cpu->regs.W.AX);
        cpu->dumpAll();
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
        vga_scrollup(cpu->regs.B.CL, cpu->regs.B.CH, cpu->regs.B.DL, cpu->regs.B.DH, cpu->regs.B.AL, cpu->regs.B.BH);
        break;
    default:
        vlog(VM_ALERT, "vm_call8: Unhandled write, %02X -> %04X", data, port);
        vm_exit(0);
        break;
    }
}

void vm_handleE6(VCpu* cpu)
{
    struct    tm *t;
    time_t    curtime;
    struct    timeval timv;
    DWORD    tick_count, tracks;
    BYTE    drive;
    FILE    *fpdrv;

    switch (cpu->regs.W.AX) {
    case 0x1601:
        if (kbd_hit()) {
            cpu->regs.W.AX = kbd_hit();
            cpu->setZF(0);
        } else {
            cpu->regs.W.AX = 0x0000;
            cpu->setZF(1);
        }
        break;
    case 0x1A00:
        // Interrupt 1A, 00: Get RTC tick count
        cpu->regs.B.AL = 0; /* midnight flag */
        curtime = time((time_t) NULL);
        t = localtime(&curtime);
        tick_count = ((t->tm_hour*3600) + (t->tm_min*60) + (t->tm_sec)) * 18.206; // yuck..
        gettimeofday(&timv, NULL);
        tick_count += timv.tv_usec / 54926.9471602768;
        cpu->regs.W.CX = tick_count >> 16;
        cpu->regs.W.DX = tick_count & 0xFFFF;
        cpu->writeUnmappedMemory16(0x046C, tick_count & 0xFFFF);
        cpu->writeUnmappedMemory16(0x046D, tick_count >> 16);
        break;
    case 0x1300:
        drive = cpu->regs.B.DL;
        if (drive >= 0x80)
            drive = drive - 0x80 + 2;
        if (drv_status[drive] != 0) {
            cpu->regs.B.AH = FD_NO_ERROR;
            cpu->setCF(0);
        } else {
            cpu->regs.B.AH = FD_CHANGED_OR_REMOVED;
            cpu->setCF(1);
        }
        cpu->writeUnmappedMemory8(drive < 2 ? 0x0441 : 0x0474, cpu->regs.B.AH);
        break;
#if 0
    case 0x1305:
        drive = cpu->regs.B.DL;
        if (drive >= 0x80)
            drive = drive - 0x80 + 2;
        if (drv_status[drive] != 0) {
            track = (cpu->regs.B.CH | ((cpu->regs.B.CL & 0xC0) << 2)) + 1;
            head = cpu->regs.B.DH;
            vlog(VM_DISKLOG, "Drive %d: Formatting track %lu, head %d.", drive, track, head);
            fpdrv = fopen(drv_imgfile[drive], "rb+");
            if (!fpdrv) {
                vlog(VM_DISKLOG, "PANIC! Could not access drive %d image.", drive);
                vm_exit(1);
            }
            fseek(fpdrv, (head + 1) * (track * drv_spt[drive] * drv_sectsize[drive]), SEEK_SET);
            fdata = malloc(drv_spt[drive] * drv_sectsize[drive]);
            memset(fdata, 0xAA, drv_spt[drive] * drv_sectsize[drive]);
            fwrite(fdata, drv_sectsize[drive], drv_spt[drive], fpdrv);
            free(fdata);
            fclose(fpdrv);
            cpu->regs.B.AH = FD_NO_ERROR;
            cpu->CF = 0;
        } else {
            cpu->regs.B.AH = FD_CHANGED_OR_REMOVED;
            cpu->CF = 1;
        }
        break;
#endif
    case 0x1308:
        drive = cpu->regs.B.DL;
        if(drive>=0x80) drive = drive - 0x80 + 2;
        if(drv_status[drive]!=0) {
            tracks = (drv_sectors[drive] / drv_spt[drive] / drv_heads[drive]) - 2;
            cpu->regs.B.AL = 0;
            cpu->regs.B.AH = FD_NO_ERROR;
            cpu->regs.B.BL = drv_type[drive];
            cpu->regs.B.BH = 0;
            cpu->regs.B.CH = tracks & 0xFF; /* Tracks */
            cpu->regs.B.CL = ((tracks >> 2) & 0xC0) | (drv_spt[drive] & 0x3F); /* Sectors per Track */
            cpu->regs.B.DH = drv_heads[drive] - 1; /* Sides */

            if (drive < 2)
                cpu->regs.B.DL = drv_status[0] + drv_status[1];
            else
                cpu->regs.B.DL = drv_status[2] + drv_status[3];

            vlog(VM_DISKLOG, "Reporting disk%d geo: %d tracks, %d spt, %d sides", drive, tracks, drv_spt[drive], drv_heads[drive]);

            /* WACKY SHIT about to take place. */
            cpu->regs.B.CH = tracks & 0xFF;
            cpu->regs.B.CL |= (tracks & 0x00030000) >> 10;

            /* ES:DI points to wacky Disk Base Table */
            #if 0
            if(drive<2) {
                cpu->ES = 0x820E; cpu->regs.W.DI = 0x0503;
            } else {
                cpu->ES = 0x820E; cpu->regs.W.DI = 0x04F8;
            }
            #endif

            cpu->setCF(0);
        } else {
            if(drive<2) cpu->regs.B.AH = FD_CHANGED_OR_REMOVED;
            else if(drive>1) {
                cpu->regs.B.AH = FD_FIXED_NOT_READY;
            }
            cpu->setCF(1);
        }

        break;
    case 0x1315:
        drive = cpu->regs.B.DL;
        if(drive >= 0x80) drive = drive - 0x80 + 2;
        if(drv_status[drive]!=0) {
            cpu->regs.B.AH = 0x01; /* Diskette, no change detection present. */
            if(drive>1) {
                cpu->regs.B.AH = 0x03; /* FIXED DISK :-) */
                /* if fixed disk, CX:DX = sectors */
                cpu->regs.W.DX = drv_sectors[drive];
                cpu->regs.W.CX = (drv_sectors[drive] << 16);
            }
            cpu->setCF(0);
        }
        else
        {
            /* Drive not present. */
            cpu->regs.B.AH = 0x00;
            cpu->setCF(1);
        }
        break;
    case 0x1318:
        drive = cpu->regs.B.DL;
        if (drive >= 0x80)
            drive = drive - 0x80 + 2;
        if (drv_status[drive]) {
            vlog(VM_DISKLOG, "Setting media type for drive %d:", drive);
            vlog(VM_DISKLOG, "%d sectors per track", cpu->regs.B.CL);
            vlog(VM_DISKLOG, "%d tracks", cpu->regs.B.CH);

            /* Wacky DBT. */
            cpu->ES = 0x820E; cpu->regs.W.DI = 0x0503;
            cpu->regs.B.AH = 0x00;
            cpu->setCF(0);
        } else {
            cpu->setCF(1);
            cpu->regs.B.AH = 0x80; // No media in drive
        }
        break;

    case 0x1600:
        cpu->regs.W.AX = kbd_getc();
        break;

    case 0x1700:
        // Interrupt 17, 00: Print character on LPT
        {
            char tmp[80];
            sprintf(tmp, "prn%d.txt", cpu->regs.W.DX);
            fpdrv = fopen(tmp, "a");
            fputc(cpu->regs.B.CL, fpdrv);
            fclose(fpdrv);
        }
        break;

    case 0x1A01:
        // Interrupt 1A, 01: Set RTC tick count
        vlog(VM_ALERT, "INT 1A,01: Attempt to set tick counter to %lu", (DWORD)(cpu->regs.W.CX<<16)|cpu->regs.W.DX);
        break;

    case 0x1A05:
        // Interrupt 1A, 05: Set BIOS date
        vlog(VM_ALERT, "INT 1A,05: Attempt to set BIOS date to %02X-%02X-%04X", cpu->regs.B.DH, cpu->regs.B.DL, cpu->regs.W.CX);
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
        drive = cpu->regs.B.DL;
        if (drive >= 0x80)
            drive = drive - 0x80 + 2;
        if (drv_status[drive] != 0) {
            cpu->regs.B.AL = 0x01;
            cpu->setCF(0);
        } else {
            cpu->regs.B.AL = 0x00;
            cpu->setCF(1);
        }
        break;

    default:
        vlog(VM_ALERT, "Unknown VM call %04X received!!", cpu->regs.W.AX);
        //vm_exit(0);
        break;
    }
}

void vga_scrollup(BYTE x1, BYTE y1, BYTE x2, BYTE y2, BYTE num, BYTE attr)
{
    BYTE *videoMemory = g_cpu->memoryPointer(0xB800, 0x0000);

    // TODO: Scroll graphics when in graphics mode (using text coordinates)

    //vlog(VM_VIDEOMSG, "vga_scrollup(%d, %d, %d, %d, %d)", x1, y1, x2, y2, num);
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
