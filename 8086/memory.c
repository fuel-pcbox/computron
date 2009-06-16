/* 8086/memory.c
 * Memory Functions
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "vomit.h"
#include "debug.h"

extern byte vga_getbyte( dword );
extern word vga_getword( dword );
extern void vga_setbyte( dword, byte );
extern void vga_setword( dword, word );

byte *mem_space = 0;
word mem_avail = 640;

void
mem_init()
{
    mem_space = malloc( 1048576 + 65536 );
    if( !mem_space )
	{
		vlog( VM_INITMSG, "Insufficient memory available." );
		vm_exit( 1 );
	}
	memset( mem_space, 0, 1048576 + 65536 );
}

void
mem_kill()
{
    free( mem_space );
}

byte
mem_getbyte( word seg, word off )
{
	dword flat_address = FLAT( seg, off );

#ifdef VM_DEBUG
	if( mempeek )
	{
		vlog( VM_MEMORYMSG, "%04X:%04X reading   BYTE at %08X", cpu.base_CS, cpu.base_IP, flat_address );
	}
#endif

	if( flat_address >= 0xA0000 && flat_address < 0xB0000 )
		return vga_getbyte( flat_address );

#ifdef VM_DEBUG
	if( options.bda_peek )
	{
		if( flat_address >= 0x400 && flat_address <= 0x4FF )
		{
			vlog( VM_MEMORYMSG, "BDA: read byte at %08X (=%02X)", flat_address, mem_space[flat_address] );
		}
	}
#endif

	return mem_space[flat_address];
}
word
mem_getword( word seg, word off )
{
	dword flat_address = FLAT(seg, off);

#ifdef VM_DEBUG
	if( mempeek )
	{
		vlog( VM_MEMORYMSG, "%04X:%04X reading   WORD at %08X", cpu.base_CS, cpu.base_IP, flat_address );
	}
#endif

#ifdef VM_DEBUG
	if( options.bda_peek )
	{
		if( flat_address >= 0x400 && flat_address <= 0x4FF )
		{
			vlog( VM_MEMORYMSG, "BDA: read word at %08X (=%04X)", flat_address, mem_space[flat_address] + (mem_space[flat_address + 1]<<8) );
		}
	}
#endif

	if( flat_address >= 0xA0000 && flat_address < 0xB0000 )
		return vga_getword( flat_address );
#ifdef VOMIT_CORRECTNESS
	if( off == 0xFFFF )
		return mem_space[flat_address] + (mem_space[seg<<4]<<8);
#endif
	return mem_space[flat_address] + (mem_space[flat_address + 1]<<8);
}

void
mem_setbyte( word seg, word off, byte b )
{
	dword flat_address = FLAT( seg, off );
	#if 0
	if( off == 0x95FF )
	{
		ui_kill();
		vlog( VM_VIDEOMSG, "WEWT BOOYAH write %02X to %04X:%04X (%08X)\n", b, seg, off, flat_address );
		vm_debug();
		if( !g_debug_step )
			ui_show();
	}
	#endif

#ifdef VM_DEBUG
	if( mempeek )
	{
		vlog( VM_MEMORYMSG, "%04X:%04X writing   BYTE at %08X", cpu.base_CS, cpu.base_IP, flat_address );
	}

	if( options.bda_peek )
	{
		if( flat_address >= 0x400 && flat_address <= 0x4FF )
		{
			vlog( VM_MEMORYMSG, "BDA: write byte at %08X: %02X", flat_address, b );
		}
	}
#endif

	if( flat_address >= 0xA0000 && flat_address < 0xB0000 )
	{
		//fprintf( stderr, "yes vga: %08x\n", flat_address);
		vga_setbyte( flat_address, b );
	}
	else
	{
		//fprintf(stderr,"not vga: %08x\n", flat_address);
		mem_space[flat_address] = b;
	}

#if 0
	if( off == 0x95FF )
	{
 		ui_kill();
		g_debug_step = 1;
		vm_debug();
		if( !g_debug_step )
		  ui_show();
	}
#endif
}

void
mem_setword( word seg, word off, word w )
{
	dword flat_address = FLAT( seg, off );

#if 0
	if( off == 0xBFFC )
	{
		ui_kill();
		vlog( VM_VIDEOMSG, "LOOOOL BOOYAH write %04X to %04X:%04X (%08X)\n", w, seg, off, flat_address );
		vm_exit( 0 );
	}
#endif

#ifdef VM_DEBUG
	if( mempeek )
	{
		vlog( VM_MEMORYMSG, "%04X:%04X writing   WORD at %08X", cpu.base_CS, cpu.base_IP, seg*16+off );
	}

	if( options.bda_peek )
	{
		if( flat_address >= 0x400 && flat_address <= 0x4FF )
		{
			vlog( VM_MEMORYMSG, "BDA: write word at %08X: %04X", flat_address, w );
		}
	}
#endif

#ifdef VOMIT_CORRECTNESS
	if( off == 0xFFFF )
	{
		mem_space[flat_address] = (byte)w;
		mem_space[seg<<4] = (byte)(w>>8);
		return;
	}
#endif
	if( flat_address >= 0xA0000 && flat_address < 0xB0000 )
	{
		vga_setword( flat_address, w );
	}
	else
	{
		mem_space[flat_address] = (byte)w;
		mem_space[flat_address + 1] = (byte)(w>>8);
	}
}

void
mem_push( word w )
{
	cpu.regs.W.SP -= 2;
	mem_setword( cpu.SS, cpu.regs.W.SP, w );
}

word
mem_pop()
{
	word w = mem_getword( cpu.SS, cpu.regs.W.SP );
	cpu.regs.W.SP += 2;
	return w;
}

void
_LDS_reg16_mem16()
{
	byte rm = cpu_pfq_getbyte();
	dword value = modrm_read32( rm );
	*treg16[rmreg(rm)] = LSW(value);
	cpu.DS = MSW(value);
}
void
_LES_reg16_mem16()
{
	byte rm = cpu_pfq_getbyte();
	dword value = modrm_read32( rm );
	*treg16[rmreg(rm)] = LSW(value);
	cpu.ES = MSW(value);
}

char *
mem_get_ascii$( word seg, word off )
{
	char buf[256];
	buf[255] = 0;
	for( int i = 0; i < 255; ++i )
	{
		buf[i] = mem_getbyte( seg, off + i );
		if( buf[i] == '$' )
		{
			buf[i] = 0;
			break;
		}
	}
	return buf;
}
