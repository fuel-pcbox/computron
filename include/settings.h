/*
 * Copyright (C) 2003-2013 Andreas Kling <kling@webkit.org>
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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtCore/QHash>
#include <QtCore/QString>
#include "types.h"

class QStringList;

class Settings
{
public:
    static Settings* createFromFile(const QString&);
    static Settings* createForAutotest(const QString& fileName);

    unsigned memorySize() const { return m_memorySize; }
    void setMemorySize(unsigned size) { m_memorySize = size; }

    WORD entryCS() const { return m_entryCS; }
    WORD entryIP() const { return m_entryIP; }

    QHash<DWORD, QString> files() const { return m_files; }
    QString keymap() const { return m_keymap; }

    bool isForAutotest() const { return m_forAutotest; }
    void setForAutotest(bool b) { m_forAutotest = b; }

    ~Settings();

private:
    Settings();
    Settings(const Settings&);
    Settings& operator=(const Settings&);

    bool handleLoadFile(const QStringList&);
    bool handleMemorySize(const QStringList&);
    bool handleFixedDisk(const QStringList&);
    bool handleFloppyDisk(const QStringList&);
    bool handleKeymap(const QStringList&);

    QHash<DWORD, QString> m_files;
    QString m_keymap;
    unsigned m_memorySize;
    WORD m_entryCS;
    WORD m_entryIP;
    bool m_forAutotest;
};

#endif
