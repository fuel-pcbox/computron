// Gameport
//
// Not really an emulation, just setting up a dummy I/O handlers
// so it won't spam me about reading port 0x201..

#include "vomit.h"

static BYTE gameport_read(VCpu* cpu, WORD port);
static void gameport_write(VCpu* cpu, WORD port, BYTE data);

void gameport_init()
{
    vm_listen(0x201, gameport_read, gameport_write);
}

BYTE gameport_read(VCpu*, WORD)
{
    return 0x80;
}

void gameport_write(VCpu*, WORD, BYTE)
{
}
