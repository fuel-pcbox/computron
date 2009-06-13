#include "vomit.h"
#include "debug.h"
#include <stdlib.h>

#define DEFAULT_TO_SS if( cpu.CurrentSegment != &cpu.SegmentPrefix ) { segment = cpu.SS; }
static void *s_last_modrm_ptr = 0L;
static int s_last_is_register = 0;

static word s_last_modrm_segment = 0;
static word s_last_modrm_offset = 0;

void
modrm_write16( byte rmbyte, word data )
{
	byte *rmp = modrm_resolve16( rmbyte );
	if( MODRM_ISREG( rmbyte ))
	{
		*((word *)rmp) = data;
		return;
	}
	mem_setword( s_last_modrm_segment, s_last_modrm_offset, data );
}

void
modrm_write8( byte rmbyte, byte data )
{
	byte *rmp = modrm_resolve8( rmbyte );
	if( MODRM_ISREG( rmbyte ))
	{
		*rmp = data;
		return;
	}
	mem_setbyte( s_last_modrm_segment, s_last_modrm_offset, data );
}

word
modrm_read16( byte rmbyte )
{
	byte *rmp = modrm_resolve16( rmbyte );
	if( MODRM_ISREG( rmbyte ))
		return *((word *)rmp);
	return mem_getword( s_last_modrm_segment, s_last_modrm_offset );
}

byte
modrm_read8( byte rmbyte )
{
	byte *rmp = modrm_resolve8( rmbyte );
	if( MODRM_ISREG( rmbyte ))
		return *rmp;
	return mem_getbyte( s_last_modrm_segment, s_last_modrm_offset );
}

void
modrm_update16( word data )
{
	if( s_last_is_register )
	{
		*((word *)s_last_modrm_ptr) = data;
		return;
	}
	mem_setword( s_last_modrm_segment, s_last_modrm_offset, data );
}

void
modrm_update8( byte data )
{
	if( s_last_is_register )
	{
		*((byte *)s_last_modrm_ptr) = data;
		return;
	}
	mem_setbyte( s_last_modrm_segment, s_last_modrm_offset, data );
}

dword
modrm_read32( byte rmbyte )
{
	/* NOTE: We don't need modrm_resolve32() at the moment. */
	byte *rmp = modrm_resolve8( rmbyte );
	if( MODRM_ISREG( rmbyte ))
	{
		vlog( VM_CPUMSG, "PANIC: Attempt to read 32-bit register." );
		uasm( cpu.base_CS, cpu.base_IP );
		vm_kill();
		exit( 1 );
	}

	return rmp[0] | (rmp[1]<<8) | (rmp[2]<<16) | (rmp[3]<<24);
}

void
_LEA_reg16_mem16()
{
	word retv = 0x0000;
	byte b = cpu_pfq_getbyte();
	switch (b & 0xC0) {
        case 0:
            switch(b & 0x07)
            {
                case 0: retv = cpu.regs.W.BX+cpu.regs.W.SI; break;
                case 1: retv = cpu.regs.W.BX+cpu.regs.W.DI; break;
                case 2: retv = cpu.regs.W.BP+cpu.regs.W.SI; break;
                case 3: retv = cpu.regs.W.BP+cpu.regs.W.DI; break;
                case 4: retv = cpu.regs.W.SI; break;
                case 5: retv = cpu.regs.W.DI; break;
                case 6: retv = cpu_pfq_getword(); break;
                default: retv = cpu.regs.W.BX; break;
            }
            break;
        case 64:
            switch(b & 0x07)
            {
                case 0: retv = cpu.regs.W.BX+cpu.regs.W.SI + signext(cpu_pfq_getbyte()); break;
                case 1: retv = cpu.regs.W.BX+cpu.regs.W.DI + signext(cpu_pfq_getbyte()); break;
                case 2: retv = cpu.regs.W.BP+cpu.regs.W.SI + signext(cpu_pfq_getbyte()); break;
                case 3: retv = cpu.regs.W.BP+cpu.regs.W.DI + signext(cpu_pfq_getbyte()); break;
                case 4: retv = cpu.regs.W.SI + signext(cpu_pfq_getbyte()); break;
                case 5: retv = cpu.regs.W.DI + signext(cpu_pfq_getbyte()); break;
                case 6: retv = cpu.regs.W.BP + signext(cpu_pfq_getbyte()); break;
                default: retv = cpu.regs.W.BX + signext(cpu_pfq_getbyte()); break;
            }
            break;
        case 128:
            switch(b & 0x07)
            {
                case 0: retv = cpu.regs.W.BX+cpu.regs.W.SI+cpu_pfq_getword(); break;
                case 1: retv = cpu.regs.W.BX+cpu.regs.W.DI+cpu_pfq_getword(); break;
                case 2: retv = cpu.regs.W.BP+cpu.regs.W.SI+cpu_pfq_getword(); break;
                case 3: retv = cpu.regs.W.BP+cpu.regs.W.DI+cpu_pfq_getword(); break;
                case 4: retv = cpu.regs.W.SI + cpu_pfq_getword(); break;
                case 5: retv = cpu.regs.W.DI + cpu_pfq_getword(); break;
                case 6: retv = cpu.regs.W.BP + cpu_pfq_getword(); break;
                default: retv = cpu.regs.W.BX + cpu_pfq_getword(); break;
            }
            break;
		case 192:
			vlog( VM_ALERT, "LEA with register source!" );
			/* LEA with register source, an invalid instruction.
			 * Call INT6 (invalid opcode exception) */
			int_call( 6 );
			break;
	}
	*treg16[rmreg( b )] = retv;
}

