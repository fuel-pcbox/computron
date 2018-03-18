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

#ifndef DEBUGGER_H
#define DEBUGGER_H

class CPU;
class QString;
class QStringList;

class Debugger
{
public:
    explicit Debugger(CPU&);
    ~Debugger();

    CPU& cpu() { return m_cpu; }

    void enter();
    void exit();

    bool isActive() const { return m_active; }

    void doConsole();

private:
    CPU& m_cpu;
    bool m_active { false };

    void handleCommand(const QString&);

    void handleQuit();
    void handleDumpRegisters();
    void handleDumpSegment(const QStringList&);
    void handleDumpIVT();
    void handleReconfigure();
    void handleStep();
    void handleContinue();
    void handleBreakpoint(const QStringList&);
    void handleDumpMemory(const QStringList&);
    void handleDumpFlatMemory(const QStringList&);
    void handleTracing(const QStringList&);
    void handleIRQ(const QStringList&);
    void handleDumpUnassembled(const QStringList&);
    void handleSelector(const QStringList&);
};

#endif
