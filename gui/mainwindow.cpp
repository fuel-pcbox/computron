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

#include "mainwindow.h"
#include "machinewidget.h"
#include "machine.h"
#include "screen.h"
#include "keyboard.h"
#include <QLabel>
#include <QStatusBar>

struct MainWindow::Private
{
    Keyboard* keyboard;
    QStatusBar* statusBar;
    QLabel* scrollLockLabel;
    QLabel* numLockLabel;
    QLabel* capsLockLabel;
};

MainWindow::MainWindow()
    : d(make<Private>())
{
    setWindowTitle("Computron");
}

MainWindow::~MainWindow()
{
}

void MainWindow::addMachine(Machine* machine)
{
    MachineWidget* machineWidget = new MachineWidget(*machine);
    setCentralWidget(machineWidget);
    setFocusProxy(machineWidget);

    d->keyboard = &machine->keyboard();

    d->statusBar = new QStatusBar;
    setStatusBar(d->statusBar);

    d->scrollLockLabel = new QLabel("SCRL");
    d->numLockLabel = new QLabel("NUM");
    d->capsLockLabel = new QLabel("CAPS");

    d->scrollLockLabel->setAutoFillBackground(true);
    d->numLockLabel->setAutoFillBackground(true);
    d->capsLockLabel->setAutoFillBackground(true);

    onLedsChanged(0);

    connect(d->keyboard, SIGNAL(ledsChanged(int)), this, SLOT(onLedsChanged(int)), Qt::QueuedConnection);

    d->statusBar->addWidget(new QLabel, 1);
    d->statusBar->addWidget(d->capsLockLabel);
    d->statusBar->addWidget(d->numLockLabel);
    d->statusBar->addWidget(d->scrollLockLabel);
}

void MainWindow::onLedsChanged(int leds)
{
    QPalette paletteForLED[2];
    paletteForLED[0] = d->scrollLockLabel->palette();
    paletteForLED[1] = d->scrollLockLabel->palette();
    paletteForLED[0].setColor(d->scrollLockLabel->backgroundRole(), Qt::gray);
    paletteForLED[1].setColor(d->scrollLockLabel->backgroundRole(), Qt::green);

    bool scrollLock = leds & Keyboard::LED::ScrollLock;
    bool numLock = leds & Keyboard::LED::NumLock;
    bool capsLock = leds & Keyboard::LED::CapsLock;

    d->scrollLockLabel->setPalette(paletteForLED[scrollLock]);
    d->numLockLabel->setPalette(paletteForLED[numLock]);
    d->capsLockLabel->setPalette(paletteForLED[capsLock]);
}
