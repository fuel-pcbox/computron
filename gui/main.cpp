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

#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QMetaType>
#include "screen.h"
#include "vomit.h"
#include "vcpu.h"
#include "debugger.h"
#include "machine.h"
#include "iodevice.h"
#include <signal.h>

static void parseArguments(const QStringList& arguments);

VomitOptions options;

static void sigint_handler(int)
{
    VM_ASSERT(g_cpu);
    VM_ASSERT(g_cpu->debugger());
    g_cpu->debugger()->enter();
}

void vomit_exit(int exitCode)
{
    exit(exitCode);
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    parseArguments(app.arguments());

    qRegisterMetaType<BYTE>("BYTE");
    qRegisterMetaType<WORD>("WORD");
    qRegisterMetaType<DWORD>("DWORD");
    qRegisterMetaType<SIGNED_BYTE>("SIGNED_BYTE");
    qRegisterMetaType<SIGNED_WORD>("SIGNED_WORD");
    qRegisterMetaType<SIGNED_DWORD>("SIGNED_DWORD");

    signal(SIGINT, sigint_handler);

    QScopedPointer<Machine> machine(Machine::createFromFile(QLatin1String("default.vmf")));

    if (!machine)
        return 1;

    if (options.start_in_debug)
        machine->cpu()->debugger()->enter();

    extern void vomit_disasm_init_tables();
    vomit_disasm_init_tables();

    QFile::remove("log.txt");

    foreach (IODevice *device, IODevice::devices())
        vlog(LogInit, "%s at 0x%p", device->name(), device);

    MainWindow mainWindow;
    mainWindow.addMachine(machine.data());
    mainWindow.show();

    return app.exec();
}

void parseArguments(const QStringList& arguments)
{
    memset(&options, 0, sizeof(options));

    if (arguments.contains("--disklog"))
        options.disklog = true;

    if (arguments.contains("--trapint"))
        options.trapint = true;

    if (arguments.contains("--iopeek"))
        options.iopeek = true;

    if (arguments.contains("--trace"))
        options.trace = true;

    if (arguments.contains("--debug"))
        options.start_in_debug = true;

#ifndef VOMIT_TRACE
    if (options.trace) {
        fprintf(stderr, "Rebuild with #define VOMIT_TRACE if you want --trace to work.\n");
        exit(1);
    }
#endif
}
