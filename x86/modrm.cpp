#include "vomit.h"
#include "vcpu.h"
#include "debug.h"

static inline bool modrmIsRegister(BYTE rm)
{
    return (rm & 0xC0) == 0xC0;
}

#define DEFAULT_TO_SS if (!cpu->hasSegmentPrefix()) { segment = cpu->getSS(); }

static void *s_last_modrm_ptr = 0L;

static word s_last_modrm_segment = 0;
static word s_last_modrm_offset = 0;

void vomit_cpu_modrm_write16(VCpu* cpu, BYTE rmbyte, WORD value)
{
    BYTE* rmp = (BYTE*)vomit_cpu_modrm_resolve16(cpu, rmbyte);
    if (rmp)
        *((WORD*)rmp) = value;
    else
        cpu->writeMemory16(s_last_modrm_segment, s_last_modrm_offset, value);
}

void vomit_cpu_modrm_write8(VCpu* cpu, BYTE rmbyte, BYTE value)
{
    BYTE* rmp = (BYTE*)vomit_cpu_modrm_resolve8(cpu, rmbyte);
    if (rmp)
        *rmp = value;
    else
        cpu->writeMemory8(s_last_modrm_segment, s_last_modrm_offset, value);
}

WORD vomit_cpu_modrm_read16(VCpu* cpu, BYTE rmbyte)
{
    BYTE* rmp = (BYTE*)vomit_cpu_modrm_resolve16(cpu, rmbyte);
    if (rmp)
        return *((WORD*)rmp);
    return cpu->readMemory16(s_last_modrm_segment, s_last_modrm_offset);
}

BYTE vomit_cpu_modrm_read8(VCpu* cpu, BYTE rmbyte)
{
    BYTE* rmp = (BYTE*)vomit_cpu_modrm_resolve8(cpu, rmbyte);
    if (rmp)
        return *rmp;
    return cpu->readMemory8(s_last_modrm_segment, s_last_modrm_offset);
}

void vomit_cpu_modrm_update16(VCpu* cpu, WORD value)
{
    if (s_last_modrm_ptr)
        *((WORD*)s_last_modrm_ptr) = value;
    else
        cpu->writeMemory16(s_last_modrm_segment, s_last_modrm_offset, value);
}

void vomit_cpu_modrm_update8(VCpu* cpu, BYTE value)
{
    if (s_last_modrm_ptr)
        *((BYTE*)s_last_modrm_ptr) = value;
    else
        cpu->writeMemory8(s_last_modrm_segment, s_last_modrm_offset, value);
}

DWORD vomit_cpu_modrm_read32(VCpu* cpu, byte rmbyte)
{
    // NOTE: We don't need modrm_resolve32() at the moment.
    BYTE* rmp = (BYTE*)vomit_cpu_modrm_resolve8(cpu, rmbyte);

    if (rmp) {
        vlog(VM_CPUMSG, "PANIC: Attempt to read 32-bit register.");
        vm_exit(1);
    }

    return cpu->readMemory32(s_last_modrm_segment, s_last_modrm_offset);
}

void *vomit_cpu_modrm_resolve8(VCpu* cpu, BYTE rmbyte)
{
    WORD segment = cpu->currentSegment();
    WORD offset = 0x0000;

    switch (rmbyte & 0xC0) {
        case 0x00:
            switch (rmbyte & 0x07) {
                case 0: offset = cpu->regs.W.BX + cpu->regs.W.SI; break;
                case 1: offset = cpu->regs.W.BX + cpu->regs.W.DI; break;
                case 2: DEFAULT_TO_SS; offset = cpu->regs.W.BP + cpu->regs.W.SI; break;
                case 3: DEFAULT_TO_SS; offset = cpu->regs.W.BP + cpu->regs.W.DI; break;
                case 4: offset = cpu->regs.W.SI; break;
                case 5: offset = cpu->regs.W.DI; break;
                case 6: offset = cpu->fetchOpcodeWord(); break;
                default: offset = cpu->regs.W.BX; break;
            }
            s_last_modrm_segment = segment;
            s_last_modrm_offset = offset;
            s_last_modrm_ptr = 0;
            break;
        case 0x40:
            offset = signext( cpu->fetchOpcodeByte() );
            switch (rmbyte & 0x07) {
                case 0: offset += cpu->regs.W.BX + cpu->regs.W.SI; break;
                case 1: offset += cpu->regs.W.BX + cpu->regs.W.DI; break;
                case 2: DEFAULT_TO_SS; offset += cpu->regs.W.BP + cpu->regs.W.SI; break;
                case 3: DEFAULT_TO_SS; offset += cpu->regs.W.BP + cpu->regs.W.DI; break;
                case 4: offset += cpu->regs.W.SI; break;
                case 5: offset += cpu->regs.W.DI; break;
                case 6: DEFAULT_TO_SS; offset += cpu->regs.W.BP; break;
                default: offset += cpu->regs.W.BX; break;
            }
            s_last_modrm_segment = segment;
            s_last_modrm_offset = offset;
            s_last_modrm_ptr = 0;
            break;
        case 0x80:
            offset = cpu->fetchOpcodeWord();
            switch (rmbyte & 0x07) {
                case 0: offset += cpu->regs.W.BX + cpu->regs.W.SI; break;
                case 1: offset += cpu->regs.W.BX + cpu->regs.W.DI; break;
                case 2: DEFAULT_TO_SS; offset += cpu->regs.W.BP + cpu->regs.W.SI; break;
                case 3: DEFAULT_TO_SS; offset += cpu->regs.W.BP + cpu->regs.W.DI; break;
                case 4: offset += cpu->regs.W.SI; break;
                case 5: offset += cpu->regs.W.DI; break;
                case 6: DEFAULT_TO_SS; offset += cpu->regs.W.BP; break;
                default: offset += cpu->regs.W.BX; break;
            }
            s_last_modrm_segment = segment;
            s_last_modrm_offset = offset;
            s_last_modrm_ptr = 0;
            break;
        case 0xC0:
            switch (rmbyte & 0x07) {
                case 0: s_last_modrm_ptr = &cpu->regs.B.AL; break;
                case 1: s_last_modrm_ptr = &cpu->regs.B.CL; break;
                case 2: s_last_modrm_ptr = &cpu->regs.B.DL; break;
                case 3: s_last_modrm_ptr = &cpu->regs.B.BL; break;
                case 4: s_last_modrm_ptr = &cpu->regs.B.AH; break;
                case 5: s_last_modrm_ptr = &cpu->regs.B.CH; break;
                case 6: s_last_modrm_ptr = &cpu->regs.B.DH; break;
                default: s_last_modrm_ptr = &cpu->regs.B.BH; break;
            }
            break;
    }
    return s_last_modrm_ptr;
}