void *
modrm_resolve8( byte rmbyte )
{
	word segment = *cpu.CurrentSegment;
	word offset = 0x0000;

	switch( rmbyte & 0xC0 )
	{
		case 0x00:
			s_last_is_register = 0;
			switch( rmbyte & 0x07 )
			{
				case 0: offset = cpu.regs.W.BX + cpu.regs.W.SI; break;
				case 1: offset = cpu.regs.W.BX + cpu.regs.W.DI; break;
				case 2: DEFAULT_TO_SS; offset = cpu.regs.W.BP + cpu.regs.W.SI; break;
				case 3: DEFAULT_TO_SS; offset = cpu.regs.W.BP + cpu.regs.W.DI; break;
				case 4: offset = cpu.regs.W.SI; break;
				case 5: offset = cpu.regs.W.DI; break;
				case 6: offset = cpu_pfq_getword(); break;
				default: offset = cpu.regs.W.BX; break;
			}
			s_last_modrm_segment = segment;
			s_last_modrm_offset = offset;
			s_last_modrm_ptr = &mem_space[(segment<<4) + offset];
			break;
		case 0x40:
			s_last_is_register = 0;
			offset = signext( cpu_pfq_getbyte() );
			switch( rmbyte & 0x07 )
			{
				case 0: offset += cpu.regs.W.BX + cpu.regs.W.SI; break;
				case 1: offset += cpu.regs.W.BX + cpu.regs.W.DI; break;
				case 2: DEFAULT_TO_SS; offset += cpu.regs.W.BP + cpu.regs.W.SI; break;
				case 3: DEFAULT_TO_SS; offset += cpu.regs.W.BP + cpu.regs.W.DI; break;
				case 4: offset += cpu.regs.W.SI; break;
				case 5: offset += cpu.regs.W.DI; break;
				case 6: DEFAULT_TO_SS; offset += cpu.regs.W.BP; break;
				default: offset += cpu.regs.W.BX; break;
			}
			s_last_modrm_segment = segment;
			s_last_modrm_offset = offset;
			s_last_modrm_ptr = &mem_space[(segment<<4) + offset];
			break;
		case 0x80:
			s_last_is_register = 0;
			offset = cpu_pfq_getword();
			switch( rmbyte & 0x07 )
			{
				case 0: offset += cpu.regs.W.BX + cpu.regs.W.SI; break;
				case 1: offset += cpu.regs.W.BX + cpu.regs.W.DI; break;
				case 2: DEFAULT_TO_SS; offset += cpu.regs.W.BP + cpu.regs.W.SI; break;
				case 3: DEFAULT_TO_SS; offset += cpu.regs.W.BP + cpu.regs.W.DI; break;
				case 4: offset += cpu.regs.W.SI; break;
				case 5: offset += cpu.regs.W.DI; break;
				case 6: DEFAULT_TO_SS; offset += cpu.regs.W.BP; break;
				default: offset += cpu.regs.W.BX; break;
			}
			s_last_modrm_segment = segment;
			s_last_modrm_offset = offset;
			s_last_modrm_ptr = &mem_space[(segment<<4) + offset];
			break;
		case 0xC0:
			s_last_is_register = 1;
			switch( rmbyte & 0x07 )
			{
				case 0: s_last_modrm_ptr = &cpu.regs.B.AL; break;
				case 1: s_last_modrm_ptr = &cpu.regs.B.CL; break;
				case 2: s_last_modrm_ptr = &cpu.regs.B.DL; break;
				case 3: s_last_modrm_ptr = &cpu.regs.B.BL; break;
				case 4: s_last_modrm_ptr = &cpu.regs.B.AH; break;
				case 5: s_last_modrm_ptr = &cpu.regs.B.CH; break;
				case 6: s_last_modrm_ptr = &cpu.regs.B.DH; break;
				default: s_last_modrm_ptr = &cpu.regs.B.BH; break;
			}
			break;
	}
	return s_last_modrm_ptr;
}

