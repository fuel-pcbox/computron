/* VM Calls
 * Abstraction layer between fake BIOS and fake Hardware ;-)
 *
 * 031030:	Optimized single-character output by changing
 *			from printf to fputc ;-)
 *
 */

#include "vomit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#ifdef VM_DEBUG
	char	tmp[80];
#endif

void
vm_call8 (word port, byte data)
{
	(void) data;

	switch(port) {
	case 0x60:
	case 0x61:
		break;
	case 0x62:	/* (VM) int13h-style disk read */
		bios_readsectors();
		break;
	case 0x63:
		bios_writesectors();
		break;
	case 0x64:
		bios_verifysectors();
		break;
	case 0x67:
		vga_scrollup(*treg8[REG_CL], *treg8[REG_CH], *treg8[REG_DL], *treg8[REG_DH], *treg8[REG_AL], *treg8[REG_BH]);
		break;
	case 0x69:
		break;
	default:
#ifdef VM_DEBUG
		if(callpeek) printf("vm_call8:  %04X -> %04X\n", data, port);
#endif
		break;
    }
    return;
}

void
vm_call16 (word port, word data)
{
	switch(port) {
	case 0x66:	/* (VM) Handler of all sorts of crap. */
		vm_handle66(data);
		break;
	case 0x00:
#ifdef VM_DEBUG
		sprintf(tmp, "Interrupt %02X, function %02X requested.\n", *treg8[REG_AL], *treg8[REG_AH]);
		vm_out(tmp, VM_ALERT);
#endif
		break;
	case 0x61:
		break;
	default:
#ifdef VM_DEBUG
		if (callpeek) {
			sprintf(tmp, "vm_call16: %04X -> %04X\n", data, port);
			vm_out(tmp, VM_LOGMSG);
		}
#endif
		break;
	}
	return;
}

void
vm_handle66 (word data)
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
		if ( g_break_pressed ) {
			ZF = 0;
			AX = 0x0000;
			break;
		}
		if (kbd_hit() != 0) {
			AX = kbd_hit();
			ZF = 0;
		} else {
			ZF = 1;
		}
		break;
	case 0x1A00:	/* INT 1A, 00: Get RTC tick count */
		*treg8[REG_AL] = 0; /* midnight flag */
		curtime = time((time_t) NULL);
		t = localtime(&curtime);
		tick_count = ( (t->tm_hour*3600) + (t->tm_min*60) + (t->tm_sec) ) * 18.206; /* semi-disgusting */
		gettimeofday(&timv, NULL);
		tick_count += timv.tv_usec / 54926.9471602768;	/* precision is healthy */
		CX = tick_count >> 16;
		DX = tick_count & 0xFFFF;
		break;
	case 0x1005:
#ifdef VM_DEBUG
		sprintf(tmp, "Request for video page %d\n", *treg8[REG_AL]);
		vm_out(tmp, VM_VIDEOMSG);
