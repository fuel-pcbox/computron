#include "vomit.h"
#include "debug.h"

static void pic_write_icw( word, byte );
static byte pic_read_icw( word );
static bool read_irr = 0;

static byte master_irr = 0;
static byte master_isr = 0;
static byte master_imr = 0xFF;

void
pic_init()
{
	vm_listen( 0x0020, pic_read_icw, pic_write_icw );
	vm_listen( 0x0021, pic_read_icw, pic_write_icw );
}

void
pic_write_icw( word port, byte data )
{
	if( port == 0x20 && data == 0x20 ) // non-specific EOI
	{
		//vlog( VM_PICMSG, "EOI" );
		return;
	}
	if( port == 0x20 && ((data & 0x18) == 0x08) ) // OCW3
	{
		//vlog( VM_PICMSG, "*** OCW3 *** %02X on port %02X", data, port );
		if( data & 0x02 )
			read_irr = data & 0x01;
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
		return;
	}
	if( port == 0x21 && ((data & 0x18) == 0x00) )
	{
		//vlog( VM_PICMSG, "*** OCW2 *** Data: %02X", data );
		return;
	}
	if( port == 0x21 ) // OCW1 - IMR write
	{
		vlog( VM_PICMSG, "New IRQ mask set: %02X", data );
		for( int i = 0; i < 8; ++i )
		{
			vlog( VM_PICMSG, " - IRQ %u: %s", i, (data & (1 << i)) ? "masked" : "service" );
		}
		master_imr = data;
		return;
	}
	vlog( VM_PICMSG, "Write PIC ICW on port %04X (data: %02X)", port, data );
	vlog( VM_PICMSG, "I can't handle that request, better quit!" );
	vm_kill( 1 );
}

byte
pic_read_icw( word port )
{
	master_irr = 0;
	master_isr = 0;
	if( read_irr )
	{
		//vlog( VM_PICMSG, "Read IRR (%02X)", master_irr );
		return master_irr;
	}
	else
	{
		vlog( VM_PICMSG, "Read ISR (%02X)", master_isr );
		return master_isr;
	}
	vlog( VM_PICMSG, "Read PIC ICW on port %04X", port );
	vlog( VM_PICMSG, "I can't handle that request, better quit!" );
	vm_kill( 1 );
	return 0;
}

void
irq( byte num )
{
	//vlog( VM_PICMSG, "IRQ %u", num );
	master_irr |= 1 << num;
}

bool
pic_next_irq( byte *retval )
{
	byte pending_requests = master_irr & ~master_imr;

	if( !pending_requests )
		return 0;

	for( int i = 0; i < 8; ++i )
		if( pending_requests & (1 << i) )
			*retval = i;
	master_irr &= ~(1 << *retval);
	return 1;
}
