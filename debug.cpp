// Computron x86 PC Emulator
// Copyright (C) 2003-2018 Andreas Kling <awesomekling@gmail.com>
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY ANDREAS KLING ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANDREAS KLING OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "debug.h"
#include "Common.h"
#include "debugger.h"
#include "machine.h"
#include "CPU.h"
#include <stdarg.h>
#include <stdio.h>

//#define LOG_TO_FILE

#ifdef LOG_TO_FILE
static FILE* s_logfile = 0L;
#endif

void vlog(VLogChannel channel, const char* format, ...)
{
    if (options.novlog)
        return;

    va_list ap;
    const char* prefix = 0L;
    bool show_on_stdout = true;

    switch (channel) {
    case LogInit: prefix = "init"; break;
    case LogExit: prefix = "exit"; break;
    case LogDisk: prefix = "disk"; break;
    case LogIO: prefix = "i/o"; break;
    case LogAlert: prefix = "alert"; break;
    case LogVGA: prefix = "vga"; break;
    case LogConfig: prefix = "config"; break;
    case LogCPU: prefix = "cpu"; break;
    case LogMouse: prefix = "mouse"; break;
    case LogPIC: prefix = "pic"; show_on_stdout = false; break;
    case LogKeyboard: prefix = "keyb"; break;
    case LogFDC: prefix = "fdc"; break;
    case LogDump: prefix = "dump"; break;
    case LogVomCtl: prefix = "vomctl"; break;
    case LogCMOS: prefix = "cmos"; break;
    case LogIDE: prefix = "ide"; break;
    case LogScreen: prefix = "screen"; break;
    case LogFPU: prefix = "fpu"; break;
    case LogTimer: prefix = "timer"; break;
    default:
        ASSERT_NOT_REACHED();
    }

#ifdef LOG_TO_FILE
    if (!s_logfile) {
        s_logfile = fopen("log.txt", "a");
        if (!s_logfile)
            return;
    }

    if (prefix)
        fprintf(s_logfile, "(%8s) ", prefix);

    if (g_cpu) {
        if (g_cpu->x32())
            fprintf(s_logfile, "[%04X:%08X] ", g_cpu->getBaseCS(), g_cpu->getBaseEIP());
        else
            fprintf(s_logfile, "[%04X:%04X] ", g_cpu->getBaseCS(), g_cpu->getBaseIP());
    }

    va_start(ap, format);
    vfprintf(s_logfile, format, ap);
    va_end(ap);
#endif

    if (!g_cpu || g_cpu->debugger().isActive() || show_on_stdout) {
        if (prefix)
            printf("(\033[33;1m%8s\033[0m) ", prefix);
        if (g_cpu) {
            if (options.vlogcycle)
                printf("%20llu ", g_cpu->cycle());
            if (g_cpu->x32())
                printf("\033[34;1m%04X:%08X\033[0m ", g_cpu->getBaseCS(), g_cpu->getBaseEIP());
            else
                printf("\033[34;1m%04X:%04X\033[0m ", g_cpu->getBaseCS(), g_cpu->getBaseIP());
        }
        va_start(ap, format);
        vprintf(format, ap);
        va_end(ap);
        puts("");
    }

 #ifdef LOG_TO_FILE
    fputc('\n', s_logfile);
    fflush(s_logfile);
#endif
}
