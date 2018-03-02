/*
 * Copyright (C) 2003-2018 Andreas Kling <awesomekling@gmail.com>
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
#include "Common.h"
#include "CPU.h"
#include "debug.h"

int CPU::dumpDisassembled(SegmentDescriptor& descriptor, DWORD offset)
{
    char buf[512];
    char* p = buf;

    BYTE* data = memoryPointer(descriptor, offset);

    if (!data) {
        vlog(LogCPU, "dumpDisassembled can't dump %04x:%08x", descriptor.index(), offset);
        return 0;
    }

    SimpleInstructionStream stream(data, a32(), o32());
    auto insn = Instruction::fromStream(stream);

    if (x32())
        p += sprintf(p, "0x%04x:0x%08x ", descriptor.index(), offset);
    else
        p += sprintf(p, "0x%04x:0x%04x ", descriptor.index(), offset);

    for (unsigned i = 0; i < insn.length(); ++i)
        p += sprintf(p, "%02x", data[i]);

    for (unsigned i = 0; i < (14-(insn.length()*2)); ++i)
        p += sprintf(p, " ");

    p += sprintf(p, " %s", qPrintable(insn.toString(offset, x32())));

    vlog(LogDump, buf);

    /* Recurse if this is a prefix instruction. */
    if (insn.op() == 0x26 || insn.op() == 0x2E || insn.op() == 0x36 || insn.op() == 0x3E || insn.op() == 0xF2 || insn.op() == 0xF3)
        dumpDisassembled(descriptor, offset + insn.length());

    return 0;
}

int CPU::dumpDisassembled(WORD segment, DWORD offset)
{
    auto descriptor = getSegmentDescriptor(segment);
    return dumpDisassembled(descriptor, offset);
}

#ifdef CT_TRACE
void CPU::dumpTrace()
{
#if 0
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
#else
    printf(
        "%04X:%08X %02X "
        "EAX=%08X EBX=%08X ECX=%08X EDX=%08X ESP=%08X EBP=%08X ESI=%08X EDI=%08X "
        "CR0=%08X A20=%u "
        "DS=%04X ES=%04X SS=%04X FS=%04X GS=%04X "
        "C=%u P=%u A=%u Z=%u S=%u I=%u D=%u O=%u "
        "A%u O%u X%u\n",
        getCS(), getEIP(),
        codeMemory()[getEIP()],
        getEAX(), getEBX(), getECX(), getEDX(), getESP(), getEBP(), getESI(), getEDI(),
        getCR0(),
        isA20Enabled(),
        getDS(), getES(), getSS(), getFS(), getGS(),
        getCF(), getPF(), getAF(), getZF(),
        getSF(), getIF(), getDF(), getOF(),
        a16() ? 16 : 32,
        o16() ? 16 : 32,
        x16() ? 16 : 32
    );
#endif
}
#endif

void CPU::dumpSelector(const char* segmentRegisterName, SegmentRegisterIndex segmentIndex)
{
    auto& descriptor = cachedDescriptor(segmentIndex);
    vlog(LogDump, "%s:", segmentRegisterName);
    dumpDescriptor(descriptor);
}

const char* CPU::registerName(SegmentRegisterIndex index)
{
    switch (index) {
    case SegmentRegisterIndex::CS:
        return "cs";
    case SegmentRegisterIndex::DS:
        return "ds";
    case SegmentRegisterIndex::ES:
        return "es";
    case SegmentRegisterIndex::SS:
        return "ss";
    case SegmentRegisterIndex::FS:
        return "fs";
    case SegmentRegisterIndex::GS:
        return "gs";
    default:
        ASSERT_NOT_REACHED();
        return nullptr;
    }
}

const char* CPU::registerName(CPU::RegisterIndex8 registerIndex)
{
    switch (registerIndex) {
    case CPU::RegisterAL:
        return "al";
    case CPU::RegisterBL:
        return "bl";
    case CPU::RegisterCL:
        return "cl";
    case CPU::RegisterDL:
        return "dl";
    case CPU::RegisterAH:
        return "ah";
    case CPU::RegisterBH:
        return "bh";
    case CPU::RegisterCH:
        return "ch";
    case CPU::RegisterDH:
        return "dh";
    }
    ASSERT_NOT_REACHED();
    return nullptr;
}

