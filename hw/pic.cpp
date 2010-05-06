#include "vomit.h"
#include "debug.h"

static void pic_master_write(vomit_cpu_t *cpu, word, byte );
static byte pic_master_read(vomit_cpu_t *cpu, word );
static void pic_slave_write(vomit_cpu_t *cpu, word, byte );
static byte pic_slave_read(vomit_cpu_t *cpu, word );

static bool master_read_irr = 1;
static bool slave_read_irr = 1;

static byte master_irr = 0;
static byte master_isr = 0;
static byte master_imr = 0; // HACK - should be 0xff

static byte slave_irr = 0;
static byte slave_isr = 0;
static byte slave_imr = 0; // HACK - should be 0xff

static bool master_icw2_expected = false;
static bool slave_icw2_expected = false;

static byte master_addr_base = 0x08;
static byte slave_addr_base = 0x70;

word g_pic_pending_requests = 0;

#define UPDATE_PENDING_REQUESTS do { g_pic_pending_requests = (master_irr & ~master_imr) | ((slave_irr & ~slave_imr) << 8); } while(0);

void
pic_init()
{
    vm_listen( 0x0020, pic_master_read, pic_master_write );
    vm_listen( 0x0021, pic_master_read, pic_master_write );

    vm_listen( 0x00A0, pic_slave_read, pic_slave_write );
    vm_listen( 0x00A1, pic_slave_read, pic_slave_write );

    UPDATE_PENDING_REQUESTS;
}

void
pic_master_write(vomit_cpu_t *cpu, word port, byte data )
{
    if( port == 0x20 && data == 0x20 ) // non-specific EOI
    {
        //vlog( VM_PICMSG, "EOI" );
        master_isr = 0;
        return;
    }
    if( port == 0x21 && ((data & 0x7) == 0x00) && master_icw2_expected ) // ICW2
    {
        vlog( VM_ALERT, "Got ICW2 %02X on port %02X", data, port );
        master_addr_base = data & 0xF8;
        master_icw2_expected = false;
    }
    if( port == 0x20 && ((data & 0x18) == 0x08) ) // OCW3
    {
        vlog( VM_PICMSG, "*** OCW3 *** %02X on port %02X", data, port );
        if( data & 0x02 )
            master_read_irr = data & 0x01;
        return;
    }
    if( port == 0x20 && data & 0x10 ) // ICW1
    {
        vlog( VM_PICMSG, "*** ICW1 *** %02X on port %02X", data, port );
        // I'm not sure we'll ever see an ICW4...
        // icw4_needed = data & 0x01;
        vlog( VM_PICMSG, "[ICW1] Cascade = %s", (data & 2) ? "yes" : "no" );
        vlog( VM_PICMSG, "[ICW1] Vector size = %u", (data & 4) ? 4 : 8 );
        vlog( VM_PICMSG, "[ICW1] Level triggered = %s", (data & 8) ? "yes" : "no" );
        master_imr = 0;
        master_isr = 0;
        master_irr = 0;
        master_read_irr = 1;
        master_icw2_expected = 1;
        UPDATE_PENDING_REQUESTS;
        return;
    }
    if( port == 0x20 && ((data & 0x18) == 0x00) )
    {
        vlog( VM_PICMSG, "*** OCW2 *** Data: %02X", data );
        return;
    }
    if( port == 0x21 ) // OCW1 - IMR write
    {
        int i;
        vlog( VM_PICMSG, "New IRQ mask set: %02X", data );
        for( i = 0; i < 8; ++i )
        {
            vlog( VM_PICMSG, " - IRQ %u: %s", i, (data & (1 << i)) ? "masked" : "service" );
        }
        master_imr = data;
        UPDATE_PENDING_REQUESTS;
        return;
    }
    vlog( VM_PICMSG, "Write PIC ICW on port %04X (data: %02X)", port, data );
    vlog( VM_PICMSG, "I can't handle that request, better quit!" );
    vm_exit( 1 );
}

byte
pic_master_read(vomit_cpu_t *cpu, word port )
{
    //master_irr = 0;
    //master_isr = 0;
    if( master_read_irr )
    {
        vlog( VM_PICMSG, "Read IRR (%02X)", master_irr );
        return master_irr;
    }
    else
    {
        vlog( VM_PICMSG, "Read ISR (%02X)", master_isr );
        return master_isr;
    }
    vlog( VM_PICMSG, "Read PIC ICW on port %04X", port );
    vlog( VM_PICMSG, "I can't handle that request, better quit!" );
    vm_exit( 1 );
    return 0;
}

