#include "mainwindow.h"
#include <QApplication>
#include <QFile>
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
    if(args.contains("--mempeek")) options.mempeek = true;
    if(args.contains("--iopeek")) options.iopeek = true;
    if(args.contains("--bda-peek")) options.bda_peek = true;
    if(args.contains("--trace")) options.trace = true;

#ifndef VOMIT_TRACE
    if(options.trace)
    {
        fprintf(stderr, "Rebuild with #define VOMIT_TRACE if you want --trace to work.\n");
        exit(1);
    }
#endif

    g_cpu = new VCpu;
    g_cpu->init();

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

