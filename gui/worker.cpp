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

#include "worker.h"
#include <QMutexLocker>
#include <QDebug>
#include "vcpu.h"

extern bool g_vomit_exit_main_loop;
extern bool vomit_reboot;

struct Worker::Private
{
    QMutex mutex;
    bool active;
    VCpu* cpu;
};

Worker::Worker(VCpu *c, QObject *parent)
    : QThread(parent),
      d(new Private)
{
    d->cpu = c;
}

Worker::~Worker()
{
    d->cpu = 0;
    delete d;
    d = 0;
}

void Worker::run()
{
    while (d->active) {
        d->cpu->mainLoop();
        while (!d->active)
            msleep(50);
    }
}

void Worker::shutdown()
{
    stopMachine();
    vomit_exit(0);
}

void Worker::startMachine()
{
    g_vomit_exit_main_loop = false;
    d->active = true;
}

void Worker::stopMachine()
{
    d->active = false;
    g_vomit_exit_main_loop = true;
}

void Worker::rebootMachine()
{
    // :(
}
