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
    , d(make<Private>())
{
    setFixedSize(220, 400);
    d->ui.setupUi(this);

    connect(&d->syncTimer, SIGNAL(timeout()), this, SLOT(sync()));
    d->syncTimer.start(200);
}

StateWidget::~StateWidget()
{
}

#define DO_LABEL(name, fmt) d->ui.lbl ## name->setText(s.sprintf(fmt, cpu.get ## name()));

#define DO_LABEL_N(name, title, fmt) do { \
        d->ui.lblTitle ## name->setText(title); \
        d->ui.lbl ## name->setText(s.sprintf(fmt, cpu.get ## name())); \
    } while (0);

void StateWidget::sync()
{
    QString s;
    auto& cpu = machine().cpu();

    if (cpu.x32()) {
        DO_LABEL_N(EBX, "EBX", "%08X");
        DO_LABEL_N(EAX, "EAX", "%08X");
        DO_LABEL_N(ECX, "ECX", "%08X");
        DO_LABEL_N(EDX, "EDX", "%08X");
        DO_LABEL_N(EBP, "EBP", "%08X");
        DO_LABEL_N(ESP, "ESP", "%08X");
        DO_LABEL_N(ESI, "ESI", "%08X");
        DO_LABEL_N(EDI, "EDI", "%08X");
        d->ui.lblPC->setText(s.sprintf("%04X:%08X", cpu.getBaseCS(), cpu.getBaseEIP()));
    } else {
        DO_LABEL_N(EBX, "BX", "%04X");
        DO_LABEL_N(EAX, "AX", "%04X");
        DO_LABEL_N(ECX, "CX", "%04X");
        DO_LABEL_N(EDX, "DX", "%04X");
        DO_LABEL_N(EBP, "BP", "%04X");
        DO_LABEL_N(ESP, "SP", "%04X");
        DO_LABEL_N(ESI, "SI", "%04X");
        DO_LABEL_N(EDI, "DI", "%04X");
        d->ui.lblPC->setText(s.sprintf("%04X:%04X", cpu.getBaseCS(), cpu.getBaseIP()));
    }
    DO_LABEL(CS, "%04X");
    DO_LABEL(DS, "%04X");
    DO_LABEL(ES, "%04X");
    DO_LABEL(SS, "%04X");
    DO_LABEL(FS, "%04X");
    DO_LABEL(GS, "%04X");
    DO_LABEL(CR0, "%08X");
    DO_LABEL(CR3, "%08X");

#define DO_FLAG(name) flagString += QString("<font color='%1'>%2</font> ").arg(cpu.get ## name() ? "black" : "#ccc").arg(# name);

    QString flagString;
    DO_FLAG(OF);
    DO_FLAG(SF);
    DO_FLAG(ZF);
    DO_FLAG(AF);
    DO_FLAG(PF);
    DO_FLAG(CF);
    DO_FLAG(IF);
    DO_FLAG(TF);
    DO_FLAG(NT);

    d->ui.lblFlags->setText(flagString);

    d->ui.lblSizes->setText(QString("A%1 O%2 X%3").arg(cpu.a16() ? 16 : 32).arg(cpu.o16() ? 16 : 32).arg(cpu.x16() ? 16 : 32));
}
