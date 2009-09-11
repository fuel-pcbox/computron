#ifndef __screen_h__
#define __screen_h__

#include <QWidget>
#include <QBitmap>
#include <QQueue>
#include <QMutex>

#include "../include/vomit.h"

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

	struct Cursor
	{
		uint8_t row;
		uint8_t column;

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

	QImage m_screen0D;
	QImage m_render0D;

	void renderMode12( QImage &target );
	void renderMode0D( QImage &target );

	int m_rows;
	int m_columns;

	bool m_tinted;

	friend void screen_direct_update( word offset );

	friend int get_current_x();
	friend int get_current_y();

	QMutex m_keyQueueLock;
	QMutex m_mouseLock;
};

#endif
