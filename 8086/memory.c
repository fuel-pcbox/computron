/* 8086/memory.c
 * Memory Functions
 *
 */

#include <stdlib.h>
#include <string.h>

#include "vomit.h"
#include "debug.h"

byte *mem_space;
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
#ifdef VM_DEBUG
	if( mempeek )
	{
		vlog( VM_MEMORYMSG, "%04X:%04X reading   BYTE at %08X", cpu.base_CS, cpu.base_IP, seg*16+off );
	}
#endif
	return mem_space[(seg<<4)+off];
}
word
mem_getword( word seg, word off )
{
#ifdef VM_DEBUG
	if( mempeek )
	{
		vlog( VM_MEMORYMSG, "%04X:%04X reading   WORD at %08X", cpu.base_CS, cpu.base_IP, seg*16+off );
	}
#endif

#ifdef VOMIT_CORRECTNESS
	if( off == 0xFFFF )
		return mem_space[(seg<<4)+off] + (mem_space[seg<<4]<<8);
#endif
	return mem_space[(seg<<4)+off] + (mem_space[(seg<<4)+off+1]<<8);
}

void
mem_setbyte( word seg, word off, byte b )
{
#ifdef VM_DEBUG
	if( mempeek )
	{
		vlog( VM_MEMORYMSG, "%04X:%04X writing   BYTE at %08X", cpu.base_CS, cpu.base_IP, seg*16+off );
	}
#endif
	mem_space[(seg<<4)+off]=b;
}
void
mem_setword( word seg, word off, word w )
{
#ifdef VM_DEBUG
	if( mempeek )
	{
		vlog( VM_MEMORYMSG, "%04X:%04X writing   WORD at %08X", cpu.base_CS, cpu.base_IP, seg*16+off );
	}
#endif

#ifdef VOMIT_CORRECTNESS
	if( off == 0xFFFF )
	{
		mem_space[(seg<<4)+off]=(byte)w; mem_space[seg<<4]=(byte)(w>>8);
		return;
	}
#endif
	mem_space[(seg<<4)+off]=(byte)w; mem_space[(seg<<4)+off+1]=(byte)(w>>8);
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
