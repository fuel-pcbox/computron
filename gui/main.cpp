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

#include "mainwindow.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include "screen.h"
#include "vomit.h"
#include "vcpu.h"
#include "debugger.h"
#include "machine.h"
#include "iodevice.h"
#include "settings.h"
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

    signal(SIGINT, sigint_handler);

    QScopedPointer<Machine> machine;

    if (options.file_to_run.length()) {
        machine.reset(Machine::createForAutotest(QString::fromStdString(options.file_to_run)));
    } else {
        machine.reset(Machine::createFromFile(QLatin1String("default.vmf")));
    }

    if (!machine)
        return 1;

    if (options.start_in_debug)
        machine->cpu()->debugger()->enter();

    extern void vomit_disasm_init_tables();
    vomit_disasm_init_tables();

    QFile::remove("log.txt");

    foreach (IODevice *device, IODevice::devices())
        vlog(LogInit, "%s present", device->name());

    if (machine->settings()->isForAutotest()) {
        machine->cpu()->mainLoop();
        return 0;
    }

    MainWindow mainWindow;
    mainWindow.addMachine(machine.data());
    mainWindow.show();

    return app.exec();
}

void parseArguments(const QStringList& arguments)
{
    for (auto it = arguments.begin(); it != arguments.end(); ) {
        const auto& argument = *it;
        if (argument == "--disklog")
            options.disklog = true;
        else if (argument == "--trapint")
            options.trapint = true;
        else if (argument == "--memdebug")
            options.memdebug = true;
        else if (argument == "--vgadebug")
            options.vgadebug = true;
        else if (argument == "--iopeek")
            options.iopeek = true;
        else if (argument == "--trace")
            options.trace = true;
        else if (argument == "--debug")
            options.start_in_debug = true;
        else if (argument == "--run") {
            ++it;
            if (it == arguments.end()) {
                fprintf(stderr, "usage: vomit --run [filename]\n");
                vomit_exit(1);
            }
            options.file_to_run = (*it).toStdString();
            continue;
        }
        ++it;
    }

#ifndef VOMIT_TRACE
    if (options.trace) {
        fprintf(stderr, "Rebuild with #define VOMIT_TRACE if you want --trace to work.\n");
        vomit_exit(1);
    }
#endif
}