#endif
		break;
	case 0x1300:
		drive = *treg8[REG_DL];
		if (drive >= 0x80)
			drive = drive - 0x80 + 2;
		if (drv_status[drive] != 0) {
			*treg8[REG_AH] = FD_NO_ERROR;
			CF = 0;
		} else {
			*treg8[REG_AH] = FD_CHANGED_OR_REMOVED;
			CF = 1;
		}
		mem_setbyte(0x0040, 0x0041, *treg8[REG_AH]);
		break;
	case 0x1305:
		drive = *treg8[REG_DL];
		if (drive >= 0x80)
			drive = drive - 0x80 + 2;
		if (drv_status[drive] != 0) {
			track = (*treg8[REG_CH] | ((*treg8[REG_CL] & 0xC0) << 2)) + 1;
			head = *treg8[REG_DH];
#ifdef VM_DEBUG
			sprintf(tmp, "Drive %d: Formatting track %lu, head %d.\n", drive, track, head);
			vm_out(tmp, VM_DISKLOG);
#endif
			fpdrv = fopen(drv_imgfile[drive], "rb+");
			if (fpdrv == NULL) {
				printf("\nPANIC! Could not access drive %d image.\n", drive);
				vm_exit(1);
			}
			fseek(fpdrv, (head + 1) * (track * drv_spt[drive] * drv_sectsize[drive]), SEEK_SET);
			fdata = malloc(drv_spt[drive] * drv_sectsize[drive]);
			memset(fdata, 0xAA, drv_spt[drive] * drv_sectsize[drive]);
			fwrite(fdata, drv_sectsize[drive], drv_spt[drive], fpdrv);
			free(fdata);
			fclose(fpdrv);
			*treg8[REG_AH] = FD_NO_ERROR;
			CF = 0;
		} else {
			*treg8[REG_AH] = FD_CHANGED_OR_REMOVED;
			CF = 1;
		}
		break;
	case 0x1308:
		drive = *treg8[REG_DL];
		if(drive>=0x80) drive = drive - 0x80 + 2;
		if(drv_status[drive]!=0) {
			tracks = (drv_sectors[drive] / drv_spt[drive] / drv_heads[drive]) - 1;
			*treg8[REG_AH] = FD_NO_ERROR;
			*treg8[REG_BL] = drv_type[drive];
			CX = (word)tracks; /* Tracks */
			*treg8[REG_CL] = drv_spt[drive]; /* Sectors per Track */
			*treg8[REG_DH] = drv_heads[drive] - 1; /* Sides */
			*treg8[REG_DL] = 2;    /* 1 drive */

			/* WACKY SHIT about to take place. */
			*treg8[REG_CH] = tracks & 0xFF;
			*treg8[REG_CL] = *treg8[REG_CL] | ((tracks & 0x00030000) >> 10);

			/* ES:DI points to wacky Disk Base Table */
			if(drive<2) {
				ES = 0x820E; DI = 0x0503;
			} else {
				ES = 0x820E; DI = 0x04F8;
			}

			CF = 0;
		} else {
			if(drive<2) *treg8[REG_AH] = FD_CHANGED_OR_REMOVED;
			else if(drive>1) {
				*treg8[REG_AH] = FD_FIXED_NOT_READY;
			}
			CF = 1;
		}

		break;
	case 0x1315:
		drive = *treg8[REG_DL];
		if(drive >= 0x80) drive = drive - 0x80 + 2;
		if(drv_status[drive]!=0) {
			*treg8[REG_AH] = 0x01; /* Diskette, no change detection present. */
			if(drive>1) {
				*treg8[REG_AH] = 0x03; /* FIXED DISK :-) */
				/* if fixed disk, CX:DX = sectors */
				DX = drv_sectors[drive];
				CX = (drv_sectors[drive] << 16);
			}
			CF = 0;
		}
		else { CF = 1; }
		break;
	case 0x1318:
		drive = *treg8[REG_DL];
		if(drive >= 0x80) drive = drive - 0x80 + 2;
		if(drv_status[drive]!=0) {
			#ifdef VM_DEBUG
				sprintf(tmp, "Setting media type for drive %d:\n", drive);
				vm_out(tmp, VM_DISKLOG);
				sprintf(tmp, "%d sectors per track\n", *treg8[REG_CL] & 63 );
				vm_out(tmp, VM_DISKLOG);
				sprintf(tmp, "%d tracks\n", ((*treg8[REG_CH]) | (*treg8[REG_CL]&0xC0)<<2)+1);
				vm_out(tmp, VM_DISKLOG);
			#endif
			/*if(( (*treg8[REG_CL]&63) != drv_spt[drive] ) || (((*treg8[REG_CH])|(*treg8[REG_CL]<<2))+1) != (drv_sectors[drive]/drv_spt[drive]/drv_heads[drive]) ) {
				printf("GEOMETRY MISMATCH!\n");
				printf("spt: %d vs. %d\n", *treg8[REG_CL]&63, drv_spt[drive]);
				printf("trk: %d vs. %d\n", (((*treg8[REG_CH])|(*treg8[REG_CL]&0xC0)<<2)+1), drv_sectors[drive]/drv_spt[drive]/drv_heads[drive]);
				*treg8[REG_AH] = 0x0C;		// unsupported or unknown
				CF = 1;
			} else {*/
				*treg8[REG_AH] = 0x00;
				CF = 0;
			/*}*/
		} else {
			CF = 1;
			*treg8[REG_AH] = 0x80;			/* No media in drive */
		}
		break;
	case 0x1600:
		AX = kbd_getc();
		break;

	case 0x1700:	/* INT 17, 00: Print character on LPT */
		#ifdef VM_DEBUG
			sprintf( tmp, "prn%d.txt", DX );
			fpdrv = fopen( tmp, "a" );
			fputc( *treg8[REG_CL], fpdrv );
			fclose( fpdrv );
		#endif
		break;

	case 0x1A01:	/* INT 1A, 01: Set tick count */
		#ifdef VM_DEBUG
			sprintf(tmp, "int1a: Attempt to set tick counter to %lu\n", (dword)(CX<<16)|DX);
			vm_out(tmp, VM_LOGMSG);
		#endif
		break;

	case 0x1A02:    /* INT 1A, 02: Get BCD current time */
		curtime = time((time_t)NULL);
		t = localtime(&curtime);
		*treg8[REG_CH] = ((t->tm_hour/10)<<4) | (t->tm_hour-((t->tm_hour/10)*10));
		*treg8[REG_CL] = ((t->tm_min /10)<<4) | (t->tm_min -((t->tm_min /10)*10));
		*treg8[REG_DH] = ((t->tm_sec /10)<<4) | (t->tm_sec -((t->tm_sec /10)*10));
		/* BIOSes from 6/10/85 and on return Daylight Savings Time support status in DL. Fuck that. */
		CF = 0;
		break;

	case 0x1A04:    /* INT 1A, 04: Get BCD date */
		curtime = time((time_t)NULL);
		t = localtime(&curtime);
		yr = t->tm_year + 1900;
		b1 = yr / 1000;
		b2 = (yr - (b1*1000)) / 100;
		*treg8[REG_CH] = (b1<<4) | b2;
		b3 = (yr - (b1*1000) - (b2*100)) / 10;
		b4 = yr - (b1*1000) - (b2*100) - (b3*10);
		*treg8[REG_CL] = (b3<<4) | b4;
		b1 = (t->tm_mon+1) / 10;
		b2 = (t->tm_mon+1) - (b1*10);
		*treg8[REG_DH] = (b1<<4) | b2;
		b1 = t->tm_mday / 10;
		b2 = t->tm_mday - (b1*10);
		*treg8[REG_DL] = (b1<<4) | b2;
		CF = 0;
		break;

#ifdef VM_DEBUG
	case 0x1A05:
		sprintf(tmp, "int1a05: Attempt to set BIOS date to %02X-%02X-%04X\n", *treg8[REG_DH], *treg8[REG_DL], CX);
		vm_out(tmp, VM_LOGMSG);
		break;

	case 0x0300:
		ui_kill();
		printf( "\nBreak triggered at %04X:%04X\n", BCS, BIP );
		vm_debug();
		if ( !g_debug_step )
			ui_show();
		break;
#endif

	case 0x0000:
		printf("vomit: VM call 0000 received. Shutting down.\n");
		vm_exit( 0 );
		break;
	case 0x0001:
		AX = mem_avail;
		*treg8[REG_BL] = cpu_type;
		break;
	}
	return;
}

