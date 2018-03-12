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
#include "Common.h"
#include "CPU.h"
#include "debugger.h"
#include "machine.h"
#include "iodevice.h"
#include "settings.h"
#include <signal.h>

static void parseArguments(const QStringList& arguments);

RuntimeOptions options;

static void sigint_handler(int)
{
    ASSERT(g_cpu);
    g_cpu->debugger().enter();
}

void hard_exit(int exitCode)
{
    exit(exitCode);
}

int main(int argc, char** argv)
{
    OwnPtr<QCoreApplication> app;

    for (int i = 1; i < argc; ++i) {
        if (QString::fromLatin1(argv[i]) == "--no-gui") {
            app = make<QCoreApplication>(argc, argv);
            break;
        }
    }

    if (!app) {
        app = make<QApplication>(argc, argv);
        QApplication::setWindowIcon(QIcon(":/icons/computron.ico"));
    }

    parseArguments(app->arguments());

    signal(SIGINT, sigint_handler);

    OwnPtr<Machine> machine;

    if (options.autotestPath.length()) {
        machine = Machine::createForAutotest(options.autotestPath);
    } else if (options.configPath.length()) {
        machine = Machine::createFromFile(options.configPath);
    } else {
        machine = Machine::createFromFile(QLatin1String("default.vmf"));
    }

    if (!machine)
        return 1;

    if (options.start_in_debug)
        machine->cpu().debugger().enter();

    QFile::remove("log.txt");

    machine->forEachIODevice([] (IODevice& device) {
        vlog(LogInit, "%s present", device.name());
    });

    if (machine->settings().isForAutotest()) {
        machine->cpu().mainLoop();
        return 0;
    }

    MainWindow mainWindow;
    mainWindow.addMachine(machine.ptr());
    mainWindow.show();
    mainWindow.setFocus();

    return app->exec();
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
        else if (argument == "--vlog-cycle")
            options.vlogcycle = true;
        else if (argument == "--crash-on-gpf")
            options.crashOnGPF = true;
        else if (argument == "--crash-on-exception")
            options.crashOnException = true;
        else if (argument == "--pedebug")
            options.pedebug = true;
        else if (argument == "--vgadebug")
            options.vgadebug = true;
        else if (argument == "--iopeek")
            options.iopeek = true;
        else if (argument == "--trace")
            options.trace = true;
        else if (argument == "--debug")
            options.start_in_debug = true;
        else if (argument == "--no-vlog")
            options.novlog = true;
        else if (argument == "--config") {
            ++it;
            if (it == arguments.end()) {
                fprintf(stderr, "usage: computron --config [filename]\n");
                hard_exit(1);
            }
            options.configPath = (*it);
            continue;
        }
        else if (argument == "--run") {
            ++it;
            if (it == arguments.end()) {
                fprintf(stderr, "usage: computron --run [filename]\n");
                hard_exit(1);
            }
            options.autotestPath = (*it);
            continue;
        }
        ++it;
    }

#ifndef CT_TRACE
    if (options.trace) {
        fprintf(stderr, "Rebuild with #define CT_TRACE if you want --trace to work.\n");
        hard_exit(1);
    }
#endif
}
