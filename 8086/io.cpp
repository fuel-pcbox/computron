/* io.c
 * I/O instructions and functions
 *
 */

#include "vomit.h"
#include "debug.h"
#include "iodevice.h"

typedef BYTE (*tintab) (vomit_cpu_t *, WORD);
typedef void (*touttab) (vomit_cpu_t *, WORD, BYTE);

static tintab vm_ioh_in[0x10000];
static touttab vm_ioh_out[0x10000];

void _OUT_imm8_AL(vomit_cpu_t *cpu)
{
    vomit_cpu_out(cpu, vomit_cpu_pfq_getbyte(cpu), cpu->regs.B.AL);
}

void _OUT_imm8_AX(vomit_cpu_t *cpu)
{
    WORD port = vomit_cpu_pfq_getbyte(cpu);
    vomit_cpu_out(cpu, port, cpu->regs.B.AL );
    vomit_cpu_out(cpu, port + 1, cpu->regs.B.AH);
}

void _OUT_DX_AL(vomit_cpu_t *cpu)
{
    vomit_cpu_out(cpu, cpu->regs.W.DX, cpu->regs.B.AL );
}

void _OUT_DX_AX(vomit_cpu_t *cpu)
{
    vomit_cpu_out(cpu, cpu->regs.W.DX, cpu->regs.B.AL);
    vomit_cpu_out(cpu, cpu->regs.W.DX + 1, cpu->regs.B.AH);
}

void _OUTSB(vomit_cpu_t *cpu)
{
    BYTE b = vomit_cpu_memory_read8(cpu, *(cpu->CurrentSegment), cpu->regs.W.SI);
    vomit_cpu_out(cpu, cpu->regs.W.DX, b);

    /* Modify SI according to DF */
    if (cpu->DF == 0)
        ++cpu->regs.W.SI;
    else
        --cpu->regs.W.SI;
}

void _OUTSW(vomit_cpu_t *cpu)
{
    BYTE lsb = vomit_cpu_memory_read8(cpu, *(cpu->CurrentSegment), cpu->regs.W.SI);
    BYTE msb = vomit_cpu_memory_read8(cpu, *(cpu->CurrentSegment), cpu->regs.W.SI + 1);
    vomit_cpu_out(cpu, cpu->regs.W.DX, lsb);
    vomit_cpu_out(cpu, cpu->regs.W.DX + 1, msb);

    /* Modify SI according to DF */
    if (cpu->DF == 0)
        cpu->regs.W.SI += 2;
    else
        cpu->regs.W.SI -= 2;
}

void _IN_AL_imm8(vomit_cpu_t *cpu)
{
    cpu->regs.B.AL = vomit_cpu_in(cpu, vomit_cpu_pfq_getbyte(cpu));
}

void _IN_AX_imm8(vomit_cpu_t *cpu)
{
    WORD port = vomit_cpu_pfq_getbyte(cpu);
    cpu->regs.B.AL = vomit_cpu_in(cpu, port);
    cpu->regs.B.AH = vomit_cpu_in(cpu, port + 1);
}

void _IN_AL_DX(vomit_cpu_t *cpu)
{
    cpu->regs.B.AL = vomit_cpu_in(cpu, cpu->regs.W.DX);
}

void _IN_AX_DX(vomit_cpu_t *cpu)
{
    cpu->regs.B.AL = vomit_cpu_in(cpu, cpu->regs.W.DX);
    cpu->regs.B.AH = vomit_cpu_in(cpu, cpu->regs.W.DX + 1);
}

void vomit_cpu_out(vomit_cpu_t *cpu, WORD port, BYTE value)
{
#ifdef VOMIT_DEBUG
    if( iopeek )
        vlog(VM_IOMSG, "[%04X:%04X] cpu_out: %02X --> %04X", cpu->base_CS, cpu->base_IP, value, port);
#endif

    if (Vomit::IODevice::writeDevices().contains(port)) {
        Vomit::IODevice::writeDevices()[port]->out8(value);
        return;
    }

    /* FIXME: Plug in via vm_listen() like everyone else. */
    if( port >= 0xE0 && port <= 0xEF )
        vm_call8(cpu, port, value);
    else
        vm_ioh_out[port](cpu, port, value);
}

BYTE vomit_cpu_in(vomit_cpu_t *cpu, WORD port)
{
#ifdef VOMIT_DEBUG
    if (iopeek)
        vlog(VM_IOMSG, "[%04X:%04X] vomit_cpu_in: %04X", cpu->base_CS, cpu->base_IP, port);
#endif

    if (Vomit::IODevice::readDevices().contains(port))
        return Vomit::IODevice::readDevices()[port]->in8();

    return vm_ioh_in[port](cpu, port);
}

static void vm_ioh_nout(vomit_cpu_t *, WORD port, BYTE value)
{
    vlog(VM_ALERT, "Unhandled I/O write to port %04X, data %02X", port, value);
}

static BYTE vm_ioh_nin(vomit_cpu_t *, WORD port)
{
    vlog(VM_ALERT, "Unhandled I/O read from port %04X", port);
    return 0xff;
}

void vm_listen( word port, tintab ioh_in, touttab ioh_out )
{
    if (ioh_out || ioh_in)
	vlog(VM_IOMSG, "Adding listener(s) for port %04X", port);

    vm_ioh_in[port] = ioh_in ? ioh_in : vm_ioh_nin;
    vm_ioh_out[port] = ioh_out ? ioh_out : vm_ioh_nout;
}
