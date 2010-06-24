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

void _OUT_imm8_EAX(VCpu* cpu)
{
    WORD port = cpu->fetchOpcodeByte();
    cpu->out(port, cpu->regs.B.AL);
    cpu->out(port + 1, cpu->regs.B.AH);
    cpu->out(port + 2, LSB(cpu->regs.W.__EAX_high_word));
    cpu->out(port + 3, MSB(cpu->regs.W.__EAX_high_word));
}

void _OUT_DX_AL(VCpu* cpu)
{
    cpu->out(cpu->getDX(), cpu->regs.B.AL);
}

void _OUT_DX_AX(VCpu* cpu)
{
    cpu->out(cpu->getDX(), cpu->regs.B.AL);
    cpu->out(cpu->getDX() + 1, cpu->regs.B.AH);
}

void _OUT_DX_EAX(VCpu* cpu)
{
    cpu->out(cpu->getDX(), cpu->regs.B.AL);
    cpu->out(cpu->getDX() + 1, cpu->regs.B.AH);
    cpu->out(cpu->getDX() + 2, LSB(cpu->regs.W.__EAX_high_word));
    cpu->out(cpu->getDX() + 3, MSB(cpu->regs.W.__EAX_high_word));
}

void _OUTSB(VCpu* cpu)
{
    BYTE data;

    if (cpu->a16()) {
        data = cpu->readMemory8(cpu->currentSegment(), cpu->getSI());
        cpu->nextSI(1);
    } else {
        data = cpu->readMemory8(cpu->currentSegment(), cpu->getESI());
        cpu->nextESI(1);
    }

    cpu->out(cpu->getDX(), data);
}

void _OUTSW(VCpu* cpu)
{
    BYTE lsb;
    BYTE msb;

    if (cpu->a16()) {
        lsb = cpu->readMemory8(cpu->currentSegment(), cpu->regs.W.SI);
        msb = cpu->readMemory8(cpu->currentSegment(), cpu->regs.W.SI + 1);
        cpu->nextSI(2);
    } else {
        lsb = cpu->readMemory8(cpu->currentSegment(), cpu->getSI());
        msb = cpu->readMemory8(cpu->currentSegment(), cpu->getESI() + 1);
        cpu->nextESI(2);
    }

    cpu->out(cpu->getDX(), lsb);
    cpu->out(cpu->getDX() + 1, msb);
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

void _IN_EAX_imm8(VCpu* cpu)
{
    WORD port = cpu->fetchOpcodeByte();
    cpu->regs.B.AL = cpu->in(port);
    cpu->regs.B.AH = cpu->in(port + 1);
    cpu->regs.W.__EAX_high_word = MAKEWORD(cpu->in(port + 2), cpu->in(port + 3));
}

void _IN_AL_DX(VCpu* cpu)
{
    cpu->regs.B.AL = cpu->in(cpu->getDX());
}

void _IN_AX_DX(VCpu* cpu)
{
    cpu->regs.B.AL = cpu->in(cpu->getDX());
    cpu->regs.B.AH = cpu->in(cpu->getDX() + 1);
}

void _IN_EAX_DX(VCpu* cpu)
{
    cpu->regs.B.AL = cpu->in(cpu->getDX());
    cpu->regs.B.AH = cpu->in(cpu->getDX() + 1);
    cpu->regs.W.__EAX_high_word = MAKEWORD(cpu->in(cpu->getDX() + 2), cpu->in(cpu->getDX() + 3));
}

void VCpu::out(WORD port, BYTE value)
{
#ifdef VOMIT_DEBUG
    if (iopeek)
        vlog(VM_IOMSG, "[%04X:%04X] VCpu::out: %02X --> %04X", getBaseCS(), getBaseIP(), value, port);
#endif

    if (IODevice::writeDevices().contains(port)) {
        IODevice::writeDevices()[port]->out8(port, value);
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

    if (IODevice::readDevices().contains(port))
        return IODevice::readDevices()[port]->in8(port);

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
