#include "vomit.h"
#include "vcpu.h"

static byte pit_read(VCpu*, WORD);
static void pit_write(VCpu*, WORD, BYTE);

void pit_init()
{
    vm_listen(0x40, pit_read, 0L);
    vm_listen(0x42, 0L, pit_write);
    vm_listen(0x43, 0L, pit_write);
}

BYTE pit_read(VCpu*, WORD)
{
    return rand() % 0xff;
}

void pit_write(VCpu*, WORD port, BYTE data)
{
    switch (port) {
    case 0x43:
        // yeah
        break;
    default:
        break;
    }
}
