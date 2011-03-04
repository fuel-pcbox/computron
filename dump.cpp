/*
 * Copyright (C) 2003-2011 Andreas Kling <kling@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ANDREAS KLING ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANDREAS KLING OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include "vomit.h"
#include "debug.h"
#include "disasm.h"

void VCpu::dump() const
{
    vlog(VM_DUMPMSG, "CPU level: %u", VOMIT_CPU_LEVEL);
    vlog(VM_DUMPMSG, "Base mem: %dK", baseMemorySize());
#ifdef VOMIT_PREFETCH_QUEUE
    vlog(VM_DUMPMSG, "Prefetch queue: %d bytes", m_prefetchQueueSize);
#else
    vlog(VM_DUMPMSG, "Prefetch queue: off");
#endif
}

int VCpu::dumpDisassembled(WORD segment, DWORD offset) const
{
    char disasm[64];
    char buf[512];
    char* p = buf;

    BYTE* opcode = memoryPointer(segment, offset);
    int width = insn_width(opcode);
    disassemble(opcode, offset, disasm, sizeof(disasm));

    p += sprintf(p, "%04X:%08X ", segment, offset);

    for (int i = 0; i < (width ? width : 7); ++i)
        p += sprintf(p, "%02X", opcode[i]);

    for (int i = 0; i < (14-((width?width:7)*2)); ++i)
        p += sprintf(p, " ");

    p += sprintf(p, " %s", disasm);

#ifdef VOMIT_TRACE
    if (options.trace)
        fprintf(stderr, "%s\n", buf);
#endif

    vlog(VM_DUMPMSG, buf);

    /* Recurse if this is a prefix instruction. */
    if (*opcode == 0x26 || *opcode == 0x2E || *opcode == 0x36 || *opcode == 0x3E || *opcode == 0xF2 || *opcode == 0xF3)
        width += dumpDisassembled(segment, offset + width);

    return width;
}

#ifdef VOMIT_TRACE
void VCpu::dumpTrace() const
{
    fprintf(stderr,
        "%04X:%08X "
        "EAX=%08X EBX=%08X ECX=%08X EDX=%08X ESP=%08X EBP=%08X ESI=%08X EDI=%08X "
        "CR0=%08X CR1=%08X CR2=%08X CR3=%08X CR4=%08X CR5=%08X CR6=%08X CR7=%08X "
        "DR0=%08X DR1=%08X DR2=%08X DR3=%08X DR4=%08X DR5=%08X DR6=%08X DR7=%08X "
        "DS=%04X ES=%04X SS=%04X FS=%04X GS=%04X "
        "C=%u P=%u A=%u Z=%u S=%u I=%u D=%u O=%u\n",
        getCS(), getEIP(),
        getEAX(), getEBX(), getECX(), getEDX(), getESP(), getEBP(), getESI(), getEDI(),
        getCR0(), getCR1(), getCR2(), getCR3(), getCR4(), getCR5(), getCR6(), getCR7(),
        getDR0(), getDR1(), getDR2(), getDR3(), getDR4(), getDR5(), getDR6(), getDR7(),
        getDS(), getES(), getSS(), getFS(), getGS(),
        getCF(), getPF(), getAF(), getZF(),
        getSF(), getIF(), getDF(), getOF()
    );
}
#endif

void dumpSelector(const VCpu* cpu, const char* segmentRegisterName, int segIndex)
{
    const VCpu::SegmentSelector& selector = cpu->m_selector[segIndex];
    vlog(VM_DUMPMSG, "%s=%04X {%08X:%05X}",
        segmentRegisterName,
        *cpu->tseg[segIndex],
        selector.base,
        selector.limit
    );
}