const char* CPU::registerName(CPU::RegisterIndex16 registerIndex)
{
    switch (registerIndex) {
    case CPU::RegisterAX:
        return "ax";
    case CPU::RegisterBX:
        return "bx";
    case CPU::RegisterCX:
        return "cx";
    case CPU::RegisterDX:
        return "dx";
    case CPU::RegisterBP:
        return "bp";
    case CPU::RegisterSP:
        return "sp";
    case CPU::RegisterSI:
        return "si";
    case CPU::RegisterDI:
        return "di";
    }
    ASSERT_NOT_REACHED();
    return nullptr;
}

const char* CPU::registerName(CPU::RegisterIndex32 registerIndex)
{
    switch (registerIndex) {
    case CPU::RegisterEAX:
        return "eax";
    case CPU::RegisterEBX:
        return "ebx";
    case CPU::RegisterECX:
        return "ecx";
    case CPU::RegisterEDX:
        return "edx";
    case CPU::RegisterEBP:
        return "ebp";
    case CPU::RegisterESP:
        return "esp";
    case CPU::RegisterESI:
        return "esi";
    case CPU::RegisterEDI:
        return "edi";
    }
    ASSERT_NOT_REACHED();
    return nullptr;
}

void CPU::dumpWatches()
{
    for (WatchedAddress& watch : m_watches) {
        if (watch.size == ByteSize) {
            BYTE data = readUnmappedMemory8(watch.address);
            if (data != watch.lastSeenValue) {
                vlog(LogDump, "\033[32;1m%08X\033[0m [%-16s] %02X", watch.address, qPrintable(watch.name), data);
                watch.lastSeenValue = data;
            }
        } else if (watch.size == WordSize) {
            WORD data = readUnmappedMemory16(watch.address);
            if (data != watch.lastSeenValue) {
                vlog(LogDump, "\033[32;1m%08X\033[0m [%-16s] %04X", watch.address, qPrintable(watch.name), data);
                watch.lastSeenValue = data;
            }
        } else if (watch.size == DWordSize) {
            DWORD data = readUnmappedMemory32(watch.address);
            if (data != watch.lastSeenValue) {
                vlog(LogDump, "\033[32;1m%08X\033[0m [%-16s] %08X", watch.address, qPrintable(watch.name), data);
                watch.lastSeenValue = data;
            }
        }
    }
}

void CPU::dumpAll()
{
    BYTE* csip = codeMemory();

    auto dumpRegister = [this](CPU::RegisterIndex16 registerIndex)
    {
        if (getPE())
            vlog(LogDump, "E%s: %08X", CPU::registerName(registerIndex), getRegister32(static_cast<CPU::RegisterIndex32>(registerIndex)));
        else
            vlog(LogDump, "%s: %04X", CPU::registerName(registerIndex), getRegister16(registerIndex));
    };

    dumpRegister(RegisterAX);
    dumpRegister(RegisterBX);
    dumpRegister(RegisterCX);
    dumpRegister(RegisterDX);
    dumpRegister(RegisterBP);
    dumpRegister(RegisterSP);
    dumpRegister(RegisterSI);
    dumpRegister(RegisterDI);

    if (!getPE()) {
        vlog(LogDump, "CS: %04X", getCS());
        vlog(LogDump, "DS: %04X", getDS());
        vlog(LogDump, "ES: %04X", getES());
        vlog(LogDump, "SS: %04X", getSS());
        vlog(LogDump, "FS: %04X", getFS());
        vlog(LogDump, "GS: %04X", getGS());
    } else {
        dumpSelector("CS", SegmentRegisterIndex::CS);
        dumpSelector("DS", SegmentRegisterIndex::DS);
        dumpSelector("ES", SegmentRegisterIndex::ES);
        dumpSelector("SS", SegmentRegisterIndex::SS);
        dumpSelector("FS", SegmentRegisterIndex::FS);
        dumpSelector("GS", SegmentRegisterIndex::GS);
    }
    vlog(LogDump, "EIP: %08X", getEIP());

    vlog(LogDump, "CR0: %08X", getCR0());
    vlog(LogDump, "CR3: %08X", getCR3());

    vlog(LogDump, "A20: %u", isA20Enabled());

    vlog(LogDump, "GDTR: {base=%08X, limit=%04X}", this->GDTR.base, this->GDTR.limit);
    vlog(LogDump, "LDTR: {base=%08X, limit=%04X}", this->LDTR.base, this->LDTR.limit);
    vlog(LogDump, "IDTR: {base=%08X, limit=%04X}", this->IDTR.base, this->IDTR.limit);

    vlog(LogDump, "C=%u P=%u A=%u Z=%u S=%u I=%u D=%u O=%u", getCF(), getPF(), getAF(), getZF(), getSF(), getIF(), getDF(), getOF());

    vlog(LogDump, "  -  (%02X %02X%02X%02X%02X%02X)", csip[0], csip[1], csip[2], csip[3], csip[4], csip[5]);

    dumpDisassembled(cachedDescriptor(SegmentRegisterIndex::CS), getBaseEIP());

    dumpMemory(cachedDescriptor(SegmentRegisterIndex::CS), getBaseEIP(), 4);

}