void *
modrm_resolve16( byte rmbyte )
{
	word segment = *cpu.CurrentSegment;
	word offset = 0x0000;

	switch( rmbyte & 0xC0 )
	{
		case 0x00:
			s_last_is_register = 0;
			switch( rmbyte & 0x07 )
			{
				case 0: offset = cpu.regs.W.BX + cpu.regs.W.SI; break;
				case 1: offset = cpu.regs.W.BX + cpu.regs.W.DI; break;
				case 2: DEFAULT_TO_SS; offset = cpu.regs.W.BP + cpu.regs.W.SI; break;
				case 3: DEFAULT_TO_SS; offset = cpu.regs.W.BP + cpu.regs.W.DI; break;
				case 4: offset = cpu.regs.W.SI; break;
				case 5: offset = cpu.regs.W.DI; break;
				case 6: offset = cpu_pfq_getword(); break;
				default: offset = cpu.regs.W.BX; break;
			}
			s_last_modrm_segment = segment;
			s_last_modrm_offset = offset;
			s_last_modrm_ptr = &mem_space[(segment<<4) + offset];
			break;
		case 0x40:
			s_last_is_register = 0;
			offset = signext( cpu_pfq_getbyte() );
			switch( rmbyte & 0x07 )
			{
				case 0: offset += cpu.regs.W.BX + cpu.regs.W.SI; break;
				case 1: offset += cpu.regs.W.BX + cpu.regs.W.DI; break;
				case 2: DEFAULT_TO_SS; offset += cpu.regs.W.BP + cpu.regs.W.SI; break;
				case 3: DEFAULT_TO_SS; offset += cpu.regs.W.BP + cpu.regs.W.DI; break;
				case 4: offset += cpu.regs.W.SI; break;
				case 5: offset += cpu.regs.W.DI; break;
				case 6: DEFAULT_TO_SS; offset += cpu.regs.W.BP; break;
				default: offset += cpu.regs.W.BX; break;
			}
			s_last_modrm_segment = segment;
			s_last_modrm_offset = offset;
			s_last_modrm_ptr = &mem_space[(segment<<4) + offset];
			break;
		case 0x80:
			s_last_is_register = 0;
			offset = cpu_pfq_getword();
			switch( rmbyte & 0x07 )
			{
				case 0: offset += cpu.regs.W.BX + cpu.regs.W.SI; break;
				case 1: offset += cpu.regs.W.BX + cpu.regs.W.DI; break;
				case 2: DEFAULT_TO_SS; offset += cpu.regs.W.BP + cpu.regs.W.SI; break;
				case 3: DEFAULT_TO_SS; offset += cpu.regs.W.BP + cpu.regs.W.DI; break;
				case 4: offset += cpu.regs.W.SI; break;
				case 5: offset += cpu.regs.W.DI; break;
				case 6: DEFAULT_TO_SS; offset += cpu.regs.W.BP; break;
				default: offset += cpu.regs.W.BX; break;
			}
			s_last_modrm_segment = segment;
			s_last_modrm_offset = offset;
			s_last_modrm_ptr = &mem_space[(segment<<4) + offset];
			break;
		case 0xC0:
			s_last_is_register = 1;
			switch( rmbyte & 0x07 )
			{
				case 0: s_last_modrm_ptr = &cpu.regs.W.AX; break;
				case 1: s_last_modrm_ptr = &cpu.regs.W.CX; break;
				case 2: s_last_modrm_ptr = &cpu.regs.W.DX; break;
				case 3: s_last_modrm_ptr = &cpu.regs.W.BX; break;
				case 4: s_last_modrm_ptr = &cpu.regs.W.SP; break;
				case 5: s_last_modrm_ptr = &cpu.regs.W.BP; break;
				case 6: s_last_modrm_ptr = &cpu.regs.W.SI; break;
				default: s_last_modrm_ptr = &cpu.regs.W.DI; break;
			}
			break;
	}
	return s_last_modrm_ptr;
}

