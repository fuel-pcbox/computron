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

#ifndef MACHINE_H
#define MACHINE_H

#include <QObject>
#include "types.h"
#include "OwnPtr.h"

class BusMouse;
class CMOS;
class FDC;
class IDE;
class Keyboard;
class PIC;
class PIT;
class PS2;
class Screen;
class Settings;
class VCpu;
class VGA;
class VGAMemory;
class VomCtl;
class Worker;

class Machine : public QObject
{
    Q_OBJECT
public:
    static OwnPtr<Machine> createFromFile(const QString& fileName);
    static OwnPtr<Machine> createForAutotest(const QString& fileName);

    explicit Machine(const QString& name, OwnPtr<Settings>&&, QObject* parent = nullptr);
    virtual ~Machine();

    QString name() const { return m_name; }
    VCpu& cpu() { return *m_cpu; }
    VGA& vga() { return *m_vga; }
    PIT& pit() { return *m_pit; }
    BusMouse& busMouse() { return *m_busMouse; }
    Keyboard& keyboard() { return *m_keyboard; }
    VomCtl& vomCtl() { return *m_vomCtl; }
    PIC& masterPIC() { return *m_masterPIC; }
    PIC& slavePIC() { return *m_slavePIC; }
    VGAMemory& vgaMemory() { return *m_vgaMemory; }
    Screen& screen() { return *m_screen; }
    Settings& settings() { return *m_settings; }

public slots:
    void start();
    void stop();
    void pause();
    void reboot();

private slots:
    void onWorkerFinished();

private:
    bool loadFile(DWORD address, const QString& fileName);

    void applySettings();

    Worker& worker() { return *m_worker; }

    QString m_name;
    OwnPtr<Settings> m_settings;
    OwnPtr<VCpu> m_cpu;
    OwnPtr<Screen> m_screen;
    OwnPtr<Worker> m_worker;

    // Memory mappers
    OwnPtr<VGAMemory> m_vgaMemory;

    // IODevices
    OwnPtr<VGA> m_vga;
    OwnPtr<PIT> m_pit;
    OwnPtr<BusMouse> m_busMouse;
    OwnPtr<CMOS> m_cmos;
    OwnPtr<FDC> m_fdc;
    OwnPtr<IDE> m_ide;
    OwnPtr<Keyboard> m_keyboard;
    OwnPtr<PIC> m_masterPIC;
    OwnPtr<PIC> m_slavePIC;
    OwnPtr<PS2> m_ps2;
    OwnPtr<VomCtl> m_vomCtl;
};

#endif
