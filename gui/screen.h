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

	void putCharacter( QPainter &p, int row, int column, byte color, byte c );

private slots:
	void paintEvent( QPaintEvent * );

private:
	bool m_inTextMode;
	int m_width, m_height;
	int m_characterWidth, m_characterHeight;
	byte *m_videoMemory;

	QBrush m_brush[16];
	QColor m_color[16];
	QBitmap m_character[256];
	QQueue<word> m_keyQueue;
};

#endif // __screen_h__
