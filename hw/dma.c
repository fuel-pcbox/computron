/*
 * 8237 DMA controller emulation
 */

#include "vomit.h"
#include "debug.h"
#include <string.h>

#define DMA_CHANNELS 8

typedef struct
{
	word count;
	bool next_count_write_is_msb;
	bool next_count_read_is_msb;

	word address;
	bool next_address_write_is_msb;
	bool next_address_read_is_msb;
} dma_channel_t;

dma_channel_t dma1_channel[DMA_CHANNELS];
dma_channel_t dma2_channel[DMA_CHANNELS];

static void dma_ch_count_write( word port, word data, byte bits );
static word dma_ch_count_read( word port, byte bits );
static void dma_ch_address_write( word port, word data, byte bits );
static word dma_ch_address_read( word port, byte bits );

void
dma_init()
{
	int i;
	for( i = 0; i < DMA_CHANNELS; ++i )
	{
		memset( &dma1_channel[i], 0, sizeof(dma_channel_t) );
		memset( &dma2_channel[i], 0, sizeof(dma_channel_t) );
	}

	vm_listen( 0x000, dma_ch_address_read, dma_ch_address_write );
	vm_listen( 0x001, dma_ch_count_read, dma_ch_count_write );
}

void
dma_ch_count_write( word port, word data, byte bits )
{
	dma_channel_t *c;
	byte channel, chip;

	if( port == 0x001 )
	{
		channel = 0;
		chip = 1;
	}
	else
		vlog( VM_DMAMSG, "Unknown channel for port %03X", port );

	c = (chip == 1) ? &dma1_channel[channel] : &dma2_channel[channel];

	if( c->next_count_write_is_msb )
		c->count |= (data << 8);
	else
		c->count |= (data & 0xFF);

	vlog( VM_DMAMSG, "Write DMA-%u channel %u count (%s), data: %02X", chip, channel, c->next_count_write_is_msb ? "MSB" : "LSB", data );

	c->next_count_write_is_msb = !c->next_count_write_is_msb;
}

word
dma_ch_count_read( word port, byte bits )
{
	dma_channel_t *c;
	byte data;
	byte channel, chip;

	if( port == 0x001 )
	{
		channel = 0;
		chip = 1;
	}
	else
		vlog( VM_DMAMSG, "Unknown channel for port %03X", port );

	c = (chip == 1) ? &dma1_channel[channel] : &dma2_channel[channel];

	if( c->next_count_read_is_msb )
		data = (c->count >> 8);
	else
		data = c->count & 0xFF;

	vlog( VM_DMAMSG, "Read DMA-%u channel %u count (%s), data: %02X", chip, channel, c->next_count_read_is_msb ? "MSB" : "LSB", data );

	c->next_count_read_is_msb = !c->next_count_read_is_msb;
	return data;
}

void
dma_ch_address_write( word port, word data, byte bits )
{
	dma_channel_t *c;
	byte channel, chip;

	if( port == 0x000 )
	{
		channel = 0;
		chip = 1;
	}
	else
		vlog( VM_DMAMSG, "Unknown channel for port %03X", port );

	c = (chip == 1) ? &dma1_channel[channel] : &dma2_channel[channel];

	if( c->next_address_write_is_msb )
		c->address |= (data << 8);
	else
		c->address |= (data & 0xFF);

	vlog( VM_DMAMSG, "Write DMA-%u channel %u address (%s), data: %02X", chip, channel, c->next_address_write_is_msb ? "MSB" : "LSB", data );

	c->next_address_write_is_msb = !c->next_address_write_is_msb;
}

word
dma_ch_address_read( word port, byte bits )
{
	dma_channel_t *c;
	byte data;
	byte channel, chip;

	if( port == 0x000 )
	{
		channel = 0;
		chip = 1;
	}
	else
		vlog( VM_DMAMSG, "Unknown channel for port %03X", port );

	c = (chip == 1) ? &dma1_channel[channel] : &dma2_channel[channel];

	if( c->next_address_read_is_msb )
		data = (c->address >> 8);
	else
		data = c->address & 0xFF;

	vlog( VM_DMAMSG, "Read DMA-%u channel %u address (%s), data: %02X", chip, channel, c->next_address_read_is_msb ? "MSB" : "LSB", data );

	c->next_address_read_is_msb = !c->next_address_read_is_msb;
	return data;
}
