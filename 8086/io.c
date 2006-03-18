/* io.c
 * I/O instructions and functions
 *
 */

#include "vomit.h"
#include "debug.h"

tintab vm_ioh_in[0xFFFF];
touttab vm_ioh_out[0xFFFF];

#ifdef VM_DEBUG
static word s_port;
#endif

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
	if( iopeek )
	{
		vlog( VM_IOMSG, "[%04X:%04X] cpu_out: %04X --> %04X", BCS, BIP, data, port );
	}
	s_port = port;
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
cpu_in( word port, byte bits )
{
#ifdef VM_DEBUG
	if( iopeek )
	{
		vlog( VM_IOMSG, "[%04X:%04X] cpu_in: %04X", BCS, BIP, port );
	}
#endif
	return vm_ioh_in[port]( bits );
}

void
vm_listen( word port, word (*ioh_in) (byte), void (*ioh_out) (word, byte) )
{
	if( ioh_out != vm_ioh_nout && ioh_in != vm_ioh_nin )
	{
		vlog( VM_IOMSG, "Adding listeners for port %04X", port );
	}

	vm_ioh_in[port] = ioh_in;
	vm_ioh_out[port] = ioh_out;
}

void
vm_ioh_nout( word data, byte bits )
{
	vlog( VM_IOMSG, "Write port %04X, data %04X (%d bits)", s_port, data, bits );
}

word
vm_ioh_nin( byte bits )
{
	vlog( VM_IOMSG, "Read port %04X (%d bits)", s_port, bits );
	return 0;
}

