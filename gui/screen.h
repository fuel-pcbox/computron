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

	void setScreenSize( int width, int height );

	void setTinted( bool );

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

	QImage m_screen12;
	QImage m_render12;

	void renderMode12( QImage &target );

	bool m_clearBackground;
	int m_rows;
	bool m_tinted;
};

#endif
