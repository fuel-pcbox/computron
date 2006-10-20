/* io.c
 * I/O instructions and functions
 *
 */

#include "vomit.h"
#include "debug.h"

typedef byte (*tintab) (word);
typedef void (*touttab) (word, byte);

tintab vm_ioh_in[0x10000];
touttab vm_ioh_out[0x10000];

void
_OUT_imm8_AL()
{
	cpu_out( cpu_pfq_getbyte(), cpu.regs.B.AL );
}

void
_OUT_imm8_AX()
{
	word port = cpu_pfq_getbyte();
	cpu_out( port, cpu.regs.B.AL );
	cpu_out( port + 1, cpu.regs.B.AH );
}

void
_OUT_DX_AL()
{
	cpu_out( cpu.regs.W.DX, cpu.regs.B.AL );
}

void
_OUT_DX_AX()
{
	cpu_out( cpu.regs.W.DX, cpu.regs.B.AL );
	cpu_out( cpu.regs.W.DX + 1, cpu.regs.B.AH );
}

void
_IN_AL_imm8()
{
	cpu.regs.B.AL = cpu_in( cpu_pfq_getbyte() );
}

void
_IN_AX_imm8()
{
	word port = cpu_pfq_getbyte();
	cpu.regs.B.AL = cpu_in( port );
	cpu.regs.B.AH = cpu_in( port + 1 );
}

void
_IN_AL_DX()
{
	cpu.regs.B.AL = cpu_in( cpu.regs.W.DX );
}

void
_IN_AX_DX()
{
	cpu.regs.B.AL = cpu_in( cpu.regs.W.DX );
	cpu.regs.B.AH = cpu_in( cpu.regs.W.DX + 1 );
}

void
cpu_out( word port, byte data )
{
#ifdef VM_DEBUG
	if( iopeek )
		vlog( VM_IOMSG, "[%04X:%04X] cpu_out: %02X --> %04X", cpu.base_CS, cpu.base_IP, data, port );
#endif
	/* FIXME: Plug in via vm_listen() like everyone else. */
	if( port >= 0xE0 && port <= 0xEF )
		vm_call8( port, data );
	else
		vm_ioh_out[port]( port, data );
}

byte
cpu_in( word port )
{
#ifdef VM_DEBUG
	if( iopeek )
		vlog( VM_IOMSG, "[%04X:%04X] cpu_in: %04X", cpu.base_CS, cpu.base_IP, port );
#endif
	return vm_ioh_in[port]( port );
}

void
vm_listen( word port, byte (*ioh_in)(word), void (*ioh_out)(word, byte) )
{
	if( ioh_out || ioh_in )
		vlog( VM_IOMSG, "Adding listeners for port %04X", port );

	vm_ioh_in[port] = ioh_in ? ioh_in : vm_ioh_nin;
	vm_ioh_out[port] = ioh_out ? ioh_out : vm_ioh_nout;
}

void
vm_ioh_nout( word port, byte data )
{
	vlog( VM_IOMSG, "Write port %04X, data %02X", port, data );
}

byte
vm_ioh_nin( word port )
{
	vlog( VM_IOMSG, "Read port %04X", port );
	return 0;
}

