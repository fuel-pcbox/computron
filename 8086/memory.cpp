/* 8086/memory.c
 * Memory Functions
 *
 */

#include <stdlib.h>
#include <string.h>

#include "vomit.h"
#include "debug.h"
#include "vga_memory.h"

BYTE vomit_cpu_memory_read8(VCpu* cpu, WORD segment, WORD offset)
{
    DWORD flat_address = FLAT(segment, offset);

#ifdef VOMIT_DEBUG
    if (mempeek) {
        vlog(VM_MEMORYMSG, "%04X:%04X reading   BYTE at %08X", cpu->base_CS, cpu->base_IP, flat_address);
    }
#endif

    if (flat_address >= 0xA0000 && flat_address < 0xB0000)
		return cpu->vgaMemory->read8(flat_address);

#ifdef VOMIT_DEBUG
    if( options.bda_peek )
    {
        if( flat_address >= 0x400 && flat_address <= 0x4FF )
        {
            vlog( VM_MEMORYMSG, "BDA: read byte at %08X (=%02X)", flat_address, cpu->memory[flat_address] );
        }
    }
#endif

    return cpu->memory[flat_address];
}
WORD vomit_cpu_memory_read16(VCpu* cpu, WORD segment, WORD offset)
{
    DWORD flat_address = FLAT(segment, offset);

#ifdef VOMIT_DEBUG
    if (mempeek) {
        vlog(VM_MEMORYMSG, "%04X:%04X reading   WORD at %08X", cpu->base_CS, cpu->base_IP, flat_address);
    }

    if (options.bda_peek) {
        if (flat_address >= 0x400 && flat_address <= 0x4FF) {
            vlog(VM_MEMORYMSG, "BDA: read word at %08X (=%04X)", flat_address, cpu->memory[flat_address] + (cpu->memory[flat_address + 1]<<8));
        }
    }
#endif

    if (flat_address >= 0xA0000 && flat_address < 0xB0000)
		return cpu->vgaMemory->read16(flat_address);

#if VOMIT_CPU_LEVEL == 0
    // FIXME: Probably broken for VGA read at 0xFFFF ...
    if (offset == 0xFFFF)
        return cpu->memory[flat_address] | (cpu->memory[segment << 4] << 8);
#endif

    WORD w = *((word *)&cpu->memory[flat_address]);
#ifdef VOMIT_BIG_ENDIAN
    w = V_BYTESWAP(w);
#endif
    return w;
}

void vomit_cpu_memory_write8(VCpu* cpu, WORD segment, WORD offset, BYTE value)
{
    DWORD flat_address = FLAT(segment, offset);

#ifdef VOMIT_DEBUG
    if (mempeek) {
        vlog(VM_MEMORYMSG, "%04X:%04X writing   BYTE at %08X", cpu->base_CS, cpu->base_IP, flat_address);
    }

    if (options.bda_peek) {
        if (flat_address >= 0x400 && flat_address <= 0x4FF) {
            vlog(VM_MEMORYMSG, "BDA: write byte at %08X: %02X", flat_address, value);
        }
    }
#endif

    if (flat_address >= 0xA0000 && flat_address < 0xB0000) {
		cpu->vgaMemory->write8(flat_address, value);
    } else {
        cpu->memory[flat_address] = value;
    }
}

void vomit_cpu_memory_write16(VCpu* cpu, WORD segment, WORD offset, WORD value)
{
    DWORD flat_address = FLAT(segment, offset);

#ifdef VOMIT_DEBUG
    if (mempeek) {
        vlog(VM_MEMORYMSG, "%04X:%04X writing   WORD at %08X", cpu->base_CS, cpu->base_IP, flat_address);
    }

    if (options.bda_peek) {
        if (flat_address >= 0x400 && flat_address <= 0x4FF) {
            vlog(VM_MEMORYMSG, "BDA: write WORD at %08X: %04X", flat_address, value);
        }
    }
#endif

    if (flat_address >= 0xA0000 && flat_address < 0xB0000) {
		cpu->vgaMemory->write16(flat_address, value);
    } else {
#if VOMIT_CPU_LEVEL == 0
        // FIXME: Probably broken for VGA read at 0xFFFF ...
        if (offset == 0xFFFF) {
            cpu->memory[flat_address] = BYTE(value);
            cpu->memory[segment << 4] = BYTE(value >> 8);
            return;
        }
#endif

        WORD* wptr = (WORD*)&cpu->memory[flat_address];
#ifdef VOMIT_BIG_ENDIAN
        *wptr = V_BYTESWAP(value);
#else
        *wptr = value;
#endif
    }
}

void VCpu::push(WORD value)
{
    this->regs.W.SP -= 2;
    vomit_cpu_memory_write16(this, this->SS, this->regs.W.SP, value);
}

WORD VCpu::pop()
{
    WORD w = vomit_cpu_memory_read16(this, this->SS, this->regs.W.SP);
    this->regs.W.SP += 2;
    return w;
}

void _LDS_reg16_mem16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    DWORD value = vomit_cpu_modrm_read32(cpu, rm);
    *cpu->treg16[rmreg(rm)] = LSW(value);
    cpu->DS = MSW(value);
}
void _LES_reg16_mem16(VCpu* cpu)
{
    BYTE rm = cpu->fetchOpcodeByte();
    DWORD value = vomit_cpu_modrm_read32(cpu, rm);
    *cpu->treg16[rmreg(rm)] = LSW(value);
    cpu->ES = MSW(value);
}
