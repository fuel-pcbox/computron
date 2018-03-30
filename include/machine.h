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

#pragma once

#include <functional>
#include <QObject>
#include "types.h"
#include "OwnPtr.h"
#include "Common.h"
#include "ROM.h"
#include <QHash>
#include <QSet>

class IODevice;
class BusMouse;
class CMOS;
class FDC;
class IDE;
class Keyboard;
class PIC;
class PIT;
class PS2;
class Settings;
class CPU;
class VGA;
class VGAMemory;
class VomCtl;
class Worker;
class MachineWidget;

class IODevicePass {
    friend IODevice;
    IODevicePass() { }
};

class Machine : public QObject
{
    Q_OBJECT
public:
    static OwnPtr<Machine> createFromFile(const QString& fileName);
    static OwnPtr<Machine> createForAutotest(const QString& fileName);

    explicit Machine(const QString& name, OwnPtr<Settings>&&, QObject* parent = nullptr);
    virtual ~Machine();

    QString name() const { return m_name; }
    CPU& cpu() { return *m_cpu; }
    VGA& vga() { return *m_vga; }
    PIT& pit() { return *m_pit; }
    BusMouse& busMouse() { return *m_busMouse; }
    Keyboard& keyboard() { return *m_keyboard; }
    VomCtl& vomCtl() { return *m_vomCtl; }
    PIC& masterPIC() { return *m_masterPIC; }
    PIC& slavePIC() { return *m_slavePIC; }
    CMOS& cmos() { return *m_cmos; }
    Settings& settings() { return *m_settings; }
    VGAMemory& vgaMemory() { return *m_vgaMemory; }

    bool isForAutotest() PURE;

    MachineWidget* widget() { return m_widget; }
    void setWidget(MachineWidget* widget) { m_widget = widget; }

    void resetAllIODevices();
    void notifyScreen();

    void forEachIODevice(std::function<void(IODevice&)>);

    IODevice* inputDeviceForPort(WORD port);
    IODevice* outputDeviceForPort(WORD port);

    void registerInputDevice(IODevicePass, WORD port, IODevice&);
    void registerOutputDevice(IODevicePass, WORD port, IODevice&);
    void registerDevice(IODevicePass, IODevice&);
    void unregisterDevice(IODevicePass, IODevice&);

public slots:
    void start();
    void stop();
    void pause();
    void reboot();

private slots:
    void onWorkerFinished();

private:
    bool loadFile(DWORD address, const QString& fileName);
    bool loadROMImage(DWORD address, const QString& fileName);

    void applySettings();

    Worker& worker() { return *m_worker; }

    IODevice* inputDeviceForPortSlowCase(WORD port);
    IODevice* outputDeviceForPortSlowCase(WORD port);

    QString m_name;
    OwnPtr<Settings> m_settings;
    OwnPtr<CPU> m_cpu;
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

    MachineWidget* m_widget { nullptr };

    QSet<IODevice*> m_allDevices;

    IODevice* m_fastInputDevices[1024];
    IODevice* m_fastOutputDevices[1024];

    QHash<WORD, IODevice*> m_allInputDevices;
    QHash<WORD, IODevice*> m_allOutputDevices;

    QVector<ROM*> m_roms;
};

inline IODevice* Machine::inputDeviceForPort(WORD port)
{
    if (port < 1024)
        return m_fastInputDevices[port];
    return inputDeviceForPortSlowCase(port);
}

inline IODevice* Machine::outputDeviceForPort(WORD port)
{
    if (port < 1024)
        return m_fastOutputDevices[port];
    return outputDeviceForPortSlowCase(port);
}
