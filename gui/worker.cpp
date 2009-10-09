#include "worker.h"
#include <QMutexLocker>
#include <QDebug>
#include "../include/vomit.h"
extern unsigned int g_vomit_exit_main_loop;
extern bool vomit_reboot;

struct Worker::Private
{
    QMutex mutex;
    bool active;
    VCpu *cpu;
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
        vomit_cpu_main(d->cpu);
        while (!d->active)
            msleep(50);
    }
}

void Worker::shutdown()
{
    stopMachine();
    vm_exit(0);
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
