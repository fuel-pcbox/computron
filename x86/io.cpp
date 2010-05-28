/* io.c
 * I/O instructions and functions
 *
 */

#include "vomit.h"
#include "debug.h"
#include "iodevice.h"
#include <QtCore/QHash>

typedef BYTE (*InputHandler) (VCpu*, WORD);
typedef void (*OutputHandler) (VCpu*, WORD, BYTE);

static QHash<WORD, InputHandler> s_inputHandlers;
static QHash<WORD, OutputHandler> s_outputHandlers;
static QSet<WORD> s_ignorePorts;

void _OUT_imm8_AL(VCpu* cpu)
{
    cpu->out(cpu->fetchOpcodeByte(), cpu->regs.B.AL);
}

void _OUT_imm8_AX(VCpu* cpu)
{
    WORD port = cpu->fetchOpcodeByte();
    cpu->out(port, cpu->regs.B.AL);
    cpu->out(port + 1, cpu->regs.B.AH);
}

void _OUT_DX_AL(VCpu* cpu)
{
    cpu->out(cpu->regs.W.DX, cpu->regs.B.AL);
}

void _OUT_DX_AX(VCpu* cpu)
{
    cpu->out(cpu->regs.W.DX, cpu->regs.B.AL);
    cpu->out(cpu->regs.W.DX + 1, cpu->regs.B.AH);
}

void _OUTSB(VCpu* cpu)
{
    BYTE b = cpu->readMemory8(cpu->currentSegment(), cpu->regs.W.SI);
    cpu->out(cpu->regs.W.DX, b);

    /* Modify SI according to DF */
    if (cpu->getDF() == 0)
        ++cpu->regs.W.SI;
    else
        --cpu->regs.W.SI;
}

void _OUTSW(VCpu* cpu)
{
    BYTE lsb = cpu->readMemory8(cpu->currentSegment(), cpu->regs.W.SI);
    BYTE msb = cpu->readMemory8(cpu->currentSegment(), cpu->regs.W.SI + 1);
    cpu->out(cpu->regs.W.DX, lsb);
    cpu->out(cpu->regs.W.DX + 1, msb);

    /* Modify SI according to DF */
    if (cpu->getDF() == 0)
        cpu->regs.W.SI += 2;
    else
        cpu->regs.W.SI -= 2;
}

void _IN_AL_imm8(VCpu* cpu)
{
    cpu->regs.B.AL = cpu->in(cpu->fetchOpcodeByte());
}

void _IN_AX_imm8(VCpu* cpu)
{
    WORD port = cpu->fetchOpcodeByte();
    cpu->regs.B.AL = cpu->in(port);
    cpu->regs.B.AH = cpu->in(port + 1);
}

void _IN_AL_DX(VCpu* cpu)
{
    cpu->regs.B.AL = cpu->in(cpu->regs.W.DX);
}

void _IN_AX_DX(VCpu* cpu)
{
    cpu->regs.B.AL = cpu->in(cpu->regs.W.DX);
    cpu->regs.B.AH = cpu->in(cpu->regs.W.DX + 1);
}

void VCpu::out(WORD port, BYTE value)
{
#ifdef VOMIT_DEBUG
    if (iopeek)
        vlog(VM_IOMSG, "[%04X:%04X] VCpu::out: %02X --> %04X", getBaseCS(), getBaseIP(), value, port);
#endif

    if (Vomit::IODevice::writeDevices().contains(port)) {
        Vomit::IODevice::writeDevices()[port]->out8(port, value);
        return;
    }

    if (!s_outputHandlers.contains(port)) {
        if (!s_ignorePorts.contains(port))
            vlog(VM_ALERT, "Unhandled I/O write to port %04X, data %02X", port, value);
        return;
    }

    s_outputHandlers[port](this, port, value);
}

BYTE VCpu::in(WORD port)
{
#ifdef VOMIT_DEBUG
    if (iopeek)
        vlog(VM_IOMSG, "[%04X:%04X] VCpu::in: %04X", getBaseCS(), getBaseIP(), port);
#endif

    if (Vomit::IODevice::readDevices().contains(port))
        return Vomit::IODevice::readDevices()[port]->in8(port);

    if (!s_inputHandlers.contains(port)) {
        if (!s_ignorePorts.contains(port))
            vlog(VM_ALERT, "Unhandled I/O read from port %04X", port);
        return 0xFF;
    }

    return s_inputHandlers[port](this, port);
}

void vm_listen(WORD port, InputHandler inputHandler, OutputHandler outputHandler)
{
    if (inputHandler || outputHandler)
        vlog(VM_IOMSG, "Adding listener(s) for port %04X", port);

    if (inputHandler)
        s_inputHandlers.insert(port, inputHandler);
    else
        s_inputHandlers.remove(port);

    if (outputHandler)
        s_outputHandlers.insert(port, outputHandler);
    else
        s_outputHandlers.remove(port);
}

void vomit_ignore_io_port(WORD port)
{
    s_ignorePorts.insert(port);
}