static inline BYTE n(BYTE b)
{
    if (b < 0x20 || ((b > 127) && (b < 160)))
        return '.';
    return b;
}

void CPU::dumpFlatMemory(DWORD address)
{
    address &= 0xFFFFFFF0;

    BYTE* p = &m_memory[address];
    int rows = 16;

    for (int i = 0; i < rows; ++i) {
        vlog(LogDump,
            "%08X   %02X %02X %02X %02X %02X %02X %02X %02X - %02X %02X %02X %02X %02X %02X %02X %02X   %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
            (address+i*16),
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
            p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15],
            n(p[0]), n(p[1]), n(p[2]), n(p[3]), n(p[4]), n(p[5]), n(p[6]), n(p[7]),
            n(p[8]), n(p[9]), n(p[10]), n(p[11]), n(p[12]), n(p[13]), n(p[14]), n(p[15])
        );
        p+=16;
    }

    p = &m_memory[address];
    for (int i = 0; i < rows; ++i) {
        fprintf(stderr,
            "db 0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X\n",
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
            p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]
        );
        p+=16;
    }
}

void CPU::dumpRawMemory(BYTE* p)
{
    int rows = 16;
    vlog(LogDump, "Raw dump %p", p);
    for (int i = 0; i < rows; ++i) {
        fprintf(stderr,
            "db 0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X\n",
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
            p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]
        );
        p+=16;
    }
}

void CPU::dumpMemory(SegmentDescriptor& descriptor, DWORD offset, int rows)
{
    offset &= 0xFFFFFFF0;

    BYTE* p = memoryPointer(descriptor, offset);
    if (!p) {
        vlog(LogCPU, "dumpMemory can't dump %04X:%08X", descriptor.index(), offset);
        vlog(LogCPU, "Trying flat dump @ %08X...", offset);
        dumpFlatMemory(offset);
        return;
    }

    for (int i = 0; i < rows; ++i) {
        vlog(LogDump,
            "%04x:%04x   %02X %02X %02X %02X %02X %02X %02X %02X - %02X %02X %02X %02X %02X %02X %02X %02X   %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
            descriptor.index(), (offset+i*16),
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
            p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15],
            n(p[0]), n(p[1]), n(p[2]), n(p[3]), n(p[4]), n(p[5]), n(p[6]), n(p[7]),
            n(p[8]), n(p[9]), n(p[10]), n(p[11]), n(p[12]), n(p[13]), n(p[14]), n(p[15])
        );
        p+=16;
    }

    p = memoryPointer(descriptor, offset);
    for (int i = 0; i < rows; ++i) {
        fprintf(stderr,
            "db 0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X\n",
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
            p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]
        );
        p+=16;
    }
}

void CPU::dumpMemory(WORD segment, DWORD offset, int rows)
{
    auto descriptor = getSegmentDescriptor(segment);
    return dumpMemory(descriptor, offset, rows);
}

