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

#include "statewidget.h"

#include "ui_statewidget.h"
#include "debug.h"
#include "machine.h"
#include "vcpu.h"
#include "screen.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtWidgets/QVBoxLayout>

struct StateWidget::Private
{
    QTimer syncTimer;
    Ui_StateWidget ui;
};

StateWidget::StateWidget(Machine& m)
    : QWidget(nullptr)
    , m_machine(m)
    , d(new Private)
{
    setFixedSize(180, 400);
    d->ui.setupUi(this);

    connect(&d->syncTimer, SIGNAL(timeout()), this, SLOT(sync()));
    d->syncTimer.start(100);
}

StateWidget::~StateWidget()
{
    delete d;
    d = nullptr;
}

#define DO_LABEL(name, fmt) d->ui.lbl ## name->setText(s.sprintf(fmt, cpu.get ## name()));

void StateWidget::sync()
{
    QString s;
    auto& cpu = *machine().cpu();

    DO_LABEL(EAX, "%08X");
    DO_LABEL(EBX, "%08X");
    DO_LABEL(ECX, "%08X");
    DO_LABEL(EDX, "%08X");
    DO_LABEL(EBP, "%08X");
    DO_LABEL(ESP, "%08X");
    DO_LABEL(ESI, "%08X");
    DO_LABEL(EDI, "%08X");
    DO_LABEL(CS, "%04X");
    DO_LABEL(DS, "%04X");
    DO_LABEL(ES, "%04X");
    DO_LABEL(SS, "%04X");
    DO_LABEL(FS, "%04X");
    DO_LABEL(GS, "%04X");
    DO_LABEL(CR0, "%08X");

    d->ui.lblPC->setText(s.sprintf("%04X:%08X", cpu.getBaseCS(), cpu.getBaseEIP()));
}
