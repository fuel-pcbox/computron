#include "screen.h"
extern "C" {
#include "../include/debug.h"
}
#include <QPainter>
#include <QApplication>
#include <QPaintEvent>

typedef struct {
  byte data[16];
} fontcharbitmap_t;

Screen::Screen()
{
	init();
	synchronizeFont();
	setTextMode( 80, 25 );
	m_videoMemory = mem_space + 0xB8000;

	m_canvas12 = QImage( 640, 480, QImage::Format_Indexed8 );

	synchronizeColors();

	m_clearBackground = true;

	setAttribute( Qt::WA_OpaquePaintEvent );
	setAttribute( Qt::WA_NoSystemBackground );

	setFocusPolicy( Qt::ClickFocus );
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

	p.eraseRect( x, y, m_characterWidth, m_characterHeight );

	// Text
	p.setPen( m_color[color & 0xF] );
	p.drawPixmap( x, y, m_character[c] );
}

extern "C" {
	bool is_video_dirty();
	void clear_video_dirty();
	bool is_palette_dirty();
	void clear_palette_dirty();
}

void
Screen::refresh()
{
	if( mem_space[0x449] == 0x12 )
	{
		if( is_palette_dirty() )
		{
			synchronizeColors();
			clear_palette_dirty();
		}

		if( is_video_dirty() )
		{
			//vlog( VM_VIDEOMSG, "Painting mode12h screen" );
			update();
			clear_video_dirty();

			// Trigger a clear on next mode3 paint...
			m_clearBackground = true;
		}
		/*
		else
		{
			static int moop = 0;
			if( moop == 0 )
			{
				update();
				moop = 10;
			}
			else --moop;
		}
		*/
	}
	else if( mem_space[0x449] == 0x03 )
	{
		setTextMode( 80, 25 );
		update();
	}
	else
	{
		update();
	}
}

void
Screen::putpixel( QPainter &p, int x, int y, int color )
{
	p.setPen( m_color[color & 0xF] );
	p.drawPoint( x, y );
}

void
Screen::paintMode12( QPaintEvent *e )
{
	setFixedSize( 640, 480 );

	extern byte vm_p0[];
	extern byte vm_p1[];
	extern byte vm_p2[];
	extern byte vm_p3[];

	word offset = 0;

	uchar *pixels = m_canvas12.bits();


	for( int y = e->rect().top(); y < e->rect().bottom(); y ++ )
	{
		for( int x = e->rect().left(); x < e->rect().right(); x += 8, ++offset )
		{
#define D(i) ((vm_p0[offset]>>i) & 1) | (((vm_p1[offset]>>i) & 1)<<1) | (((vm_p2[offset]>>i) & 1)<<2) | (((vm_p3[offset]>>i) & 1)<<3)

			pixels[(y*640) + x+7] = D(0);
			pixels[(y*640) + x+6] = D(1);
			pixels[(y*640) + x+5] = D(2);
			pixels[(y*640) + x+4] = D(3);
			pixels[(y*640) + x+3] = D(4);
			pixels[(y*640) + x+2] = D(5);
			pixels[(y*640) + x+1] = D(6);
			pixels[(y*640) + x+0] = D(7);

			/*
			m_canvas12.setPixel( x+7, y, p1 );
			m_canvas12.setPixel( x+6, y, p2 );
			m_canvas12.setPixel( x+5, y, p3 );
			m_canvas12.setPixel( x+4, y, p4 );
			m_canvas12.setPixel( x+3, y, p5 );
			m_canvas12.setPixel( x+2, y, p6 );
			m_canvas12.setPixel( x+1, y, p7 );
			m_canvas12.setPixel( x+0, y, p8 );
			*/
		}
	}

	QPainter wp( this );
	wp.drawImage( e->rect(), m_canvas12 );
}

#if 0
void
Screen::paintMode12( QPaintEvent *e )
{
	setFixedSize( 640, 480 );

	//QPixmap pm( e->rect().size() );
	static QPixmap pm( 640, 480 );
	QPainter p( &pm );

	extern byte vm_p0[];
	extern byte vm_p1[];
	extern byte vm_p2[];
	extern byte vm_p3[];

	word offset = 0;

	for( int y = e->rect().top(); y < e->rect().bottom(); y ++ )
	{
		for( int x = e->rect().left(); x < e->rect().right(); x += 8, ++offset )
		{
			byte data[4];
			data[0] = vm_p0[offset];
			data[1] = vm_p1[offset];
			data[2] = vm_p2[offset];
			data[3] = vm_p3[offset];

#define D(i) ((vm_p0[offset]>>i) & 1) | (((vm_p1[offset]>>i) & 1)<<1) | (((vm_p2[offset]>>i) & 1)<<2) | (((vm_p3[offset]>>i) & 1)<<3)

			byte p1 = D(0);
			byte p2 = D(1);
			byte p3 = D(2);
			byte p4 = D(3);
			byte p5 = D(4);
			byte p6 = D(5);
			byte p7 = D(6);
			byte p8 = D(7);

			putpixel( p, x+7, y, p1 );
			putpixel( p, x+6, y, p2 );
			putpixel( p, x+5, y, p3 );
			putpixel( p, x+4, y, p4 );
			putpixel( p, x+3, y, p5 );
			putpixel( p, x+2, y, p6 );
			putpixel( p, x+1, y, p7 );
			putpixel( p, x+0, y, p8 );
		}
	}

	QPainter wp( this );
	wp.drawPixmap( e->rect(), pm );
}
#endif

void
Screen::resizeEvent( QResizeEvent *e )
{
	QWidget::resizeEvent( e );
	vlog( VM_VIDEOMSG, "Resizing viewport" );
}

void
Screen::paintEvent( QPaintEvent *e )
{
	if( mem_space[0x449] == 0x12 )
	{
		paintMode12( e );
		return;
	}

	QPainter p( this );
	//synchronizeFont();

	if( m_clearBackground )
	{
		p.eraseRect( rect() );
	}

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
			if( m_clearBackground || ((lcx == x && lcy == y) || *v != last[y][x] || *(v+1) != lasta[y][x] ))
			{
				putCharacter( p, y, x, *(v+1), *v);
				last[y][x] = *v;
				lasta[y][x] = *(v+1);
			}
			v += 2;
		}
	}

	byte cursorStart = vga_read_register( 0x0A ) * 2;
	byte cursorEnd = vga_read_register( 0x0B ) * 2;

	//vlog( VM_VIDEOMSG, "cursor: %d to %d", cursorStart, cursorEnd );

	p.fillRect( cx * m_characterWidth, cy * m_characterHeight + cursorStart, m_characterWidth, cursorEnd - cursorStart, QBrush( m_color[13] ));

	lcx = cx;
	lcy = cy;

	m_clearBackground = false;
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

void
Screen::synchronizeColors()
{
	for( int i = 0; i < 16; ++i )
	{
		m_color[i].setRgb(
			vga_color_register[vga_palette_register[i]].r << 2,
			vga_color_register[vga_palette_register[i]].g << 2,
			vga_color_register[vga_palette_register[i]].b << 2
		);
	}

	for( int i = 0; i < 16; ++i )
		m_brush[i] = QBrush( m_color[i] );

	for( int i = 0; i < 16; ++i )
		m_canvas12.setColor( i, m_color[i].rgb() );
}
