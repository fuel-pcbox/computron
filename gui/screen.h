#ifndef __screen_h__
#define __screen_h__

#include <QWidget>
#include <QBitmap>
#include <QQueue>

extern "C" {
	#include "../include/vomit.h"
}

class Screen : public QWidget
{
	Q_OBJECT

public:
	Screen();
	virtual ~Screen();

	int characterWidth() const;
	int characterHeight() const;

	bool inTextMode() const;

	void setTextMode( int w, int h );

	void synchronizeFont();
	void synchronizeColors();

	void putCharacter( QPainter &p, int row, int column, byte color, byte c );

	word nextKey();
	word peekKey();
	byte popKeyData();

protected:
	void keyPressEvent( QKeyEvent *e );
	void keyReleaseEvent( QKeyEvent *e );
	void mouseMoveEvent( QMouseEvent *e );
	void mousePressEvent( QMouseEvent *e );
	void mouseReleaseEvent( QMouseEvent *e );

public slots:
	void refresh();

private slots:
	void flushKeyBuffer();

private:
	void paintEvent( QPaintEvent * );
	void resizeEvent( QResizeEvent * );
	void init();
	bool m_inTextMode;
	int m_width, m_height;
	int m_characterWidth, m_characterHeight;
	byte *m_videoMemory;

	QBrush m_brush[16];
	QColor m_color[16];
	QBitmap m_character[256];
	QQueue<word> m_keyQueue;
	QQueue<byte> m_rawQueue;

	QImage m_canvas12;

	void paintMode12( QPaintEvent * );
	void putpixel( QPainter &p, int x, int y, int color );

	bool m_clearBackground;
};

#endif // __screen_h__
