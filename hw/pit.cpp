#include "vomit.h"
#include "8086.h"

static byte pit_read(VCpu* cpu, word port);
static void pit_write(VCpu* cpu, word port, byte data);

void
pit_init()
{
    vm_listen( 0x40, pit_read, 0L );
    vm_listen( 0x42, 0L, pit_write );
    vm_listen( 0x43, 0L, pit_write );
}

byte
pit_read(VCpu* cpu, word port )
{
    (void) port;
    return rand() % 0xff;
}

void
pit_write(VCpu* cpu, word port, byte data )
{
    (void) data;
    switch( port )
    {
        case 0x43:
            // yeah
            break;
        default:
            break;
    }
}
