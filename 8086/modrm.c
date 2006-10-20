#include "vomit.h"
#include "debug.h"

#define DEFAULT_TO_SS if( cpu.CurrentSegment == &cpu.SegmentPrefix ) { segment = *cpu.CurrentSegment; } else { segment = cpu.SS; }
static void *s_last_modrm_ptr = 0L;

void *
modrm_resolve( byte rmbyte, byte bits )
{
	word segment = *cpu.CurrentSegment;
	word offset = 0x0000;

	switch( rmbyte & 0xC0 )
	{
		case 0x00:
			switch( rmbyte & 0x07 )
			{
				case 0: offset = cpu.regs.W.BX + cpu.regs.W.SI; break;
				case 1: offset = cpu.regs.W.BX + cpu.regs.W.DI; break;
				case 2: DEFAULT_TO_SS; offset = cpu.regs.W.BP + cpu.regs.W.SI; break;
				case 3: DEFAULT_TO_SS; offset = cpu.regs.W.BP + cpu.regs.W.DI; break;
				case 4: offset = cpu.regs.W.SI; break;
				case 5: offset = cpu.regs.W.DI; break;
				case 6: offset = cpu_pfq_getword(); break;
				case 7: offset = cpu.regs.W.BX; break;
			}
			s_last_modrm_ptr = &mem_space[(segment<<4) + offset];
			break;
		case 0x40:
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
				case 7: offset += cpu.regs.W.BX; break;
			}
			s_last_modrm_ptr = &mem_space[(segment<<4) + offset];
			break;
		case 0x80:
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
				case 7: offset += cpu.regs.W.BX; break;
			}
			s_last_modrm_ptr = &mem_space[(segment<<4) + offset];
			break;
		case 0xC0:
			switch( rmbyte & 0x07 )
			{
				case 0: s_last_modrm_ptr = (bits == 8) ? &cpu.regs.B.AL : &cpu.regs.W.AX; break;
				case 1: s_last_modrm_ptr = (bits == 8) ? &cpu.regs.B.CL : &cpu.regs.W.CX; break;
				case 2: s_last_modrm_ptr = (bits == 8) ? &cpu.regs.B.DL : &cpu.regs.W.DX; break;
				case 3: s_last_modrm_ptr = (bits == 8) ? &cpu.regs.B.BL : &cpu.regs.W.BX; break;
				case 4: s_last_modrm_ptr = (bits == 8) ? &cpu.regs.B.AH : &cpu.regs.W.SP; break;
				case 5: s_last_modrm_ptr = (bits == 8) ? &cpu.regs.B.CH : &cpu.regs.W.BP; break;
				case 6: s_last_modrm_ptr = (bits == 8) ? &cpu.regs.B.DH : &cpu.regs.W.SI; break;
				case 7: s_last_modrm_ptr = (bits == 8) ? &cpu.regs.B.BH : &cpu.regs.W.DI; break;
			}
			break;
	}
	return s_last_modrm_ptr;
}

void
modrm_write16( byte rmbyte, word data )
{
	byte *rmp = modrm_resolve( rmbyte, 16 );
	if( MODRM_ISREG( rmbyte ))
	{
		*((word *)rmp) = data;
		return;
	}
	rmp[0] = LSB(data);
	rmp[1] = MSB(data);
}

void
modrm_write8( byte rmbyte, byte data )
{
	*((byte *)modrm_resolve( rmbyte, 8 )) = data;
}

word
modrm_read16( byte rmbyte )
{
	byte *rmp = modrm_resolve( rmbyte, 16 );
	if( MODRM_ISREG( rmbyte ))
		return *((word *)rmp);
	return (rmp[0] | (rmp[1] << 8));
}

byte
modrm_read8( byte rmbyte )
{
	return *((byte *)modrm_resolve( rmbyte, 8 ));
}

void
modrm_update16( byte rmbyte, word data )
{
	byte *rmp = s_last_modrm_ptr;
	if( MODRM_ISREG( rmbyte ))
	{
		*((word *)rmp) = data;
		return;
	}
	rmp[0] = LSB( data );
	rmp[1] = MSB( data );
}

void
modrm_update8( byte rmbyte, byte data )
{
	/* FIXME: We don't really need rmbyte here. But it'd look weird
	 *        with different semantics for update8 and update16.
	 *        Macro it away? */
	(void) rmbyte;
	*((byte *)s_last_modrm_ptr) = data;
}

dword
modrm_read32( byte rmbyte )
{
	byte *rmp = modrm_resolve( rmbyte, 32 );
	if( MODRM_ISREG( rmbyte ))
	{
		vlog( VM_CPUMSG, "PANIC: Attempt to read 32-bit register." );
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
                case 7: retv = cpu.regs.W.BX; break;
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
                case 7: retv = cpu.regs.W.BX + signext(cpu_pfq_getbyte()); break;
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
                case 7: retv = cpu.regs.W.BX + cpu_pfq_getword(); break;
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
