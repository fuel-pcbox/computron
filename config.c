/* config.c
 *
 * I'm sorry.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vomit.h"

void vm_loadconf() {
	FILE *fconf, *ftmp;
	char curline[256], *curtok; char lfname[MAX_FN_LENGTH];
	#ifdef VM_DEBUG
		char tmp[256];
	#endif
	byte tb, lnum;
	word lseg, loff, ldrv, lspt, lhds, lsect, lsectsize, ltype;

	fconf = fopen("vm.conf", "r");
	if(fconf == NULL) {
		vm_out("config: Could not load vm.conf", VM_ERRORMSG);
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
				#ifdef VM_DEBUG
					sprintf(tmp,"Loading %s to %04X:%04X.\n",lfname,lseg,loff);
					vm_out(tmp, VM_INITMSG);
				#endif
                ftmp = fopen(lfname, "rb");
                if (ftmp == NULL) { vm_out("config: Error while processing \"loadfile\" line.\n", VM_ERRORMSG); vm_exit(1); }
                fread(mem_space+lseg*16+loff, 1, MAX_FILESIZE, ftmp);
                fclose(ftmp);
				continue;
            }
			else if(strcmp(curtok,"setbyte")==0) {
                lseg = strtol(strtok(NULL,": \t\n"), NULL, 16);
                loff = strtol(strtok(NULL," \t\n"), NULL, 16);
                curtok = strtok(NULL, " \t\n");
				#ifdef VM_DEBUG
					sprintf(tmp, "Entering at %04X:%04X: ", lseg, loff);
					vm_out(tmp, VM_INITMSG);
				#endif
                while(curtok) {
                    tb = strtol(curtok,NULL,16);
                    mem_setbyte(lseg, loff++, tb);
					#ifdef VM_DEBUG
						sprintf(tmp, "%02X ", tb);
						vm_out(tmp, 0);
					#endif
                    curtok = strtok(NULL, " \t\n");
                }
                curtok=curline;
                #ifdef VM_DEBUG
					vm_out("\n", 0);
				#endif
				continue;
            }
			else if(strcmp(curtok,"drive")==0) {	/* segfaults ahead! */
                ldrv = strtol(strtok(NULL, " \t\n"), NULL, 16);
                strcpy(lfname, strtok(NULL, " \t\n"));
                lspt = strtol(strtok(NULL, " \t\n"), NULL, 16);
                lhds = strtol(strtok(NULL, " \t\n"), NULL, 16);
                lsect = strtol(strtok(NULL, " \t\n"), NULL, 16);
                lsectsize = strtol(strtok(NULL, " \t\n"), NULL, 16);
				ltype = strtol(strtok(NULL, " \t\n"), NULL, 16);
				#ifdef VM_DEBUG
					sprintf(tmp, "drv%d: %s (%dspt, %dh, %ds (%db))\n", ldrv, lfname, lspt, lhds, lsect, lsectsize);
					vm_out(tmp, VM_INITMSG);
				#endif
                strcpy(drv_imgfile[ldrv], lfname);
                drv_spt[ldrv] = lspt;
                drv_heads[ldrv] = lhds;
                drv_sectors[ldrv] = lsect;
                drv_sectsize[ldrv] = lsectsize;
				drv_type[ldrv] = ltype;
                drv_status[ldrv] = 1;
				continue;
            }
			else if(strcmp(curtok,"setflags")==0) {
                curtok = strtok(NULL, " \t\n");
                cpu_setflags((word)strtol(curtok, NULL, 16));
				continue;
            }
			else if(strcmp(curtok,"cpu")==0) {
				curtok = strtok(NULL, " \t\n");
				cpu_type = (word)(strtol(curtok, NULL, 10));
				#ifdef VM_DEBUG
					sprintf(tmp, "Setting CPU type to %d\n", cpu_type);
					vm_out(tmp, VM_INITMSG);
				#endif
				continue;
			}
			else if(strcmp(curtok,"memory")==0) {
				curtok = strtok(NULL, " \t\n");
				mem_avail = (word)(strtol(curtok, NULL, 10));
				#ifdef VM_DEBUG
					sprintf(tmp, "Memory size: %d kilobytes.\n", mem_avail);
					vm_out(tmp, VM_INITMSG);
				#endif
			}
			else if(strcmp(curtok,"setstack")==0) {
                curtok = strtok(NULL, ": \t\n");
                SS = (word)strtol(curtok, NULL, 16);
                curtok = strtok(NULL, " \t\n");
                StackPointer = (word)strtol(curtok, NULL, 16);
				continue;
            }
			else if(strcmp(curtok,"entry")==0) {
                curtok = strtok(NULL, ": \t\n");
                CS = (word)strtol(curtok, NULL, 16);
                curtok = strtok(NULL, " \t\n");
                IP = (word)strtol(curtok, NULL, 16);
				continue;
            }
			else if(strcmp(curtok,"addint")==0) {
                lnum = (byte)strtol(strtok(NULL, " \t\n"), NULL, 16);
                lseg = (word)strtol(strtok(NULL, ": \t\n"), NULL, 16);
                loff = (word)strtol(strtok(NULL, " \t\n"), NULL, 16);
				#ifdef VM_DEBUG
					sprintf(tmp, "Installing interrupt %02X\n", lnum);
					vm_out(tmp, VM_INITMSG);
				#endif
		        cpu_addint(lnum, lseg, loff);
				continue;
            }
        }
    }
    fclose(fconf);
    return;
}

