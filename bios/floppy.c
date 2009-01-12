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
#include <stdio.h>
#include <stdlib.h>

char drv_imgfile[4][MAX_FN_LENGTH];
char drv_title[4][MAX_DRV_TITLESIZE+1];
byte drv_status[4], drv_type[4];
dword drv_spt[4], drv_heads[4], drv_sectors[4], drv_sectsize[4];

word
chs2lba( byte drive, word cyl, byte head, word sector )
{
	return (sector - 1) +
	       (head * drv_spt[drive]) +
	       (cyl * drv_spt[drive] * drv_heads[drive]);
}

byte floppy_read(byte drive, word cylinder, word head, word sector, word count, word segment, word offset) {
	FILE *fpdrv;

	word lba = chs2lba(drive, cylinder, head, sector);
	byte *fderr = (byte *)mem_space + 0x441;		/* fd status in bda */

	if(!drv_status[drive]) {
		if(disklog)
		{
			vlog( VM_DISKLOG, "Drive %02X not ready.", drive );
		}
		if(drive<2) *fderr = FD_CHANGED_OR_REMOVED;
		else if(drive>1) *fderr = FD_TIMEOUT;
		return *fderr;
	}
	if((sector>drv_spt[drive])||(head>=drv_heads[drive])) {
		if( disklog )
		{
			vlog( VM_DISKLOG, "%04X:%04X Drive %d read request out of geometrical bounds (%d/%d/%d)", cpu.CS, cpu.IP, drive, cylinder, head, sector );
		}
		*fderr = FD_TIMEOUT;
		return *fderr;
	}
	if(lba>drv_sectors[drive]) {
		if( disklog )
		{
			vlog( VM_DISKLOG, "Drive %d bogus sector request (LBA %d).\n", drive, lba );
		}
		*fderr = FD_TIMEOUT;
		return *fderr;
	}
	if( disklog )
	{
		vlog( VM_DISKLOG, "Drive %d reading %d sectors at %d/%d/%d (LBA %d) to %04X:%04X", drive, count, cylinder, head, sector, lba, segment, offset );
	}
	fpdrv = fopen(drv_imgfile[drive], "rb");
	if( !fpdrv )
	{
		vlog( VM_DISKLOG, "PANIC: Could not access drive %d image!", drive );
		vm_exit( 1 );
	}
	fflush(fpdrv);
	fseek(fpdrv, lba*drv_sectsize[drive], SEEK_SET);
	fread(mem_space+(segment*16)+(offset), drv_sectsize[drive], count, fpdrv);
	fclose(fpdrv);
	*fderr = FD_NO_ERROR;
	return FD_NO_ERROR;
}

byte floppy_write(byte drive, word cylinder, word head, word sector, word count, word segment, word offset) {
    FILE *fpdrv;
    word lba = chs2lba(drive, cylinder, head, sector);
    byte *fderr = (byte *)mem_space + 0x441;        /* fd status in bda */

    if(!drv_status[drive]) {
		if( disklog )
		{
			vlog( VM_DISKLOG, "Drive %02X not ready", drive);
		}
        if(drive<2) *fderr = FD_CHANGED_OR_REMOVED;
        else if(drive>1) *fderr = FD_TIMEOUT;
        return *fderr;
    }
    if((sector>drv_spt[drive])||(head>=drv_heads[drive])) {
		if( disklog )
		{
			vlog( VM_DISKLOG, "Drive %d write request out of geometrical bounds (%d/%d/%d)", drive, cylinder, head, sector);
		}
        *fderr = FD_TIMEOUT;
        return *fderr;
    }
    if(lba>drv_sectors[drive]) {
		if( disklog )
		{
			vlog( VM_DISKLOG, "Drive %d bogus sector request (LBA %d)", drive, lba );
		}
        *fderr = FD_TIMEOUT;
		return *fderr;
    }
	if( disklog )
	{
		vlog( VM_DISKLOG, "Drive %d writing %d sectors at %d/%d/%d (LBA %d) from %04X:%04X", drive, count, cylinder, head, sector, lba, segment, offset );
	}
    fpdrv = fopen(drv_imgfile[drive], "rb+");
    if( !fpdrv )
	{
        vlog( VM_DISKLOG, "PANIC: Could not access drive %d image!", drive );
        vm_exit( 1 );
    }
    fseek(fpdrv, lba*drv_sectsize[drive], SEEK_SET);
    fwrite(mem_space+(segment<<4)+offset, drv_sectsize[drive], count, fpdrv);
	fflush(fpdrv); /* best make sure! */
    fclose(fpdrv);
	*fderr = FD_NO_ERROR;
    return FD_NO_ERROR;
}

