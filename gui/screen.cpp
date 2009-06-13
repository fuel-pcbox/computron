#include "screen.h"
extern "C" {
#include "../include/debug.h"
#include "../include/vomit.h"
}
#include <QPainter>
#include <QApplication>
#include <QPaintEvent>

typedef struct {
  byte data[16];
} fontcharbitmap_t;

Screen::Screen()
{
	m_rows = 0;
	m_width = 0;
	m_height = 0;
	m_tinted = false;

	init();
	synchronizeFont();
	setTextMode( 80, 25 );
	m_videoMemory = mem_space + 0xB8000;

	m_screen12 = QImage( 640, 480, QImage::Format_Indexed8 );
	m_render12 = QImage( 640, 480, QImage::Format_Indexed8 );

	m_screen0D = QImage( 320, 200, QImage::Format_Indexed8 );
	m_render0D = QImage( 320, 200, QImage::Format_Indexed8 );

	m_screen12.fill(0);
	m_render12.fill(0);
	m_screen0D.fill(0);
	m_render0D.fill(0);

	synchronizeColors();

	m_clearBackground = true;

	setAttribute( Qt::WA_OpaquePaintEvent );
	setAttribute( Qt::WA_NoSystemBackground );

	setFocusPolicy( Qt::ClickFocus );

	setMouseTracking( true );
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

	if( m_tinted )
	{
		p.save();
		p.setCompositionMode( QPainter::CompositionMode_SourceOver );
		p.setOpacity( 0.3 );
		p.fillRect( x, y, m_characterWidth, m_characterHeight, Qt::blue );
		p.restore();
	}
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
	if( (mem_space[0x449] & 0x7F) == 0x12 )
	{
		if( is_palette_dirty() )
		{
			synchronizeColors();
			clear_palette_dirty();
		}

		if( is_video_dirty() )
		{
			//vlog( VM_VIDEOMSG, "Painting mode12h screen" );
			renderMode12( m_render12 );

			QRect updateRect;

			uchar *newBits = m_render12.bits();
			uchar *oldBits = m_screen12.bits();

			// Compare screen rendition to what's showing ATM, to find the region
			// that needs to be updated.
			for( int y = 0; y < 480; ++y )
			{
				uchar *newPixels = &newBits[y*640];
				uchar *oldPixels = &oldBits[y*640];
				for( int x = 0; x < 640; ++x )
				{
					if( newPixels[x] != oldPixels[x] )
					{
						if( updateRect.isNull() )
							updateRect.setCoords( x, y, x, y );
						else
						{
							if( x < updateRect.left() )
								updateRect.setLeft( x );
							if( x > updateRect.right() )
								updateRect.setRight( x );
							if( y < updateRect.top() )
								updateRect.setTop( y );
							if( y > updateRect.bottom() )
								updateRect.setBottom( y );
						}
					}
				}
			}

			m_screen12 = m_render12;
			update( updateRect );
			clear_video_dirty();

			// Trigger a clear on next mode3 paint...
			m_clearBackground = true;
		}
	}
	else if( (mem_space[0x449] & 0x7F) == 0x0D )
	{
		if( is_palette_dirty() )
		{
			synchronizeColors();
			clear_palette_dirty();
		}

		if( is_video_dirty() )
		{
			//vlog( VM_VIDEOMSG, "Painting mode0Dh screen" );
			renderMode0D( m_render0D );

			QRect updateRect;

			uchar *newBits = m_render0D.bits();
			uchar *oldBits = m_screen0D.bits();

			// Compare screen rendition to what's showing ATM, to find the region
			// that needs to be updated.
			for( int y = 0; y < 200; ++y )
			{
				uchar *newPixels = &newBits[y*640];
				uchar *oldPixels = &oldBits[y*640];
				for( int x = 0; x < 320; ++x )
				{
					if( newPixels[x] != oldPixels[x] )
					{
						if( updateRect.isNull() )
							updateRect.setCoords( x, y, x, y );
						else
						{
							if( x < updateRect.left() )
								updateRect.setLeft( x );
							if( x > updateRect.right() )
								updateRect.setRight( x );
							if( y < updateRect.top() )
								updateRect.setTop( y );
							if( y > updateRect.bottom() )
								updateRect.setBottom( y );
						}
					}
				}
			}

			m_screen0D = m_render0D;
			update( updateRect );
			clear_video_dirty();

			// Trigger a clear on next mode3 paint...
			m_clearBackground = true;
		}
	}
	else if( (mem_space[0x449] & 0x7F) == 0x03 )
	{
		int rows = mem_space[0x484] + 1;
		switch( rows )
		{
			case 25:
			case 50:
				break;
			default:
				rows = 25;
				break;
		}
		setTextMode( 80, rows );
		update();
	}
	else
	{
		update();
	}
}

void
Screen::setScreenSize( int width, int height )
{
	if( m_width == width && m_height == height )
		return;

	m_width = width;
	m_height = height;

	setFixedSize( m_width, m_height );
}

void
Screen::renderMode12( QImage &target )
{
	extern byte vm_p0[];
	extern byte vm_p1[];
	extern byte vm_p2[];
	extern byte vm_p3[];

	int offset = 0;

	for( int y = 0; y < 480; ++y )
	{
		uchar *px = &target.bits()[y*640];

		for( int x = 0; x < 640; x += 8, ++offset )
		{
#define D(i) ((vm_p0[offset]>>i) & 1) | (((vm_p1[offset]>>i) & 1)<<1) | (((vm_p2[offset]>>i) & 1)<<2) | (((vm_p3[offset]>>i) & 1)<<3)

			*(px++) = D(7);
			*(px++) = D(6);
			*(px++) = D(5);
			*(px++) = D(4);
			*(px++) = D(3);
			*(px++) = D(2);
			*(px++) = D(1);
			*(px++) = D(0);
		}
	}
}

