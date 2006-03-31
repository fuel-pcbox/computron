#include "screen.h"
#include <QPainter>

typedef struct {
  byte data[16];
} fontcharbitmap_t;

Screen::Screen()
{
	synchronizeFont();
	setTextMode( 80, 25 );
	m_videoMemory = mem_space + 0xB8000;

	m_color[0].setRgb( 0, 0, 0 );
	m_color[1].setRgb( 0, 0, 0x7f );
	m_color[2].setRgb( 0, 0x7f, 0 );
	m_color[3].setRgb( 0, 0x7f, 0x7f );
	m_color[4].setRgb( 0x7f, 0, 0 );
	m_color[5].setRgb( 0x7f, 0, 0x7f );
	m_color[6].setRgb( 0x7f, 0x7f, 0 );
	m_color[7].setRgb( 0x7f, 0x7f, 0x7f );

	m_color[8].setRgb( 0, 0, 0 );
	m_color[9].setRgb( 0, 0, 0xff );
	m_color[10].setRgb( 0, 0xff, 0 );
	m_color[11].setRgb( 0, 0xff, 0xff );
	m_color[12].setRgb( 0xff, 0, 0 );
	m_color[13].setRgb( 0xff, 0, 0xff );
	m_color[14].setRgb( 0xff, 0xff, 0 );
	m_color[15].setRgb( 0xff, 0xff, 0xff );

	for( int i = 0; i < 16; ++i )
	{
		m_brush[i] = QBrush( m_color[i] );
	}

	setAttribute( Qt::WA_OpaquePaintEvent );
}

Screen::~Screen()
{
}

void
Screen::putCharacter( QPainter &p, int row, int column, byte color, byte c )
{
	int x = column * m_characterWidth;
	int y = row * m_characterHeight;

	p.setBackground( m_brush[color >> 4] );

	// Text
	p.setPen( m_color[color & 0xF] );
	p.drawPixmap( x, y, m_character[c] );
}

void
Screen::paintEvent( QPaintEvent *e )
{
	(void) e;
	QPainter p( this );

	p.setBackgroundMode( Qt::OpaqueMode );

	//synchronizeFont();

	byte *v = m_videoMemory;
	static byte last[25][80];
	static byte lasta[25][80];
	static byte lcx = 0, lcy = 0;

	byte cx, cy;
	load_cursor( &cy, &cx );

	for( int y = 0; y < 25; ++y )
	{
		for( int x = 0; x < 80; ++x )
		{
			if( (lcx == x && lcy == y) || *v != last[y][x] || *(v+1) != lasta[y][x] )
			{
				putCharacter( p, y, x, *(v+1), *v);
				last[y][x] = *v;
				lasta[y][x] = *(v+1);
			}
			v += 2;
		}
	}

	byte cursorStart = vga_read_register( 0x0A );
	byte cursorEnd = vga_read_register( 0x0B );
	
	p.fillRect( cx * m_characterWidth, cy * m_characterHeight + cursorStart, m_characterWidth, cursorEnd - cursorStart, QBrush( m_color[13] ));

	lcx = cx;
	lcy = cy;
}

void
Screen::setTextMode( int w, int h )
{
	int wi = w * m_characterWidth;
	int he = h * m_characterHeight;

	setFixedSize( wi, he );
	m_inTextMode = true;
}

bool
Screen::inTextMode() const
{
	return m_inTextMode;
}

void
Screen::synchronizeFont()
{
	m_characterWidth = 8;
	m_characterHeight = 16;
	const QSize s( 8, 16 );

	fontcharbitmap_t *fbmp = (fontcharbitmap_t *)(mem_space + 0xC4000);

	for( int i = 0; i < 256; ++i )
	{
	//	m_character[i] = QBitmap::fromData( s, (const uchar *)(bx_vgafont[i].data), QImage::Format_MonoLSB );
		m_character[i] = QBitmap::fromData( s, (const byte *)fbmp[i].data, QImage::Format_MonoLSB );
	}
}
