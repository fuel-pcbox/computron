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

#include "vomit.h"
#include "debugger.h"
#include <stdarg.h>
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

    if (g_cpu->debugger()->isActive() || show_on_stdout) {
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