void * vomit_cpu_modrm_resolve16(VCpu* cpu, BYTE rmbyte)
{
    WORD segment = cpu->currentSegment();
    WORD offset = 0x0000;

    switch (rmbyte & 0xC0) {
        case 0x00:
            switch (rmbyte & 0x07) {
                case 0: offset = cpu->regs.W.BX + cpu->regs.W.SI; break;
                case 1: offset = cpu->regs.W.BX + cpu->regs.W.DI; break;
                case 2: DEFAULT_TO_SS; offset = cpu->regs.W.BP + cpu->regs.W.SI; break;
                case 3: DEFAULT_TO_SS; offset = cpu->regs.W.BP + cpu->regs.W.DI; break;
                case 4: offset = cpu->regs.W.SI; break;
                case 5: offset = cpu->regs.W.DI; break;
                case 6: offset = cpu->fetchOpcodeWord(); break;
                default: offset = cpu->regs.W.BX; break;
            }
            s_last_modrm_segment = segment;
            s_last_modrm_offset = offset;
            s_last_modrm_ptr = 0;
            break;
        case 0x40:
            offset = signext( cpu->fetchOpcodeByte() );
            switch (rmbyte & 0x07) {
                case 0: offset += cpu->regs.W.BX + cpu->regs.W.SI; break;
                case 1: offset += cpu->regs.W.BX + cpu->regs.W.DI; break;
                case 2: DEFAULT_TO_SS; offset += cpu->regs.W.BP + cpu->regs.W.SI; break;
                case 3: DEFAULT_TO_SS; offset += cpu->regs.W.BP + cpu->regs.W.DI; break;
                case 4: offset += cpu->regs.W.SI; break;
                case 5: offset += cpu->regs.W.DI; break;
                case 6: DEFAULT_TO_SS; offset += cpu->regs.W.BP; break;
                default: offset += cpu->regs.W.BX; break;
            }
            s_last_modrm_segment = segment;
            s_last_modrm_offset = offset;
            s_last_modrm_ptr = 0;
            break;
        case 0x80:
            offset = cpu->fetchOpcodeWord();
            switch (rmbyte & 0x07) {
                case 0: offset += cpu->regs.W.BX + cpu->regs.W.SI; break;
                case 1: offset += cpu->regs.W.BX + cpu->regs.W.DI; break;
                case 2: DEFAULT_TO_SS; offset += cpu->regs.W.BP + cpu->regs.W.SI; break;
                case 3: DEFAULT_TO_SS; offset += cpu->regs.W.BP + cpu->regs.W.DI; break;
                case 4: offset += cpu->regs.W.SI; break;
                case 5: offset += cpu->regs.W.DI; break;
                case 6: DEFAULT_TO_SS; offset += cpu->regs.W.BP; break;
                default: offset += cpu->regs.W.BX; break;
            }
            s_last_modrm_segment = segment;
            s_last_modrm_offset = offset;
            s_last_modrm_ptr = 0;
            break;
        case 0xC0:
            switch (rmbyte & 0x07) {
                case 0: s_last_modrm_ptr = &cpu->regs.W.AX; break;
                case 1: s_last_modrm_ptr = &cpu->regs.W.CX; break;
                case 2: s_last_modrm_ptr = &cpu->regs.W.DX; break;
                case 3: s_last_modrm_ptr = &cpu->regs.W.BX; break;
                case 4: s_last_modrm_ptr = &cpu->regs.W.SP; break;
                case 5: s_last_modrm_ptr = &cpu->regs.W.BP; break;
                case 6: s_last_modrm_ptr = &cpu->regs.W.SI; break;
                default: s_last_modrm_ptr = &cpu->regs.W.DI; break;
            }
            break;
    }
    return s_last_modrm_ptr;
}
