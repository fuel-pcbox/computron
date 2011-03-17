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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "vomit.h"
#include "vcpu.h"
#include "debug.h"
#include "debugger.h"
#include "iodevice.h"
#include "vga.h"
#include <signal.h>

vomit_options_t options;

static void sigint_handler(int)
{
    VM_ASSERT(g_cpu);
    VM_ASSERT(g_cpu->debugger());
    g_cpu->debugger()->enter();
}

void vomit_init()
{
    vlog(VM_INITMSG, "Registering I/O devices");
    foreach (IODevice *device, IODevice::devices())
        vlog(VM_INITMSG, "%s at 0x%p", device->name(), device);

    vm_loadconf();

    signal(SIGINT, sigint_handler);
}

void vm_kill()
{
    vlog(VM_KILLMSG, "Killing VM");
    delete g_cpu;
}

void vm_exit(int exit_code)
{
    vm_kill();
    exit(exit_code);
}