void
bios_readsectors()
{
	word cyl = cpu.regs.B.CH;
    word sect = cpu.regs.B.CL;
	byte drive = cpu.regs.B.DL;

	if(drive>=0x80) drive = drive - 0x80 + 2;

	cpu.regs.B.AH = floppy_read( drive, cyl, cpu.regs.B.DH, sect, cpu.regs.B.AL, cpu.ES, cpu.regs.W.BX );

	if( cpu.regs.B.AH == 0x00 )
		cpu.CF = 0;
	else {
		cpu.CF = 1;
		cpu.regs.B.AL = 0x00;
	}
}

void
bios_writesectors()
{
    word cyl = cpu.regs.B.CH;
    word sect = cpu.regs.B.CL;
	byte drive = cpu.regs.B.DL;

	if(drive>=0x80) drive = drive - 0x80 + 2;

	cpu.regs.B.AH = floppy_write( drive, cyl, cpu.regs.B.DH, sect, cpu.regs.B.AL, cpu.ES, cpu.regs.W.BX );

	if( cpu.regs.B.AH ==0x00 )
		cpu.CF = 0;
	else {
		cpu.CF = 1;
		cpu.regs.B.AL = 0x00;
	}
}


byte
floppy_verify( byte drive, word cylinder, word head, word sector, word count, word segment, word offset )
{
	FILE *fpdrv;
	byte *dummy;
	word lba = chs2lba(drive, cylinder, head, sector);
	word veri;
	byte *fderr = (byte *)mem_space + 0x441;		/* fd status in bda */
	(void) segment;
	(void) offset;

	if(!drv_status[drive]) {
		if( disklog )
		{
			vlog( VM_DISKLOG, "Drive %02X not ready", drive );
		}
		if(drive<2) *fderr = FD_CHANGED_OR_REMOVED;
		else if(drive>1) *fderr = FD_TIMEOUT;
		return *fderr;
	}
	if((sector>drv_spt[drive])||(head>=drv_heads[drive])) {
		if( disklog )
		{
			vlog( VM_DISKLOG, "Drive %d verify request out of geometrical bounds", drive );
		}
		*fderr = FD_BAD_COMMAND;
		return *fderr;
	}
	if(lba>drv_sectors[drive]) {
		if( disklog )
		{
			vlog( VM_DISKLOG, "Drive %d bogus sector request (LBA %d)", drive, lba );
		}
		*fderr = FD_BAD_COMMAND;
		return *fderr;
	}
	if( disklog )
	{
		vlog( VM_DISKLOG, "Drive %d verifying %d sectors at %d/%d/%d (LBA %d)", drive, count, cylinder, head, sector, lba );
	}
	fpdrv = fopen(drv_imgfile[drive], "rb");
	if( !fpdrv )
	{
		vlog( VM_DISKLOG, "PANIC: Could not access drive %d image!", drive );
		vm_exit( 1 );
	}
	fflush(fpdrv);
	fseek(fpdrv, lba * drv_sectsize[drive], SEEK_SET);

	dummy = malloc(count * drv_sectsize[drive] * sizeof(byte));
		veri = fread(dummy, drv_sectsize[drive], count, fpdrv);
		/* XXX: Eh? */
		if(veri!=count) printf("EYY!\n");
	free(dummy);
	fclose(fpdrv);
	*fderr = FD_NO_ERROR;
	return FD_NO_ERROR;
}

void
bios_verifysectors()
{
	word cyl = cpu.regs.B.CH;
	word sect = cpu.regs.B.CL;
	byte drive = cpu.regs.B.DL;

	if(drive>=0x80) drive = drive - 0x80 + 2;

	cpu.regs.B.AH = floppy_verify( drive, cyl, cpu.regs.B.DH, sect, cpu.regs.B.AL, cpu.ES, cpu.regs.W.BX );

	if( cpu.regs.B.AH == 0x00 )
		cpu.CF = 0;
	else {
		cpu.CF = 1;
		cpu.regs.B.AL = 0x00;
	}
}
