/* io.c
 * I/O instructions and functions
 *
 */

#include "vomit.h"
#include "debug.h"

tintab vm_ioh_in[0xFFFF];
touttab vm_ioh_out[0xFFFF];

void _OUT_imm8_AL() { cpu_out( cpu_pfq_getbyte(), cpu.regs.B.AL, 8); }
void _OUT_imm8_AX() { cpu_out( cpu_pfq_getbyte(), cpu.regs.W.AX, 16); }
void _OUT_DX_AL() { cpu_out( cpu.regs.W.DX, cpu.regs.B.AL, 8 ); }
void _OUT_DX_AX() { cpu_out( cpu.regs.W.DX, cpu.regs.W.AX, 16 ); }

void _IN_AL_imm8() { cpu.regs.B.AL = cpu_in( cpu_pfq_getbyte(), 8 ); }
void _IN_AX_imm8() { cpu.regs.W.AX = cpu_in( cpu_pfq_getbyte(), 16 ); }
void _IN_AL_DX() { cpu.regs.B.AL = cpu_in( cpu.regs.W.DX, 8 ); }
void _IN_AX_DX() { cpu.regs.W.AX = cpu_in( cpu.regs.W.DX, 16 ); }

void
cpu_out (word port, word data, byte bits) {
#ifdef VM_DEBUG
	if( iopeek )
	{
		vlog( VM_IOMSG, "[%04X:%04X] cpu_out: %04X --> %04X", cpu.base_CS, cpu.base_IP, data, port );
	}
#endif
	switch(port) {
		default:
			if( port >= 0xE0 && port <= 0xEF ) {
				if(bits==8) {
					vm_call8(port, (byte)data);
				} else {
					vm_call16(port, data);
				}
				break;
			}
			vm_ioh_out[port] (port, data, bits);
			break;
	}
}

word
cpu_in( word port, byte bits )
{
#ifdef VM_DEBUG
	if( iopeek )
	{
		vlog( VM_IOMSG, "[%04X:%04X] cpu_in: %04X", cpu.base_CS, cpu.base_IP, port );
	}
#endif
	return vm_ioh_in[port]( port, bits );
}

void
vm_listen( word port, word (*ioh_in) (word, byte), void (*ioh_out) (word, word, byte) )
{
	if( ioh_out || ioh_in )
	{
		vlog( VM_IOMSG, "Adding listeners for port %04X", port );
	}

	vm_ioh_in[port] = ioh_in ? ioh_in : vm_ioh_nin;
	vm_ioh_out[port] = ioh_out ? ioh_out : vm_ioh_nout;
}

void
vm_ioh_nout( word port, word data, byte bits )
{
	vlog( VM_IOMSG, "Write port %04X, data %04X (%d bits)", port, data, bits );
}

word
vm_ioh_nin( word port, byte bits )
{
	vlog( VM_IOMSG, "Read port %04X (%d bits)", port, bits );
	return 0;
}

