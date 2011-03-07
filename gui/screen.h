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

#ifndef __screen_h__
#define __screen_h__

#include <QWidget>

#include "vomit.h"

class Screen : public QWidget
{
    Q_OBJECT
public:
    Screen(VCpu *cpu, QWidget *parent = 0);
    virtual ~Screen();

    int characterWidth() const;
    int characterHeight() const;

    bool inTextMode() const;
    void setTextMode( int w, int h );

    void synchronizeFont();
    void synchronizeColors();

    WORD nextKey();
    WORD peekKey();
    BYTE popKeyData();

    void setScreenSize( int width, int height );

    void setTinted( bool );

    struct Cursor
    {
        BYTE row;
        BYTE column;

        Cursor() : row(0), column(0) {}
        Cursor(uint8_t r, uint8_t c) : row(r), column(c) {}
    };

protected:
    void keyPressEvent( QKeyEvent *e );
    void keyReleaseEvent( QKeyEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );

public slots:
    void refresh();
    void loadKeymap(const QString &filename);

private slots:
    void flushKeyBuffer();

private:
    void paintEvent( QPaintEvent * );
    void resizeEvent( QResizeEvent * );
    void init();
    void putCharacter(QPainter& p, int row, int column, BYTE color, BYTE c);

    bool m_inTextMode;
    int m_width, m_height;
    int m_characterWidth, m_characterHeight;

    QImage m_screen12;
    QImage m_render12;

    QImage m_render0D;

    QImage m_render13;

    void renderMode13( QImage &target );
    void renderMode12( QImage &target );
    void renderMode0D( QImage &target );

    int m_rows;
    int m_columns;

    bool m_tinted;

    friend int get_current_x();
    friend int get_current_y();

    struct Private;
    Private *d;
};

#endif
