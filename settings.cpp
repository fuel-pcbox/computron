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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vomit.h"
#include "floppy.h"
#include "debug.h"
#include "settings.h"
#include <QFile>
#include <QStringList>

struct FloppyType
{
    const char* name;
    WORD sectorsPerTrack;
    WORD heads;
    DWORD sectors;
    WORD bytesPerSector;
    BYTE mediaType;
};

static FloppyType gFloppyTypes[] =
{
    { "1.44M", 18, 2, 2880, 512, 4 },
    { "720kB",  9, 2, 1440, 512, 3 },
    { "1.2M",  15, 2, 2400, 512, 2 },
    { "360kB",  9, 2,  720, 512, 1 },
    { "320kB",  8, 2,  640, 512, 0 },
    { "160kB",  8, 1,  320, 512, 0 },
    { 0L,       0, 0,    0,   0, 0 }
};

Settings::Settings()
    : m_memorySize(0)
{
}

Settings::~Settings()
{
}

static bool parseAddress(const QString& string, DWORD* address)
{
    VM_ASSERT(address);

    QStringList parts = string.split(QLatin1Char(':'), QString::SkipEmptyParts);
    if (parts.count() != 2)
        return false;

    bool ok;
    WORD segment = parts.at(0).toUInt(&ok, 16);
    if (!ok)
        return false;

    DWORD offset = parts.at(1).toUInt(&ok, 16);
    if (!ok)
        return false;

    *address = vomit_toFlatAddress(segment, offset);
    return true;
}

bool Settings::handleLoadFile(const QStringList& arguments)
{
    // load-file <segment:offset> <path/to/file>

    if (arguments.count() != 2)
        return false;

    DWORD address;
    if (!parseAddress(arguments.at(0), &address))
        return false;

    m_files.insert(address, arguments.at(1));
    return true;
}

bool Settings::handleMemorySize(const QStringList& arguments)
{
    // memory-size <size>

    if (arguments.count() != 1)
        return false;

    bool ok;
    unsigned size = arguments.at(0).toUInt(&ok);
    if (!ok)
        return false;

    setMemorySize(size * 1024);
    return true;
}

bool Settings::handleFixedDisk(const QStringList& arguments)
{
    // fixed-disk <index> <path/to/file> <size>

    if (arguments.count() != 3)
        return false;

    bool ok;
    unsigned index = arguments.at(0).toUInt(&ok);
    if (!ok)
        return false;

    QString fileName = arguments.at(1);

    unsigned size = arguments.at(2).toUInt(&ok);
    if (!ok)
        return false;

    vlog(LogConfig, "Fixed disk %u: %s (%ld KiB)", index, qPrintable(fileName), size);

    // FIXME: This code sucks.
    index += 2;
    strcpy(drv_imgfile[index], qPrintable(fileName));
    drv_spt[index] = 63;
    drv_heads[index] = 16;
    drv_sectsize[index] = 512;
    drv_status[index] = 1;
    drv_sectors[index] = (size * 1024) / drv_sectsize[index];

    return true;
}

bool Settings::handleFloppyDisk(const QStringList& arguments)
{
    // floppy-disk <index> <type> <path/to/file>

    if (arguments.count() != 3)
        return false;

    bool ok;
    unsigned index = arguments.at(0).toUInt(&ok);
    if (!ok)
        return false;

    QString type = arguments.at(1);
    QString fileName = arguments.at(2);

    FloppyType* ft;
    for (ft = gFloppyTypes; ft->name; ++ft) {
        if (type == QLatin1String(ft->name))
            break;
    }

    if (!ft->name) {
        vlog(LogConfig, "Invalid floppy type: \"%s\"", qPrintable(type));
        return false;
    }

    // FIXME: This code sucks.
    strcpy(drv_imgfile[index], qPrintable(fileName));
    drv_spt[index] = ft->sectorsPerTrack;
    drv_heads[index] = ft->heads;
    drv_sectors[index] = ft->sectors;
    drv_sectsize[index] = ft->bytesPerSector;
    drv_type[index] = ft->mediaType;
    drv_status[index] = 1;

    // FIXME: What should the media type be?
    drv_type[index] = 0;

    vlog(LogConfig, "Floppy %u: %s (%uspt, %uh, %us (%ub))", index, qPrintable(fileName), ft->sectorsPerTrack, ft->heads, ft->sectors, ft->bytesPerSector);
    return true;
}

Settings* Settings::createFromFile(const QString& fileName)
{
#if 0
    char curline[256], *curtok;
    char lfname[MAX_FN_LENGTH];
    WORD lseg, loff, ldrv, lspt, lhds, lsect, lsectsize;
#endif

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        vlog(LogConfig, "Couldn't load %s", qPrintable(fileName));
        return 0;
    }

    unsigned lineNumber = 0;
    Settings* settings = new Settings;

    if (!settings)
        return 0;

    static QRegExp whitespaceRegExp("\\s");

    while (!file.atEnd()) {
        QString line = QString::fromLocal8Bit(file.readLine());
        lineNumber++;

        if (line.startsWith(QLatin1Char('#')))
            continue;

        QStringList arguments = line.split(whitespaceRegExp, QString::SkipEmptyParts);

        if (arguments.isEmpty())
            continue;

        QString command = arguments.takeFirst();

        bool success = false;

        if (command == QLatin1String("load-file"))
            success = settings->handleLoadFile(arguments);
        else if (command == QLatin1String("memory-size"))
            success = settings->handleMemorySize(arguments);
        else if (command == QLatin1String("fixed-disk"))
            success = settings->handleFixedDisk(arguments);
        else if (command == QLatin1String("floppy-disk"))
            success = settings->handleFloppyDisk(arguments);

        if (!success) {
            vlog(LogConfig, "Failed parsing %s:%u %s", qPrintable(fileName), lineNumber, qPrintable(line));
            return 0;
        }
    }

    return settings;
}