void
Screen::renderMode0D( QImage &target )
{
	extern byte vm_p0[];
	extern byte vm_p1[];
	extern byte vm_p2[];
	extern byte vm_p3[];

	int offset = 0;

	for( int y = 0; y < 200; ++y )
	{
		uchar *px = &target.bits()[y*320];

#define A0D(i) ((vm_p0[offset]>>i) & 1) | (((vm_p1[offset]>>i) & 1)<<1) | (((vm_p2[offset]>>i) & 1)<<2) | (((vm_p3[offset]>>i) & 1)<<3)
		for( int x = 0; x < 320; x += 8, ++offset )
		{
			*(px++) = D(7);
			*(px++) = D(6);
			*(px++) = D(5);
			*(px++) = D(4);
			*(px++) = D(3);
			*(px++) = D(2);
			*(px++) = D(1);
			*(px++) = D(0);
		}
	}
	return;

	for( int y = 0; y < 200; ++y )
	{
		uchar *px = &target.bits()[y*320];

		for( int x = 0; x < 320; x += 8, ++offset )
		{
#define D(i) ((vm_p0[offset]>>i) & 1) | (((vm_p1[offset]>>i) & 1)<<1) | (((vm_p2[offset]>>i) & 1)<<2) | (((vm_p3[offset]>>i) & 1)<<3)

			*(px++) = D(7);
			*(px++) = D(6);
			*(px++) = D(5);
			*(px++) = D(4);
			*(px++) = D(3);
			*(px++) = D(2);
			*(px++) = D(1);
			*(px++) = D(0);
		}
	}
}

void
Screen::resizeEvent( QResizeEvent *e )
{
	QWidget::resizeEvent( e );
	vlog( VM_VIDEOMSG, "Resizing viewport" );

	m_clearBackground = true;
	update();
}

void
Screen::paintEvent( QPaintEvent *e )
{
	if( (mem_space[0x449] & 0x7F) == 0x12 )
	{
		setScreenSize( 640, 480 );
		QPainter wp( this );
		wp.drawImage( e->rect(), m_screen12.copy( e->rect() ));

		if( m_tinted )
		{
			wp.setOpacity( 0.3 );
			wp.fillRect( e->rect(), Qt::blue );
		}
		return;
	}

	if( (mem_space[0x449] & 0x7F) == 0x0D )
	{
		setScreenSize( 320, 200 );
		QPainter wp( this );
		wp.drawImage( e->rect(), m_screen0D.copy( e->rect() ));

		if( m_tinted )
		{
			wp.setOpacity( 0.3 );
			wp.fillRect( e->rect(), Qt::blue );
		}
		return;
	}

	QPainter p( this );
	//synchronizeFont();

	if( m_clearBackground )
	{
		p.eraseRect( rect() );
	}

	byte *v = m_videoMemory;
	static byte last[50][80];
	static byte lasta[50][80];
	static byte lcx = 0, lcy = 0;

	byte cx, cy;
	load_cursor( &cy, &cx );

	for( int y = 0; y < m_rows; ++y )
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

	byte cursorStart = vga_read_register( 0x0A );
	byte cursorEnd = vga_read_register( 0x0B );

	// HACK 2000!
	if( cursorEnd < 14 )
	{
		cursorEnd *= 2;
		cursorStart *= 2;
	}

	//vlog( VM_VIDEOMSG, "rows: %d, row: %d, col: %d", m_rows, cy, cx );
	//vlog( VM_VIDEOMSG, "cursor: %d to %d", cursorStart, cursorEnd );

	p.setCompositionMode( QPainter::CompositionMode_Xor );
	p.fillRect( cx * m_characterWidth, cy * m_characterHeight + cursorStart, m_characterWidth, cursorEnd - cursorStart, QBrush( m_color[14] ));

	lcx = cx;
	lcy = cy;

	m_clearBackground = false;
}

void
Screen::setTextMode( int w, int h )
{
	int wi = w * m_characterWidth;
	int he = h * m_characterHeight;

	m_rows = h;

	setScreenSize( wi, he );
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
	{
		m_screen12.setColor( i, m_color[i].rgb() );
		m_render12.setColor( i, m_color[i].rgb() );
		m_screen0D.setColor( i, m_color[i].rgb() );
		m_render0D.setColor( i, m_color[i].rgb() );
	}
}

static int currentX = 0;
static int currentY = 0;

void
Screen::mouseMoveEvent( QMouseEvent *e )
{
	currentX = e->x();
	currentY = e->y();

	QWidget::mouseMoveEvent( e );

	busmouse_event();
}

void
Screen::mousePressEvent( QMouseEvent *e )
{
	currentX = e->x();
	currentY = e->y();

	QWidget::mousePressEvent( e );
	switch( e->button() )
	{
		case Qt::LeftButton:
			busmouse_press( 1 );
			break;
		case Qt::RightButton:
			busmouse_press( 2 );
			break;
	}
}

void
Screen::mouseReleaseEvent( QMouseEvent *e )
{
	currentX = e->x();
	currentY = e->y();

	QWidget::mouseReleaseEvent( e );
	switch( e->button() )
	{
		case Qt::LeftButton:
			busmouse_release( 1 );
			break;
		case Qt::RightButton:
			busmouse_release( 2 );
			break;
	}
}

int
get_current_x()
{
	return currentX;
}

int
get_current_y()
{
	return currentY;
}

void
Screen::setTinted( bool t )
{
	m_tinted = t;
	m_clearBackground = true;
	repaint();
}