static inline WORD isrSegment(const CPU& cpu, BYTE isr)
{
    return cpu.readUnmappedMemory16(isr * 4) + 2;
}

static inline WORD isrOffset(const CPU& cpu, BYTE isr)
{
    return cpu.readUnmappedMemory16(isr * 4);
}

void CPU::dumpIVT()
{
    // XXX: For alignment reasons, we're skipping INT FF
    for (int i = 0; i < 0xFF; i += 4) {
        vlog(LogDump,
            "%02X>  %04X:%04X\t%02X>  %04X:%04X\t%02X>  %04X:%04X\t%02X>  %04X:%04X",
            i, isrSegment(*this, i), isrOffset(*this, i),
            i + 1, isrSegment(*this, i + 1), isrOffset(*this, i + 1),
            i + 2, isrSegment(*this, i + 2), isrOffset(*this, i + 2),
            i + 3, isrSegment(*this, i + 3), isrOffset(*this, i + 3)
        );
    }
}

void CPU::dumpDescriptor(const Descriptor& descriptor)
{
    if (descriptor.isSegmentDescriptor())
        dumpDescriptor(descriptor.asSegmentDescriptor());
    else
        dumpDescriptor(descriptor.asSystemDescriptor());
}

void CPU::dumpDescriptor(const SegmentDescriptor& descriptor)
{
    if (descriptor.isCode())
        dumpDescriptor(descriptor.asCodeSegmentDescriptor());
    else
        dumpDescriptor(descriptor.asDataSegmentDescriptor());
}

void CPU::dumpDescriptor(const Gate& gate)
{
    vlog(LogCPU, "System segment %04x: { type: %s (%02x), entry:%04x:%06x, paramCount:%u, bits:%u, P:%s, DPL:%u }",
        gate.index(),
        gate.typeName(),
        (BYTE)gate.type(),
        gate.selector(),
        gate.offset(),
        gate.parameterCount(),
        gate.D() ? 32 : 16,
        gate.present() ? "yes" : "no",
        gate.DPL()
    );
}

void CPU::dumpDescriptor(const SystemDescriptor& segment)
{
    if (segment.isGate()) {
        dumpDescriptor(segment.asGate());
        return;
    }
    vlog(LogCPU, "System segment %04x: { type: %s (%02x), bits:%u, P:%s, DPL:%u }",
        segment.index(),
        segment.typeName(),
        (BYTE)segment.type(),
        segment.D() ? 32 : 16,
        segment.present() ? "yes" : "no",
        segment.DPL()
    );
}

void CPU::dumpDescriptor(const CodeSegmentDescriptor& segment)
{
    vlog(LogCPU, "%s segment %04x: { type: CODE, base:%08x, limit:%06x, bits:%u, P:%s, G:%s, DPL:%u, A:%s, readable:%s, conforming:%s }",
        segment.isGlobal() ? "Global" : "Local",
        segment.index(),
        segment.base(),
        segment.limit(),
        segment.D() ? 32 : 16,
        segment.present() ? "yes" : "no",
        segment.granularity() ? "4K" : "1b",
        segment.DPL(),
        segment.accessed() ? "yes" : "no",
        segment.conforming() ? "yes" : "no",
        segment.readable() ? "yes" : "no"
    );
}

void CPU::dumpDescriptor(const DataSegmentDescriptor& segment)
{
    vlog(LogCPU, "%s segment %04x: { type: DATA, base:%08x, limit:%06x, bits:%u, P:%s, G:%s, DPL:%u, A:%s, writable:%s, expandDown:%s }",
        segment.isGlobal() ? "Global" : "Local",
        segment.index(),
        segment.base(),
        segment.limit(),
        segment.D() ? 32 : 16,
        segment.present() ? "yes" : "no",
        segment.granularity() ? "4K" : "1b",
        segment.DPL(),
        segment.accessed() ? "yes" : "no",
        segment.writable() ? "yes" : "no",
        segment.expandDown() ? "yes" : "no"
    );
}

void CPU::dumpSegment(WORD index)
{
    dumpDescriptor(getDescriptor(index));
}

