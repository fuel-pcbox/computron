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

#include "debugger.h"
#include "vomit.h"
#include "debug.h"
#include "vcpu.h"
#include <QDebug>
#include <QStringBuilder>
#include <QStringList>
#include <QLatin1Literal>
#ifdef HAVE_READLINE
#include <readline/readline.h>
#endif

Debugger::Debugger(VCpu* cpu)
    : m_cpu(cpu)
    , m_active(false)
{
}

Debugger::~Debugger()
{
}

void Debugger::enter()
{
    m_active = true;
}

void Debugger::exit()
{
    m_active = false;
}

static QString doPrompt(const VCpu* cpu)
{
    static QString brightMagenta("\033[35;1m");
    static QString brightCyan("\033[34;1m");
    static QString defaultColor("\033[0m");

    QString s;

    if (cpu->getPE())
        s.sprintf("%04X:%08X", cpu->getCS(), cpu->getEIP());
    else
        s.sprintf("%04X:%04X", cpu->getCS(), cpu->getIP());

    QString prompt = brightMagenta % QLatin1Literal("VOMIT ") % brightCyan % s % defaultColor % QLatin1Literal("> ");

#ifdef HAVE_READLINE
    char* line = readline(prompt.toLatin1().constData());
#else
    char* line = (char*)malloc(1024 * sizeof(char));
    fgets(line, 1024, stdin);
#endif

    QString command = line ? QString::fromUtf8(line) : QString::fromLatin1("end-of-file");
    free(line);
    return command;
}

void Debugger::handleCommand(const QString& rawCommand)
{
    QStringList arguments = rawCommand.split(QChar(' '), QString::SkipEmptyParts);

    if (arguments.isEmpty())
        return;

    QString command = arguments.takeFirst();
    QString lowerCommand = command.toLower();

    if (lowerCommand == "q" || lowerCommand == "quit" || lowerCommand == "end-of-file")
        return handleQuit();

    if (lowerCommand == "r" || lowerCommand == "dump-registers")
        return handleDumpRegisters();

    if (lowerCommand == "i" || lowerCommand == "dump-ivt")
        return handleDumpIVT();

    if (lowerCommand == "reconf")
        return handleReconfigure();

    if (lowerCommand == "s" || lowerCommand == "step")
        return handleStep();

    if (lowerCommand == "c" || lowerCommand == "continue")
        return handleContinue();

    if (lowerCommand == "d" || lowerCommand == "dump-memory")
        return handleDumpMemory(arguments);

    if (lowerCommand == "seg")
        return handleDumpSegment(arguments);

    if (lowerCommand == "m")
        return handleDumpFlatMemory(arguments);

    if (lowerCommand == "b")
        return handleBreakpoint(arguments);

    printf("Unknown command: %s\n", command.toUtf8().constData());
}

void Debugger::handleBreakpoint(const QStringList& arguments)
{
    if (arguments.size() != 3) {
        printf("usage: b <add|del> segment:offset\n");
        if (!cpu()->breakpoints().empty()) {
            printf("\nCurrent breakpoints:\n");
            for (auto& breakpoint : cpu()->breakpoints()) {
                printf("    @0x%08X\n", breakpoint);
            }
            printf("\n");
        }
        return;
    }
    WORD segment = arguments.at(1).toUInt(0, 16);
    DWORD offset = arguments.at(2).toUInt(0, 16);
    DWORD flat = vomit_toFlatAddress(segment, offset);
    if (arguments[0] == "add") {
        printf("add breakpoint: %04X:%08X -> @0x%08X\n", segment, offset, flat);
        cpu()->breakpoints().insert(flat);
    }
    if (arguments[0] == "del") {
        printf("delete breakpoint: %04X:%08X -> @0x%08X\n", segment, offset, flat);
        cpu()->breakpoints().erase(flat);
    }
}

void Debugger::doConsole()
{
    VM_ASSERT(isActive());
    VM_ASSERT(cpu());

    while (isActive()) {
        QString rawCommand = doPrompt(cpu());
        handleCommand(rawCommand);
    }


}

void Debugger::handleQuit()
{
    vomit_exit(0);
}

void Debugger::handleDumpRegisters()
{
    cpu()->dumpAll();
}

void Debugger::handleDumpIVT()
{
    cpu()->dumpIVT();
}

void Debugger::handleReconfigure()
{
    // FIXME: Implement.
}

void Debugger::handleStep()
{
    cpu()->exec();
    cpu()->dumpAll();
}

void Debugger::handleContinue()
{
    exit();
}

void Debugger::handleDumpMemory(const QStringList& arguments)
{
    // FIXME: Handle 32-bit offsets.
    WORD segment = cpu()->getCS();
    DWORD offset = cpu()->getEIP() & 0xFFF0;

    if (arguments.size() == 1)
        offset = arguments.at(0).toUInt(0, 16);
    else if (arguments.size() == 2) {
        segment = arguments.at(0).toUInt(0, 16);
        offset = arguments.at(1).toUInt(0, 16);
    }

    cpu()->dumpMemory(segment, offset, 16);
}

void Debugger::handleDumpSegment(const QStringList& arguments)
{
    WORD segment = cpu()->getCS();

    if (arguments.size() >= 1)
        segment = arguments.at(0).toUInt(0, 16);

    cpu()->dumpSegment(segment);
}

void Debugger::handleDumpFlatMemory(const QStringList& arguments)
{
    DWORD address = cpu()->getEIP();

    if (arguments.size() == 1)
        address = arguments.at(0).toUInt(0, 16);

    cpu()->dumpFlatMemory(address);
}
