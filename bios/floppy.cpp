/* bios/int13.c
 * ROM-BIOS Disk Operations
 *
 * YES, I know this whole thing is very ugly.
 * But I want to get it running, damn it :/
 *
 * 031028: All errorcodes were returned in AL.
 * Supposed to be AH. Probably a healthy fix :-)
 *
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

WORD chs2lba(BYTE drive, WORD cyl, BYTE head, WORD sector)
{
    return (sector - 1) +
           (head * drv_spt[drive]) +
           (cyl * drv_spt[drive] * drv_heads[drive]);
}

BYTE floppy_read(BYTE drive, WORD cylinder, WORD head, WORD sector, WORD count, WORD segment, WORD offset)
{
    WORD lba = chs2lba(drive, cylinder, head, sector);
    BYTE* fderr = g_cpu->memoryPointer(0x0000, 0x0441); // fd status in bda

    if (!drv_status[drive]) {
        if (disklog)
            vlog(VM_DISKLOG, "Drive %02X not ready.", drive);
        if (drive < 2)
            *fderr = FD_CHANGED_OR_REMOVED;
        else if (drive > 1)
            *fderr = FD_TIMEOUT;
        return *fderr;
    }
    if ((sector > drv_spt[drive])||(head >= drv_heads[drive])) {
        if (disklog)
            vlog(VM_DISKLOG, "%04X:%04X Drive %d read request out of geometrical bounds (%d/%d/%d)", g_cpu->getCS(), g_cpu->getIP(), drive, cylinder, head, sector);
        *fderr = FD_TIMEOUT;
        return *fderr;
    }
    if (lba > drv_sectors[drive]) {
        if (disklog)
            vlog(VM_DISKLOG, "Drive %d bogus sector request (LBA %d)", drive, lba);
        *fderr = FD_TIMEOUT;
        return *fderr;
    }
    if (disklog)
        vlog(VM_DISKLOG, "Drive %d reading %d sectors at %d/%d/%d (LBA %d) to %04X:%04X [offset=0x%x]", drive, count, cylinder, head, sector, lba, segment, offset, lba*drv_sectsize[drive]);
    FILE* fpdrv = fopen(drv_imgfile[drive], "rb");
    if (!fpdrv) {
        vlog(VM_DISKLOG, "PANIC: Could not access drive %d image!", drive);
        vm_exit(1);
    }
    fflush(fpdrv);
    fseek(fpdrv, lba*drv_sectsize[drive], SEEK_SET);
    ssize_t bytesRead = fread(g_cpu->memory+(segment*16)+(offset), drv_sectsize[drive], count, fpdrv);
#ifdef VOMIT_DETECT_UNINITIALIZED_ACCESS
    for (DWORD address = segment * 16 + offset; address < segment * 16 + offset + BYTEsRead; ++address)
        g_cpu->markDirty(address);
#endif
    fclose(fpdrv);
    *fderr = FD_NO_ERROR;
    return FD_NO_ERROR;
}

BYTE floppy_write(BYTE drive, WORD cylinder, WORD head, WORD sector, WORD count, WORD segment, WORD offset)
{
    WORD lba = chs2lba(drive, cylinder, head, sector);
    BYTE* fderr = g_cpu->memoryPointer(0x0000, 0x0441); // fd status in bda

    if (!drv_status[drive]) {
        if (disklog)
            vlog(VM_DISKLOG, "Drive %02X not ready", drive);
        if (drive < 2)
            *fderr = FD_CHANGED_OR_REMOVED;
        else if (drive > 1)
            *fderr = FD_TIMEOUT;
        return *fderr;
    }
    if ((sector > drv_spt[drive])||(head>=drv_heads[drive])) {
        if (disklog)
            vlog(VM_DISKLOG, "Drive %d write request out of geometrical bounds (%d/%d/%d)", drive, cylinder, head, sector);
        *fderr = FD_TIMEOUT;
        return *fderr;
    }
    if (lba > drv_sectors[drive]) {
        if (disklog)
            vlog(VM_DISKLOG, "Drive %d bogus sector request (LBA %d)", drive, lba);
        *fderr = FD_TIMEOUT;
        return *fderr;
    }
    if (disklog)
        vlog(VM_DISKLOG, "Drive %d writing %d sectors at %d/%d/%d (LBA %d) from %04X:%04X", drive, count, cylinder, head, sector, lba, segment, offset);
    FILE* fpdrv = fopen(drv_imgfile[drive], "rb+");
    if (!fpdrv) {
        vlog(VM_DISKLOG, "PANIC: Could not access drive %d image!", drive);
        vm_exit(1);
    }
    fseek(fpdrv, lba*drv_sectsize[drive], SEEK_SET);
    fwrite(g_cpu->memory+(segment<<4)+offset, drv_sectsize[drive], count, fpdrv);
    fflush(fpdrv);
    fclose(fpdrv);
    *fderr = FD_NO_ERROR;
    return FD_NO_ERROR;
}

void bios_readsectors()
{
    WORD cyl = g_cpu->regs.B.CH;
    WORD sect = g_cpu->regs.B.CL;
    BYTE drive = g_cpu->regs.B.DL;

    if (drive >= 0x80)
        drive = drive - 0x80 + 2;

    g_cpu->regs.B.AH = floppy_read(drive, cyl, g_cpu->regs.B.DH, sect, g_cpu->regs.B.AL, g_cpu->ES, g_cpu->regs.W.BX);

    if (g_cpu->regs.B.AH == 0x00)
        g_cpu->setCF(0);
    else {
        g_cpu->setCF(1);
        g_cpu->regs.B.AL = 0x00;
    }
}

void bios_writesectors()
{
    WORD cyl = g_cpu->regs.B.CH;
    WORD sect = g_cpu->regs.B.CL;
    BYTE drive = g_cpu->regs.B.DL;

    if (drive >= 0x80)
        drive = drive - 0x80 + 2;

    g_cpu->regs.B.AH = floppy_write(drive, cyl, g_cpu->regs.B.DH, sect, g_cpu->regs.B.AL, g_cpu->ES, g_cpu->regs.W.BX);

    if (g_cpu->regs.B.AH == 0x00)
        g_cpu->setCF(0);
    else {
        g_cpu->setCF(1);
        g_cpu->regs.B.AL = 0x00;
    }
}

BYTE floppy_verify(BYTE drive, WORD cylinder, WORD head, WORD sector, WORD count, WORD segment, WORD offset)
{
    WORD lba = chs2lba(drive, cylinder, head, sector);
    BYTE* fderr = g_cpu->memoryPointer(0x0000, 0x0441); // fd status in bda

    if (!drv_status[drive]) {
        if (disklog)
            vlog(VM_DISKLOG, "Drive %02X not ready", drive);
        if (drive < 2)
            *fderr = FD_CHANGED_OR_REMOVED;
        else if (drive > 1)
            *fderr = FD_TIMEOUT;
        return *fderr;
    }
    if ((sector > drv_spt[drive]) || (head >= drv_heads[drive])) {
        if (disklog)
            vlog(VM_DISKLOG, "Drive %d verify request out of geometrical bounds", drive);
        *fderr = FD_BAD_COMMAND;
        return *fderr;
    }
    if (lba > drv_sectors[drive]) {
        if (disklog)
            vlog(VM_DISKLOG, "Drive %d bogus sector request (LBA %d)", drive, lba);
        *fderr = FD_BAD_COMMAND;
        return *fderr;
    }
    if (disklog)
        vlog(VM_DISKLOG, "Drive %d verifying %d sectors at %d/%d/%d (LBA %d)", drive, count, cylinder, head, sector, lba);
    FILE* fpdrv = fopen(drv_imgfile[drive], "rb");
    if (!fpdrv) {
        vlog(VM_DISKLOG, "PANIC: Could not access drive %d image!", drive);
        vm_exit(1);
    }
    fflush(fpdrv);
    fseek(fpdrv, lba * drv_sectsize[drive], SEEK_SET);

    BYTE* dummy = new BYTE[(count * drv_sectsize[drive])];
    WORD veri = fread(dummy, drv_sectsize[drive], count, fpdrv);
    if (veri != count)
        vlog(VM_ALERT, "veri != count, something went wrong");
    delete [] dummy;
    fclose(fpdrv);
    *fderr = FD_NO_ERROR;
    return FD_NO_ERROR;
}

void bios_verifysectors()
{
    WORD cyl = g_cpu->regs.B.CH;
    WORD sect = g_cpu->regs.B.CL;
    BYTE drive = g_cpu->regs.B.DL;

    if (drive >= 0x80)
        drive = drive - 0x80 + 2;

    g_cpu->regs.B.AH = floppy_verify(drive, cyl, g_cpu->regs.B.DH, sect, g_cpu->regs.B.AL, g_cpu->ES, g_cpu->regs.W.BX);

    if (g_cpu->regs.B.AH == 0x00)
        g_cpu->setCF(0);
    else {
        g_cpu->setCF(1);
        g_cpu->regs.B.AL = 0x00;
    }
}