void
irq( byte num )
{
    //vlog( VM_PICMSG, "IRQ %u", num );

    if( num < 8 )
    {
        master_irr |= 1 << num;
        master_isr |= 1 << num;
    }
    else
    {
        slave_irr |= 1 << (num - 8);
    }

    UPDATE_PENDING_REQUESTS;
}

void pic_service_irq(vomit_cpu_t *cpu)
{
    int i;

    if( !g_pic_pending_requests )
        return;

    byte interrupt_to_service = 0xFF;

    for( i = 0; i < 16; ++i )
        if( g_pic_pending_requests & (1 << i) )
            interrupt_to_service = i;

    if( interrupt_to_service == 0xFF )
        return;

    if( interrupt_to_service < 8 )
    {
        master_irr &= ~(1 << interrupt_to_service);
        master_isr |= (1 << interrupt_to_service);

        cpu->jumpToInterruptHandler(master_addr_base | interrupt_to_service);
    }
    else
    {
        slave_irr &= ~(1 << (interrupt_to_service - 8));
        slave_isr |= (1 << (interrupt_to_service - 8));

        cpu->jumpToInterruptHandler(slave_addr_base | interrupt_to_service);
    }

    UPDATE_PENDING_REQUESTS;

    cpu->setState(VCpu::Alive);
}

void
pic_slave_write(vomit_cpu_t *cpu, word port, byte data )
{
    if( port == 0xA0 && data == 0x20 ) // non-specific EOI
    {
        //vlog( VM_PICMSG, "EOI" );
        slave_isr = 0;
        return;
    }
    if( port == 0xA1 && ((data & 0x7) == 0x00) && slave_icw2_expected ) // ICW2
    {
        vlog( VM_ALERT, "Got ICW2 %02X on port %02X", data, port );
        slave_addr_base = data & 0xF8;
        slave_icw2_expected = false;
    }
    if( port == 0xA0 && ((data & 0x18) == 0x08) ) // OCW3
    {
        vlog( VM_PICMSG, "*** OCW3 *** %02X on port %02X", data, port );
        if( data & 0x02 )
            slave_read_irr = data & 0x01;
        return;
    }
    if( port == 0xA0 && data & 0x10 ) // ICW1
    {
        vlog( VM_PICMSG, "*** ICW1 *** %02X on port %02X", data, port );
        // I'm not sure we'll ever see an ICW4...
        // icw4_needed = data & 0x01;
        vlog( VM_PICMSG, "[ICW1] Cascade = %s", (data & 2) ? "yes" : "no" );
        vlog( VM_PICMSG, "[ICW1] Vector size = %u", (data & 4) ? 4 : 8 );
        vlog( VM_PICMSG, "[ICW1] Level triggered = %s", (data & 8) ? "yes" : "no" );
        slave_imr = 0;
        slave_isr = 0;
        slave_irr = 0;
        slave_read_irr = 1;
        slave_icw2_expected = 1;
        UPDATE_PENDING_REQUESTS;
        return;
    }
    if( port == 0xA0 && ((data & 0x18) == 0x00) )
    {
        vlog( VM_PICMSG, "*** OCW2 *** Data: %02X", data );
        return;
    }
    if( port == 0xA1 ) // OCW1 - IMR write
    {
        int i;
        vlog( VM_PICMSG, "New IRQ mask set: %02X", data );
        for( i = 0; i < 8; ++i )
        {
            vlog( VM_PICMSG, " - IRQ %u: %s", i, (data & (1 << i)) ? "masked" : "service" );
        }
        slave_imr = data;
        UPDATE_PENDING_REQUESTS;
        return;
    }
    vlog( VM_PICMSG, "Write PIC ICW on port %04X (data: %02X)", port, data );
    vlog( VM_PICMSG, "I can't handle that request, better quit!" );
    vm_exit( 1 );
}

byte
pic_slave_read(vomit_cpu_t *cpu, word port )
{
    (void) port;
    vlog( VM_PICMSG, "Can't read from slave yet!" );
    vm_exit( 1 );
    return 0;
}
