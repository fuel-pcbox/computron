/* config.c
 *
 * I'm sorry.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vomit.h"
#include "debug.h"

void vm_loadconf() {
	FILE *fconf, *ftmp;
	char curline[256], *curtok; char lfname[MAX_FN_LENGTH];
	byte tb, lnum;
	word lseg, loff, ldrv, lspt, lhds, lsect, lsectsize, ltype;

	fconf = fopen("vm.conf", "r");
	if(fconf == NULL) {
		vlog( VM_CONFIGMSG, "Couldn't load vm.conf" );
		vm_exit(1);
		return;
	}

    while(!feof(fconf)) {
        fgets(curline, 256, fconf);
        if((curline[0]!='#')&&(strlen(curline)>1)) {
            curtok = strtok(curline, " \t\n");
            if(strcmp(curtok,"loadfile")==0) {
                strcpy(lfname, strtok(NULL, " \t\n"));
                lseg = (word)strtol(strtok(NULL, ": \t\n"),NULL,16);
                loff = (word)strtol(strtok(NULL, " \t\n"),NULL,16);

				vlog( VM_INITMSG, "Loading %s to %04X:%04X", lfname, lseg, loff );

                ftmp = fopen(lfname, "rb");
                if (ftmp == NULL)
				{
					vlog( VM_CONFIGMSG, "Error while processing \"loadfile\" line." );
					vm_exit(1);
				}
                fread(mem_space+lseg*16+loff, 1, MAX_FILESIZE, ftmp);
                fclose(ftmp);
            }
			else if(strcmp(curtok,"setbyte")==0) {
                lseg = strtol(strtok(NULL,": \t\n"), NULL, 16);
                loff = strtol(strtok(NULL," \t\n"), NULL, 16);
                curtok = strtok(NULL, " \t\n");

				vlog( VM_INITMSG, "Entering at %04X:%04X", lseg, loff );

                while(curtok) {
                    tb = strtol(curtok,NULL,16);
					vlog( VM_INITMSG, "  [%04X] = %02X", loff, tb );
                    mem_setbyte(lseg, loff, tb);
					++loff;
                    curtok = strtok(NULL, " \t\n");
                }
                curtok=curline;
            }
			else if(strcmp(curtok,"drive")==0) {	/* segfaults ahead! */
                ldrv = strtol(strtok(NULL, " \t\n"), NULL, 16);
                strcpy(lfname, strtok(NULL, " \t\n"));
                lspt = strtol(strtok(NULL, " \t\n"), NULL, 16);
                lhds = strtol(strtok(NULL, " \t\n"), NULL, 16);
                lsect = strtol(strtok(NULL, " \t\n"), NULL, 16);
                lsectsize = strtol(strtok(NULL, " \t\n"), NULL, 16);
				ltype = strtol(strtok(NULL, " \t\n"), NULL, 16);

				vlog( VM_INITMSG, "Drive %d: %s (%dspt, %dh, %ds (%db))", ldrv, lfname, lspt, lhds, lsect, lsectsize );

                strcpy(drv_imgfile[ldrv], lfname);
                drv_spt[ldrv] = lspt;
                drv_heads[ldrv] = lhds;
                drv_sectors[ldrv] = lsect;
                drv_sectsize[ldrv] = lsectsize;
				drv_type[ldrv] = ltype;
                drv_status[ldrv] = 1;
            }
			else if(strcmp(curtok,"setflags")==0) {
                curtok = strtok(NULL, " \t\n");
                cpu_setflags((word)strtol(curtok, NULL, 16));
            }
			else if(strcmp(curtok,"cpu")==0) {
				curtok = strtok(NULL, " \t\n");
				cpu_type = (word)(strtol(curtok, NULL, 10));
				vlog( VM_INITMSG, "Setting CPU type to %d", cpu_type );
			}
			else if(strcmp(curtok,"memory")==0) {
				curtok = strtok(NULL, " \t\n");
				mem_avail = (word)(strtol(curtok, NULL, 10));
				vlog( VM_INITMSG, "Memory size: %d kilobytes", mem_avail );
			}
			else if(strcmp(curtok,"entry")==0) {
                curtok = strtok(NULL, ": \t\n");
                cpu.CS = (word)strtol(curtok, NULL, 16);
                curtok = strtok(NULL, " \t\n");
                cpu.IP = (word)strtol(curtok, NULL, 16);
				cpu_jump( cpu.CS, cpu.IP );
            }
			else if(strcmp(curtok,"addint")==0) {
                lnum = (byte)strtol(strtok(NULL, " \t\n"), NULL, 16);
                lseg = (word)strtol(strtok(NULL, ": \t\n"), NULL, 16);
                loff = (word)strtol(strtok(NULL, " \t\n"), NULL, 16);
				vlog( VM_INITMSG, "Software interrupt %02X at %04X:%04X", lnum, lseg, loff );
		        cpu_addint(lnum, lseg, loff);
            }
        }
    }
    fclose(fconf);
    return;
}

