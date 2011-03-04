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

static MainWindow *mw = 0L;

int
main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QStringList args = app.arguments();

    memset(&options, 0, sizeof(options));
    if(args.contains("--disklog")) options.disklog = true;
    if(args.contains("--trapint")) options.trapint = true;
    if(args.contains("--iopeek")) options.iopeek = true;
    if(args.contains("--trace")) options.trace = true;
    if(args.contains("--debug")) options.start_in_debug = true;

#ifndef VOMIT_TRACE
    if(options.trace)
    {
        fprintf(stderr, "Rebuild with #define VOMIT_TRACE if you want --trace to work.\n");
        exit(1);
    }
#endif

    qRegisterMetaType<BYTE>("BYTE");
    qRegisterMetaType<WORD>("WORD");
    qRegisterMetaType<DWORD>("DWORD");
    qRegisterMetaType<SIGNED_BYTE>("SIGNED_BYTE");
    qRegisterMetaType<SIGNED_WORD>("SIGNED_WORD");
    qRegisterMetaType<SIGNED_DWORD>("SIGNED_DWORD");

    g_cpu = new VCpu;

    if (options.start_in_debug)
        g_cpu->attachDebugger();

    extern void vomit_disasm_init_tables();
    vomit_disasm_init_tables();

    QFile::remove("log.txt");

    vomit_init();

    mw = new MainWindow(g_cpu);
    mw->show();

    return app.exec();
}

WORD kbd_getc()
{
    if (!mw || !mw->screen())
        return 0x0000;
    return mw->screen()->nextKey();
}

WORD kbd_hit()
{
    if (!mw || !mw->screen())
        return 0x0000;
    return mw->screen()->peekKey();
}

BYTE kbd_pop_raw()
{
    if (!mw || !mw->screen())
        return 0x00;
    return mw->screen()->popKeyData();
}

