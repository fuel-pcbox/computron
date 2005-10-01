/* io.c
 * I/O instructions and functions
 *
 */

#include "vomit.h"
#include <stdio.h>

#ifdef VM_DEBUG
	char tmp[40];
#endif

tintab vm_ioh_in[0xFFFF];		/* make me ffff */
touttab vm_ioh_out[0xFFFF];		/* me too.		*/

void _OUT_imm8_AL() { cpu_out(cpu_pfq_getbyte(), *treg8[REG_AL], 8); }
void _OUT_imm8_AX() { cpu_out(cpu_pfq_getbyte(), AX, 16); }
void _OUT_DX_AL() { cpu_out(DX, *treg8[REG_AL], 8); }
void _OUT_DX_AX() { cpu_out(DX, AX, 16); }

void _IN_AL_imm8() { *treg8[REG_AL] = cpu_in(cpu_pfq_getbyte(), 8); }
void _IN_AX_imm8() { AX = cpu_in(cpu_pfq_getbyte(), 16); }
void _IN_AL_DX() { *treg8[REG_AL] = cpu_in(DX, 8); }
void _IN_AX_DX() { AX = cpu_in(DX, 16); }

void
cpu_out (word port, word data, byte bits) {
	#ifdef VM_DEBUG
		if((iopeek)&&(port<0x60 || port>0x6F) && port != 0x3D4 && port != 0x3D5 && port != 0) {
			sprintf(tmp, "[%04X:%04X] cpu_out: %04X --> %04X\n", BCS, BIP, data, port);
			vm_out(tmp, VM_IOMSG);
		}
	#endif
	switch(port) {
		default:
			if(((port>0x5F)&&(port<0x70))||(port==0x00)) {
				if(bits==8) {
					vm_call8(port, (byte)data);
				} else {
					vm_call16(port, data);
				}
				break;
			}
			vm_ioh_out[port] (data, bits);
			break;		
	}
}

word
cpu_in (word port, byte bits) {
	#ifdef VM_DEBUG
		if((iopeek)&&(port<0x60 || port>0x6F) && port != 0x3D4 && port != 0x3D5 && port != 0) {
			sprintf(tmp, "[%04X:%04X] cpu_in: %04X\n", BCS, BIP, port);
			vm_out(tmp, VM_IOMSG);
		}
	#endif
	switch(port) {
		default:
			return vm_ioh_in[port] (bits);
			break;
	}
	return 0;
}

void
vm_listen (word port, word (*ioh_in) (byte), void (*ioh_out) (word, byte)) {
	vm_ioh_in[port] = ioh_in;
	vm_ioh_out[port] = ioh_out;
	return;
}

void
vm_ioh_nout (word port, byte bits) {
	(void) port;
	(void) bits;
	return;
}

word
vm_ioh_nin (byte bits) {
	(void) bits;
	return 0;
}