void VCpu::dumpAll() const
{
    BYTE* csip = codeMemory();

#ifdef VOMIT_PREFETCH_QUEUE
    BYTE x = m_prefetchQueueIndex;
    BYTE dpfq[6];

    for (int i = 0; i < m_prefetchQueueSize; ++i) {
        dpfq[i] = m_prefetchQueue[x++];
        if (x == m_prefetchQueueSize)
            x = 0;
    }
#endif

    vlog(VM_DUMPMSG, "EAX=%08X", getEAX());
    vlog(VM_DUMPMSG, "EBX=%08X", getEBX());
    vlog(VM_DUMPMSG, "ECX=%08X", getECX());
    vlog(VM_DUMPMSG, "EDX=%08X", getEDX());
    vlog(VM_DUMPMSG, "EBP=%08X", getEBP());
    vlog(VM_DUMPMSG, "ESP=%08X", getESP());
    vlog(VM_DUMPMSG, "ESI=%08X", getESI());
    vlog(VM_DUMPMSG, "EDI=%08X", getEDI());


    if (!getPE()) {
        vlog(VM_DUMPMSG, "CS=%04X", getCS());
        vlog(VM_DUMPMSG, "DS=%04X", getDS());
        vlog(VM_DUMPMSG, "ES=%04X", getES());
        vlog(VM_DUMPMSG, "SS=%04X", getSS());
        vlog(VM_DUMPMSG, "FS=%04X", getFS());
        vlog(VM_DUMPMSG, "GS=%04X", getGS());
    } else {
        dumpSelector(this, "CS", RegisterCS);
        dumpSelector(this, "DS", RegisterDS);
        dumpSelector(this, "ES", RegisterES);
        dumpSelector(this, "SS", RegisterSS);
        dumpSelector(this, "FS", RegisterFS);
        dumpSelector(this, "GS", RegisterGS);
    }

    vlog(VM_DUMPMSG, "CR0=%08X", getCR0());

    vlog(VM_DUMPMSG, "GDTR={base=%08X, limit=%04X}", this->GDTR.base, this->GDTR.limit);
    vlog(VM_DUMPMSG, "LDTR={base=%08X, limit=%04X}", this->LDTR.base, this->LDTR.limit);
    vlog(VM_DUMPMSG, "IDTR={base=%08X, limit=%04X}", this->IDTR.base, this->IDTR.limit);

    vlog(VM_DUMPMSG, "C=%u P=%u A=%u Z=%u S=%u I=%u D=%u O=%u", getCF(), getPF(), getAF(), getZF(), getSF(), getIF(), getDF(), getOF());

    vlog(VM_DUMPMSG, "  -  (%02X %02X%02X%02X%02X%02X)", csip[0], csip[1], csip[2], csip[3], csip[4], csip[5]);
#ifdef VOMIT_PREFETCH_QUEUE
    vlog(VM_DUMPMSG, "  -  [%02X %02X%02X%02X%02X%02X]", dpfq[0], dpfq[1], dpfq[2], dpfq[3], dpfq[4], dpfq[5]);
#endif

    dumpDisassembled(getBaseCS(), getBaseEIP());
}

static inline BYTE n(BYTE b)
{
    if (b < 0x20 || ((b > 127) && (b < 160)))
        return '.';
    return b;
}

void VCpu::dumpMemory(WORD segment, DWORD offset, int rows) const
{
    BYTE* p = memoryPointer(segment, offset);

    for (int i = 0; i < rows; ++i) {
        vlog(VM_DUMPMSG,
            "%04X:%04X   %02X %02X %02X %02X %02X %02X %02X %02X - %02X %02X %02X %02X %02X %02X %02X %02X   %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
            segment, (offset+i*16),
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
            p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15],
            n(p[0]), n(p[1]), n(p[2]), n(p[3]), n(p[4]), n(p[5]), n(p[6]), n(p[7]),
            n(p[8]), n(p[9]), n(p[10]), n(p[11]), n(p[12]), n(p[13]), n(p[14]), n(p[15])
        );
        p+=16;
    }
}

static WORD iseg(BYTE isr) { return g_cpu->readMemory16(isr * 4) + 2; }
static WORD ioff(BYTE isr) { return g_cpu->readMemory16(isr * 4); }

void VCpu::dumpIVT() const
{
    // XXX: For alignment reasons, we're skipping INT FF
    for (int i = 0; i < 0xFF; i += 4) {
        vlog(VM_DUMPMSG,
            "%02X>  %04X:%04X\t%02X>  %04X:%04X\t%02X>  %04X:%04X\t%02X>  %04X:%04X",
            i, iseg(i), ioff(i),
            i+1, iseg(i+1), ioff(i+1),
            i+2, iseg(i+2), ioff(i+2),
            i+3, iseg(i+3), ioff(i+3)
        );
    }
}
