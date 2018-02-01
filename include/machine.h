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
    virtual ~Machine();
    static Machine* createFromFile(const QString& fileName);
    static Machine* createForAutotest(const QString& fileName);

    QString name() const { return m_name; }
    VCpu& cpu() const { return *m_cpu; }
    VGA& vga() const { return *m_vga; }
    PIT& pit() const { return *m_pit; }
    BusMouse& busMouse() const { return *m_busMouse; }
    Keyboard& keyboard() const { return *m_keyboard; }
    VomCtl& vomCtl() const { return *m_vomCtl; }
    PIC& masterPIC() const { return *m_masterPIC; }
    PIC& slavePIC() const { return *m_slavePIC; }
    VGAMemory& vgaMemory() const { return *m_vgaMemory; }
    Screen& screen() const { return *m_screen; }
    Settings& settings() const { return *m_settings; }

public slots:
    void start();
    void stop();
    void pause();
    void reboot();

private slots:
    void onWorkerFinished();

private:
    explicit Machine(const QString& name, Settings*, QObject* parent = 0);
    bool loadFile(DWORD address, const QString& fileName);

    void applySettings();

    Worker& worker() const { return *m_worker; }

    QString m_name;
    Settings* m_settings { nullptr };
    VCpu* m_cpu { nullptr };
    Screen* m_screen { nullptr };
    VGAMemory* m_vgaMemory { nullptr };
    Worker* m_worker { nullptr };
    VGA* m_vga { nullptr };
    PIT* m_pit { nullptr };
    BusMouse* m_busMouse { nullptr };
    CMOS* m_cmos { nullptr };
    FDC* m_fdc { nullptr };
    IDE* m_ide { nullptr };
    Keyboard* m_keyboard { nullptr };
    PIC* m_masterPIC { nullptr };
    PIC* m_slavePIC { nullptr };
    PS2* m_ps2 { nullptr };
    VomCtl* m_vomCtl { nullptr };
};

#endif
