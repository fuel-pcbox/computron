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

#include "mainwindow.h"
#include "machine.h"
#include "worker.h"
#include "screen.h"
#include "debug.h"
#include <QtGui/QToolBar>
#include <QtGui/QAction>
#include <QtGui/QFileDialog>
#include <QtGui/QVBoxLayout>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtCore/QDebug>

void vomit_set_drive_image(int drive_id, const char *filename);

struct MainWindow::Private
{
    QToolBar *mainToolBar;

    QAction *startMachine;
    QAction *pauseMachine;
    QAction *stopMachine;

    Worker *worker;
    Screen *screen;

    QTimer syncTimer;

    Machine* machine;
};

MainWindow::MainWindow(Machine* m)
    : d(new Private)
{
    d->machine = m;

    d->screen = new Screen(machine()->cpu());
    d->worker = new Worker(machine()->cpu(), this);

    QObject::connect(d->worker, SIGNAL(finished()), this, SLOT(close()));
    d->worker->startMachine();
    d->worker->start();

    setWindowTitle("Vomit");
    setUnifiedTitleAndToolBarOnMac(true);
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QWidget *widget = new QWidget(this);

    QVBoxLayout *l = new QVBoxLayout;
    l->setSpacing(0);
    l->setMargin(0);
    widget->setLayout(l);
    l->addWidget(d->screen);

    d->screen->setFocus();

    setCentralWidget(widget);

    QAction *chooseFloppyAImage = new QAction(QIcon(":/icons/disk.png"), tr("Floppy A:"), this);
    QAction *chooseFloppyBImage = new QAction(QIcon(":/icons/disk.png"), tr("Floppy B:"), this);
    d->pauseMachine = new QAction(QIcon(":/icons/control_pause.png"), tr("Pause"), this);
    d->startMachine = new QAction(QIcon(":/icons/control_play.png"), tr("Start"), this);
    d->stopMachine = new QAction(QIcon(":/icons/control_stop.png"), tr("Stop"), this);

    QAction *rebootMachine = new QAction(QIcon(":/icons/arrow_refresh.png"), tr("Reboot"), this);

    d->startMachine->setEnabled(false);
    d->pauseMachine->setEnabled(true);
    d->stopMachine->setEnabled(true);

    d->mainToolBar = addToolBar(tr("Virtual Machine"));

    d->mainToolBar->addAction(d->startMachine);
    d->mainToolBar->addAction(d->pauseMachine);
    d->mainToolBar->addAction(d->stopMachine);
    d->mainToolBar->addAction(rebootMachine);

    d->mainToolBar->addSeparator();

    d->mainToolBar->addAction(chooseFloppyAImage);
    d->mainToolBar->addAction(chooseFloppyBImage);

    connect(chooseFloppyAImage, SIGNAL(triggered(bool)), SLOT(slotFloppyAClicked()));
    connect(chooseFloppyBImage, SIGNAL(triggered(bool)), SLOT(slotFloppyBClicked()));

    connect(rebootMachine, SIGNAL(triggered(bool)), SLOT(slotRebootMachine()));

    connect(d->pauseMachine, SIGNAL(triggered(bool)), SLOT(slotPauseMachine()));
    connect(d->startMachine, SIGNAL(triggered(bool)), SLOT(slotStartMachine()));
    connect(d->stopMachine, SIGNAL(triggered(bool)), SLOT(slotStopMachine()));

    QObject::connect(&d->syncTimer, SIGNAL(timeout()), d->screen, SLOT(refresh()));
    QObject::connect(&d->syncTimer, SIGNAL(timeout()), d->screen, SLOT(flushKeyBuffer()));
    d->syncTimer.start(50);

    QObject::connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(onAboutToQuit()));
}

MainWindow::~MainWindow()
{
    delete d;
    d = 0L;
}

void MainWindow::slotFloppyAClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Choose floppy B image")
   );
    if (fileName.isNull())
        return;
    vomit_set_drive_image(0, qPrintable(fileName));
}

void MainWindow::slotFloppyBClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Choose floppy B image")
   );
    if (fileName.isNull())
        return;
    vomit_set_drive_image(1, qPrintable(fileName));
}

void MainWindow::slotPauseMachine()
{
    d->pauseMachine->setEnabled(false);
    d->startMachine->setEnabled(true);
    d->stopMachine->setEnabled(true);

    d->screen->setTinted(true);

    VM_ASSERT(d->worker);
    d->worker->stopMachine();
}

void MainWindow::slotStopMachine()
{
    d->pauseMachine->setEnabled(false);
    d->startMachine->setEnabled(true);
    d->stopMachine->setEnabled(false);

    d->screen->setTinted(true);

    VM_ASSERT(d->worker);
    d->worker->stopMachine();
}

void MainWindow::slotStartMachine()
{
    d->pauseMachine->setEnabled(true);
    d->startMachine->setEnabled(false);
    d->stopMachine->setEnabled(true);

    d->screen->setTinted(false);

    VM_ASSERT(d->worker);
    d->worker->startMachine();
}

void MainWindow::slotRebootMachine()
{
    VM_ASSERT(d->worker);
    d->worker->rebootMachine();
}

Machine* MainWindow::machine() const
{
    return d->machine;
}

void MainWindow::onAboutToQuit()
{
    d->worker->stopMachine();
}
