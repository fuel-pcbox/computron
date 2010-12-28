// debug.cpp
// MS Debug-like VM interface
// + home of vlog()

#include "vomit.h"
#include "disasm.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <QStringList>

static FILE* s_logfile = 0L;

void vlog(int category, const char* format, ...)
{
    va_list ap;
    const char* prefix = 0L;
    bool show_on_stdout = true;

    switch (category) {
    case VM_INITMSG: prefix = "init"; break;
    case VM_DISKLOG: prefix = "disk"; break;
    case VM_KILLMSG: prefix = "kill"; break;
    case VM_IOMSG:   prefix = "i/o"; show_on_stdout = false; break;
    case VM_ALERT:   prefix = "alert"; break;
    case VM_PRNLOG:  prefix = "lpt"; break;
    case VM_VIDEOMSG: prefix = "video"; break;
    case VM_CONFIGMSG: prefix = "config"; break;
    case VM_CPUMSG:  prefix = "cpu"; break;
    case VM_MEMORYMSG: prefix = "memory"; show_on_stdout = false; break;
    case VM_MOUSEMSG: prefix = "mouse"; break;
    case VM_PICMSG: prefix = "pic"; show_on_stdout = false; break;
    case VM_DMAMSG: prefix = "dma"; show_on_stdout = false; break;
    case VM_KEYMSG: prefix = "keyb"; break;
    case VM_FDCMSG: prefix = "fdc"; break;
    case VM_DUMPMSG: show_on_stdout = false; break;
    case VM_VOMCTL: prefix = "vomctl"; break;
    case VLOG_CMOS: prefix = "cmos"; break;
    }

    if (!s_logfile) {
        s_logfile = fopen("log.txt", "a");
        if (!s_logfile)
            return;
    }

    if (prefix)
        fprintf(s_logfile, "(%8s) ", prefix);

    va_start(ap, format);
    vfprintf(s_logfile, format, ap);
    va_end(ap);

    if (g_cpu->inDebugger() || show_on_stdout) {
        if (prefix)
            printf("(%8s) ", prefix);
        va_start(ap, format);
        vprintf(format, ap);
        va_end(ap);
        puts("");
    }

    fputc('\n', s_logfile);

    fflush(s_logfile);
}

void uasm(WORD segment, WORD offset, int n)
{
    for (int i = 0; i < n; ++i) {
        int w = g_cpu->dumpDisassembled(segment, offset);
        if (!w)
            break;
        offset += w;
    }
}

#ifdef VOMIT_DEBUG
void VCpu::debugger()
{
    char curcmd[256];
    WORD mseg;
    DWORD moff;

    memset(curcmd, 0, sizeof(curcmd));

    m_inDebugger = true;

    if (m_debugOneStep) {
        dumpAll();
        m_debugOneStep = false;
    }

    for (;;) {
        printf("[%u/%u] %04X:%08X> ", a16() ? 16 : 32, o16() ? 16 : 32, getCS(), getEIP());
        fflush(stdout);
        fgets(curcmd, sizeof(curcmd), stdin);
        if (feof(stdin)) {
            vlog(VM_KILLMSG, "EOF on stdin, exiting.");
            vm_exit(0);
        }

        static QRegExp separators("[ \n]");
        QStringList tokens = QString::fromLatin1(curcmd).split(separators, QString::SkipEmptyParts);

        if (tokens.isEmpty())
            continue;

            qDebug() << tokens;

        if (tokens[0] == "g") {
            detachDebugger();
            return;
        }

        if (tokens[0] == "s") {
            m_debugOneStep = true;
            return;
        }

        if (tokens[0] == "c")
            dump();

        else if (tokens[0] == "b") {
            if (tokens.size() == 2) {
                DWORD newBreakPoint = tokens[1].toUInt(0, 16);
                m_breakPoints.append(newBreakPoint);
                printf("Added breakpoint at %08X\n", newBreakPoint);
            } else if (tokens.size() == 1) {
                unsigned int i = 0;
                foreach (DWORD breakPoint, m_breakPoints)
                    printf("[%u]: %08X\n", i++, breakPoint);
            }
        }

        else if (tokens[0] == "reconf")
            config_reload();

        else if (tokens[0] == "r")
            dumpAll();

        else if (tokens[0] == "i")
            dumpIVT();

        else if (tokens[0] == "d") {
            mseg = getCS();
            moff = getEIP() & 0xFFF0;

            if (tokens.size() == 2)
                moff = tokens[1].toUInt(0, 16);
            else if (tokens.size() == 3) {
                mseg = tokens[1].toUInt(0, 16);
                moff = tokens[2].toUInt(0, 16);
            }

            g_cpu->dumpMemory(mseg, moff, 16);
        }

#if 0
        else if (tokens[0] == "u") {
            curtok = strtok(NULL, ": \n");
                if (curtok!=0) {
                    mseg = strtol(curtok,NULL,16);
                    curtok = mseg ? strtok(NULL, " \n") : NULL;
                    if (curtok!=0) {
                        moff = strtol(curtok,NULL,16);
                        uasm(mseg, moff, 16);
                    } else {
                        uasm(getCS(), mseg, 16);
                    }
                } else {
                    uasm(getCS(), getIP(), 16);
                    curtok = &curcmd[0];
                }
            }
#endif

            else if (tokens[0] == "q")
                vm_exit(0);

            else if (tokens[0] == "?") {
                printf( "\tavailable commands:\n\n" \
                        "\tc\t\tcpu information\n" \
                        "\tr\t\tdump registers and stack\n" \
                        "\td [SEG OFF]\tdump memory\n" \
                        "\ti\t\tdump interrupt vector table\n" \
                        "\tg\t\tcontinue execution\n" \
                        "\ts\t\tstep execution\n" \
                        "\tq\t\tabort execution\n" \
                        "\n");
            }
        }
}
#endif
