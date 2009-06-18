/* VM Calls
 * Abstraction layer between fake BIOS and fake Hardware ;-)
 *
 * 031030:	Optimized single-character output by changing
 *			from printf to fputc ;-)
 *
 */

#include "vomit.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

void
vm_call8( word port, byte data )
{
	switch( port )
	{
		case 0xE0:
			vlog( VM_ALERT, "Interrupt %02X, function %04X requested", cpu.regs.B.AL, cpu.regs.W.AX );
			//dump_all();
			break;
		case 0xE6:
			vm_handleE6( cpu.regs.W.AX );
			break;
		case 0xE2:	/* (VM) int13h-style disk read */
			bios_readsectors();
			break;
		case 0xE3:
			bios_writesectors();
			break;
		case 0xE4:
			bios_verifysectors();
			break;
		case 0xE7:
			vga_scrollup( cpu.regs.B.CL, cpu.regs.B.CH, cpu.regs.B.DL, cpu.regs.B.DH, cpu.regs.B.AL, cpu.regs.B.BH );
			break;
		default:
			vlog( VM_ALERT, "vm_call8: Unhandled write, %02X -> %04X", data, port );
			vm_exit( 0 );
			break;
    }
}

void
vm_handleE6(word data)
{
	struct	tm *t;
	time_t	curtime;
	struct	timeval timv;
	byte	b1, b2, b3, b4;
	byte	*fdata;
	int		yr;
	dword	tick_count, tracks, track;
	word	head;
	byte	drive;
	FILE	*fpdrv;

	switch (data) {
	case 0x1601:
		if( kbd_hit() )
		{
			cpu.regs.W.AX = kbd_hit();
			cpu.ZF = 0;
		}
		else
		{
			cpu.regs.W.AX = 0x0000;
			cpu.ZF = 1;
		}
		break;
	case 0x1A00:	/* INT 1A, 00: Get RTC tick count */
		cpu.regs.B.AL = 1; /* midnight flag */
		curtime = time((time_t) NULL);
		t = localtime(&curtime);
		tick_count = ( (t->tm_hour*3600) + (t->tm_min*60) + (t->tm_sec) ) * 18.206; /* semi-disgusting */
		gettimeofday(&timv, NULL);
		tick_count += timv.tv_usec / 54926.9471602768;	/* precision is healthy */
		cpu.regs.W.CX = tick_count >> 16;
		cpu.regs.W.DX = tick_count & 0xFFFF;
		mem_setword( 0x0040, 0x006C, tick_count & 0xFFFF );
		mem_setword( 0x0040, 0x006D, tick_count >> 16 );
		break;
	case 0x1300:
		drive = cpu.regs.B.DL;
		if (drive >= 0x80)
			drive = drive - 0x80 + 2;
		if (drv_status[drive] != 0) {
			cpu.regs.B.AH = FD_NO_ERROR;
			cpu.CF = 0;
		} else {
			cpu.regs.B.AH = FD_CHANGED_OR_REMOVED;
			cpu.CF = 1;
		}
		mem_setbyte(0x0040, drive < 2 ? 0x0041 : 0x0072, cpu.regs.B.AH);
		break;
#if 0
	case 0x1305:
		drive = cpu.regs.B.DL;
		if (drive >= 0x80)
			drive = drive - 0x80 + 2;
		if (drv_status[drive] != 0) {
			track = (cpu.regs.B.CH | ((cpu.regs.B.CL & 0xC0) << 2)) + 1;
			head = cpu.regs.B.DH;
			vlog( VM_DISKLOG, "Drive %d: Formatting track %lu, head %d.", drive, track, head );
			fpdrv = fopen(drv_imgfile[drive], "rb+");
			if( !fpdrv )
			{
				vlog( VM_DISKLOG, "PANIC! Could not access drive %d image.", drive );
				vm_exit( 1 );
			}
			fseek(fpdrv, (head + 1) * (track * drv_spt[drive] * drv_sectsize[drive]), SEEK_SET);
			fdata = malloc(drv_spt[drive] * drv_sectsize[drive]);
			memset(fdata, 0xAA, drv_spt[drive] * drv_sectsize[drive]);
			fwrite(fdata, drv_sectsize[drive], drv_spt[drive], fpdrv);
			free(fdata);
			fclose(fpdrv);
			cpu.regs.B.AH = FD_NO_ERROR;
			cpu.CF = 0;
		} else {
			cpu.regs.B.AH = FD_CHANGED_OR_REMOVED;
			cpu.CF = 1;
		}
		break;
#endif
	case 0x1308:
		drive = cpu.regs.B.DL;
		if(drive>=0x80) drive = drive - 0x80 + 2;
		if(drv_status[drive]!=0) {
			tracks = (drv_sectors[drive] / drv_spt[drive] / drv_heads[drive]) - 2;
			cpu.regs.B.AL = 0;
			cpu.regs.B.AH = FD_NO_ERROR;
			cpu.regs.B.BL = drv_type[drive];
			cpu.regs.B.BH = 0;
			cpu.regs.B.CH = tracks & 0xFF; /* Tracks */
			cpu.regs.B.CL = ((tracks >> 2) & 0xC0) | (drv_spt[drive] & 0x3F); /* Sectors per Track */
			cpu.regs.B.DH = drv_heads[drive] - 1; /* Sides */

			if( drive < 2 )
				cpu.regs.B.DL = drv_status[0] + drv_status[1];
			else
				cpu.regs.B.DL = drv_status[2] + drv_status[3];

			vlog( VM_DISKLOG, "Reporting disk%d geo: %d tracks, %d spt, %d sides", drive, tracks, drv_spt[drive], drv_heads[drive] );

			/* WACKY SHIT about to take place. */
			cpu.regs.B.CH = tracks & 0xFF;
			cpu.regs.B.CL |= (tracks & 0x00030000) >> 10;

			/* ES:DI points to wacky Disk Base Table */
			#if 0
			if(drive<2) {
				cpu.ES = 0x820E; cpu.regs.W.DI = 0x0503;
			} else {
				cpu.ES = 0x820E; cpu.regs.W.DI = 0x04F8;
			}
			#endif

			cpu.CF = 0;
		} else {
			if(drive<2) cpu.regs.B.AH = FD_CHANGED_OR_REMOVED;
			else if(drive>1) {
				cpu.regs.B.AH = FD_FIXED_NOT_READY;
			}
			cpu.CF = 1;
		}

		break;
	case 0x1315:
		drive = cpu.regs.B.DL;
		if(drive >= 0x80) drive = drive - 0x80 + 2;
		if(drv_status[drive]!=0) {
			cpu.regs.B.AH = 0x01; /* Diskette, no change detection present. */
			if(drive>1) {
				cpu.regs.B.AH = 0x03; /* FIXED DISK :-) */
				/* if fixed disk, CX:DX = sectors */
				cpu.regs.W.DX = drv_sectors[drive];
				cpu.regs.W.CX = (drv_sectors[drive] << 16);
			}
			cpu.CF = 0;
		}
		else
		{
			/* Drive not present. */
			cpu.regs.B.AH = 0x00;
			cpu.CF = 1;
		}
		break;
	case 0x1318:
		drive = cpu.regs.B.DL;
		if(drive >= 0x80) drive = drive - 0x80 + 2;
		if( drv_status[drive] )
		{
			vlog( VM_DISKLOG, "Setting media type for drive %d:", drive );
			vlog( VM_DISKLOG, "%d sectors per track", cpu.regs.B.CL );
			vlog( VM_DISKLOG, "%d tracks", cpu.regs.B.CH );

			/* Wacky DBT. */
			cpu.ES = 0x820E; cpu.regs.W.DI = 0x0503;
			cpu.regs.B.AH = 0x00;
			cpu.CF = 0;
		} else {
			cpu.CF = 1;
			cpu.regs.B.AH = 0x80;			/* No media in drive */
		}
		break;

	case 0x1600:
		cpu.regs.W.AX = kbd_getc();
		break;

	case 0x1700:	/* INT 17, 00: Print character on LPT */
		{
			char tmp[80];
			sprintf( tmp, "prn%d.txt", cpu.regs.W.DX );
			fpdrv = fopen( tmp, "a" );
			fputc( cpu.regs.B.CL, fpdrv );
			fclose( fpdrv );
		}
		break;

	case 0x1A01:	/* INT 1A, 01: Set tick count */
		vlog( VM_ALERT, "INT 1A,01: Attempt to set tick counter to %lu", (dword)(cpu.regs.W.CX<<16)|cpu.regs.W.DX );
		break;

	case 0x1A02:    /* INT 1A, 02: Get BCD current time */
		curtime = time((time_t)NULL);
		t = localtime(&curtime);
		cpu.regs.B.CH = ((t->tm_hour/10)<<4) | (t->tm_hour-((t->tm_hour/10)*10));
		cpu.regs.B.CL = ((t->tm_min /10)<<4) | (t->tm_min -((t->tm_min /10)*10));
		cpu.regs.B.DH = ((t->tm_sec /10)<<4) | (t->tm_sec -((t->tm_sec /10)*10));
		/* BIOSes from 6/10/85 and on return Daylight Savings Time support status in DL. Fuck that. */
		cpu.CF = 0;
		break;

	case 0x1A04:    /* INT 1A, 04: Get BCD date */
		curtime = time((time_t)NULL);
		t = localtime(&curtime);
		yr = t->tm_year + 1900;
		b1 = yr / 1000;
		b2 = (yr - (b1*1000)) / 100;
		cpu.regs.B.CH = (b1<<4) | b2;
		b3 = (yr - (b1*1000) - (b2*100)) / 10;
		b4 = yr - (b1*1000) - (b2*100) - (b3*10);
		cpu.regs.B.CL = (b3<<4) | b4;
		b1 = (t->tm_mon+1) / 10;
		b2 = (t->tm_mon+1) - (b1*10);
		cpu.regs.B.DH = (b1<<4) | b2;
		b1 = t->tm_mday / 10;
		b2 = t->tm_mday - (b1*10);
		cpu.regs.B.DL = (b1<<4) | b2;
		cpu.CF = 0;
		break;

	case 0x1A05:
		vlog( VM_ALERT, "INT 1A,05: Attempt to set BIOS date to %02X-%02X-%04X", cpu.regs.B.DH, cpu.regs.B.DL, cpu.regs.W.CX );
		break;

	case 0x0000:
		vlog( VM_KILLMSG, "VM call 0 received -- shutting down" );
		vm_exit( 0 );
		break;

	case 0x0002:
		cpu.regs.W.AX = cpu.type;
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
		drive = cpu.regs.B.DL;
		if (drive >= 0x80)
			drive = drive - 0x80 + 2;
		if (drv_status[drive] != 0) {
			cpu.regs.B.AL = 0x01;
			cpu.CF = 0;
		} else {
			cpu.regs.B.AL = 0x00;
			cpu.CF = 1;
		}
		break;

	default:
		vlog( VM_ALERT, "Unknown VM call %04X received!!", cpu.regs.W.AX );
		//vm_exit( 0 );
		break;
	}
}
