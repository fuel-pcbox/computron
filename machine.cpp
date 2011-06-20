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

#include "machine.h"
#include "settings.h"
#include "vcpu.h"
#include "vga_memory.h"
#include "iodevice.h"
#include "pit.h"
#include "worker.h"
#include "screen.h"
#include <QtCore/QFile>

Machine* Machine::createFromFile(const QString& fileName)
{
    Settings* settings = Settings::createFromFile(fileName);

    if (!settings)
        return 0;

    return new Machine(fileName, settings);
}

Machine* Machine::createForAutotest(const QString& fileName)
{
    Settings* settings = Settings::createForAutotest(fileName);

    if (!settings)
        return 0;

    return new Machine(fileName, settings);
}

Machine::Machine(const QString& name, Settings* settings, QObject* parent)
    : QObject(parent)
    , m_name(name)
    , m_cpu(new VCpu(this))
    , m_screen(0)
    , m_settings(settings)
    , m_worker(0)
{
    applySettings();

    cpu()->setBaseMemorySize(640 * 1024);

    // FIXME: Move this somewhere else.
    static const BYTE bootCode[] = { 0xEA, 0x00, 0x00, 0x00, 0xF0 };
    BYTE* entryPoint = cpu()->memoryPointer(0xF000, 0xFFF0);
    VM_ASSERT(entryPoint);
    memcpy(entryPoint, bootCode, sizeof(bootCode));

    // FIXME: Move this somewhere else.
    // Mitigate spam about uninteresting ports.
    IODevice::ignorePort(0x220);
    IODevice::ignorePort(0x221);
    IODevice::ignorePort(0x222);
    IODevice::ignorePort(0x223);
    IODevice::ignorePort(0x201); // Gameport.

    m_vgaMemory = new VGAMemory(this);
    m_screen = new Screen(this);

    // FIXME: Sort out worker ownership.
    m_worker = new Worker(cpu());

    QObject::connect(worker(), SIGNAL(finished()), this, SLOT(onWorkerFinished()));

    PIT::the()->boot();

    worker()->startMachine();
    worker()->start();
}

Machine::~Machine()
{
    delete m_cpu;
    m_cpu = 0;
    delete m_settings;
    m_settings = 0;
}

void Machine::applySettings()
{
    if (!settings())
        return;

    cpu()->setExtendedMemorySize(settings()->memorySize());
    // FIXME: Apply memory-size setting.

    cpu()->setCS(settings()->entryCS());
    cpu()->setIP(settings()->entryIP());

    QHash<DWORD, QString> files = settings()->files();

    QHash<DWORD, QString>::const_iterator it = files.constBegin();
    QHash<DWORD, QString>::const_iterator end = files.constEnd();

    for (; it != end; ++it) {
        if (!loadFile(it.key(), it.value())) {
            // FIXME: Should we abort if a load fails?
        }
    }
}

bool Machine::loadFile(DWORD address, const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        vlog(LogConfig, "Failed to open %s", qPrintable(fileName));
        return false;
    }

    QByteArray fileContents = file.readAll();

    vlog(LogConfig, "Loading %s at 0x%08X", qPrintable(fileName), address);

    BYTE* memoryPointer = cpu()->memoryPointer(address);
    VM_ASSERT(memoryPointer);

    // FIXME: Don't overrun the CPU's memory buffer.
    memcpy(memoryPointer, fileContents.constData(), fileContents.size());
    return true;
}

void Machine::start()
{
    screen()->setTinted(false);
    worker()->startMachine();
}

void Machine::pause()
{
    screen()->setTinted(true);
    worker()->stopMachine();
}

void Machine::stop()
{
    worker()->stopMachine();
}

void Machine::reboot()
{
    // FIXME: Implement.
}

void Machine::onWorkerFinished()
{
    // FIXME: Implement.
}
