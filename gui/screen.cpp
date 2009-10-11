#include "screen.h"
#include "debug.h"
#include "vomit.h"
#include "vga_memory.h"

#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QBitmap>
#include <QtCore/QMutex>
#include <QtCore/QQueue>
#include <QtCore/QDebug>

typedef struct {
  byte data[16];
} fontcharbitmap_t;

static Screen *s_self = 0L;

struct Screen::Private
{
    QBitmap character[256];
    QBrush brush[16];
    QColor color[16];

    QMutex keyQueueLock;
    QMutex mouseLock;

    QQueue<word> keyQueue;
    QQueue<byte> rawQueue;

    byte *videoMemory;

    VCpu *cpu;
};

Screen::Screen(VCpu *cpu, QWidget *parent)
    : QWidget(parent),
      d(new Private)
{
    s_self = this;
    d->cpu = cpu;

    m_rows = 0;
    m_columns = 0;
    m_width = 0;
    m_height = 0;
    m_tinted = false;

    init();
    synchronizeFont();
    setTextMode( 80, 25 );
    d->videoMemory = d->cpu->memory + 0xB8000;

    m_screen12 = QImage( 640, 480, QImage::Format_Indexed8 );
    m_render12 = QImage( 640, 480, QImage::Format_Indexed8 );

    m_screen0D = QImage( 320, 200, QImage::Format_Indexed8 );
    m_render0D = QImage( 320, 200, QImage::Format_Indexed8 );

    m_screen12.fill(0);
    m_render12.fill(0);
    m_screen0D.fill(0);
    m_render0D.fill(0);

    synchronizeColors();

    setFocusPolicy( Qt::ClickFocus );

    setMouseTracking( true );

#if 0
    // HACK 2000: Type w<ENTER> at boot for Windows ;-)
    d->keyQueue.enqueue(0x1177);
    d->keyQueue.enqueue(0x1C0D);
#endif
}

Screen::~Screen()
{
}

void
Screen::putCharacter( QPainter &p, int row, int column, byte color, byte c )
{
    int x = column * m_characterWidth;
    int y = row * m_characterHeight;

    p.setBackground( d->brush[color >> 4] );

    p.eraseRect( x, y, m_characterWidth, m_characterHeight );

    // Text
    p.setPen( d->color[color & 0xF] );
    p.drawPixmap( x, y, d->character[c] );

    if( m_tinted )
    {
        p.save();
        p.setCompositionMode( QPainter::CompositionMode_SourceOver );
        p.setOpacity( 0.3 );
        p.fillRect( x, y, m_characterWidth, m_characterHeight, Qt::blue );
        p.restore();
    }
}

bool is_palette_dirty();
void clear_palette_dirty();

void
Screen::refresh()
{
    if ((d->cpu->memory[0x449] & 0x7F) == 0x12)
    {
        if( is_palette_dirty() )
        {
            synchronizeColors();
            clear_palette_dirty();
        }

        if (d->cpu->vgaMemory->isDirty()) {
            update(d->cpu->vgaMemory->dirtyRect());
            d->cpu->vgaMemory->clearDirty();
        }

    } else if ((d->cpu->memory[0x449] & 0x7F) == 0x0D) {
        if( is_palette_dirty() )
        {
            synchronizeColors();
            clear_palette_dirty();
        }

        if (d->cpu->vgaMemory->isDirty()) {
            //vlog( VM_VIDEOMSG, "Painting mode0Dh screen" );
            renderMode0D(m_render0D);

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
        }
    } else if ((d->cpu->memory[0x449] & 0x7F) == 0x03) {
        int rows = d->cpu->memory[0x484] + 1;
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
	const BYTE *p0 = d->cpu->vgaMemory->plane(0);
	const BYTE *p1 = d->cpu->vgaMemory->plane(1);
	const BYTE *p2 = d->cpu->vgaMemory->plane(2);
	const BYTE *p3 = d->cpu->vgaMemory->plane(3);

    int offset = 0;

    for( int y = 0; y < 480; ++y )
    {
        uchar *px = &target.bits()[y*640];

        for( int x = 0; x < 640; x += 8, ++offset )
        {
#define D(i) ((p0[offset]>>i) & 1) | (((p1[offset]>>i) & 1)<<1) | (((p2[offset]>>i) & 1)<<2) | (((p3[offset]>>i) & 1)<<3)

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
	const BYTE *p0 = d->cpu->vgaMemory->plane(0);
	const BYTE *p1 = d->cpu->vgaMemory->plane(1);
	const BYTE *p2 = d->cpu->vgaMemory->plane(2);
	const BYTE *p3 = d->cpu->vgaMemory->plane(3);

    int offset = 0;

    for( int y = 0; y < 200; ++y )
    {
        uchar *px = &target.bits()[y*320];

#define A0D(i) ((p0[offset]>>i) & 1) | (((p1[offset]>>i) & 1)<<1) | (((p2[offset]>>i) & 1)<<2) | (((p3[offset]>>i) & 1)<<3)
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
#define D(i) ((p0[offset]>>i) & 1) | (((p1[offset]>>i) & 1)<<1) | (((p2[offset]>>i) & 1)<<2) | (((p3[offset]>>i) & 1)<<3)

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

void Screen::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    vlog(VM_VIDEOMSG, "Resizing viewport");
    update();
}

void Screen::paintEvent(QPaintEvent *e)
{
    if ((d->cpu->memory[0x449] & 0x7F) == 0x12) {

        setScreenSize(640, 480);
        QPainter wp(this);
        wp.setClipRegion(e->rect());
        wp.drawImage(rect(), *d->cpu->vgaMemory->modeImage(0x12));

        if (m_tinted) {
            wp.setOpacity( 0.3 );
            wp.fillRect( e->rect(), Qt::blue );
        }
        return;
    }

    if ((d->cpu->memory[0x449] & 0x7F) == 0x0D) {

        setScreenSize( 320, 200 );
        QPainter wp(this);
        wp.setClipRegion(e->rect());
        wp.drawImage(rect(), m_screen0D);

        if (m_tinted) {
            wp.setOpacity( 0.3 );
            wp.fillRect( e->rect(), Qt::blue );
        }
        return;
    }

    QPainter p( this );
    //synchronizeFont();

    byte *v = d->videoMemory;

    byte cx, cy;
    load_cursor( &cy, &cx );

    Cursor cursor(cy, cx);

    // Repaint everything
    for (int y = 0; y < m_rows; ++y) {
        for (int x = 0; x < m_columns; ++x) {
            putCharacter(p, y, x, v[1], v[0]);
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

    //vlog( VM_VIDEOMSG, "rows: %d, row: %d, col: %d", m_rows, cursor.row, cursor.column );
    //vlog( VM_VIDEOMSG, "cursor: %d to %d", cursorStart, cursorEnd );

    //p.setCompositionMode( QPainter::CompositionMode_Xor );
    p.fillRect( cursor.column * m_characterWidth, cursor.row * m_characterHeight + cursorStart, m_characterWidth, cursorEnd - cursorStart, d->brush[14] );
}

void
Screen::setTextMode( int w, int h )
{
    int wi = w * m_characterWidth;
    int he = h * m_characterHeight;

    m_rows = h;
    m_columns = w;

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

    fontcharbitmap_t *fbmp = (fontcharbitmap_t *)(d->cpu->memory + 0xC4000);

    for (int i = 0; i < 256; ++i) {
        d->character[i] = QBitmap::fromData(s, (const byte *)fbmp[i].data, QImage::Format_MonoLSB);
    }
}

void
Screen::synchronizeColors()
{
    for( int i = 0; i < 16; ++i )
    {
        d->color[i].setRgb(
            vga_color_register[vga_palette_register[i]].r << 2,
            vga_color_register[vga_palette_register[i]].g << 2,
            vga_color_register[vga_palette_register[i]].b << 2
        );
    }

    for( int i = 0; i < 16; ++i )
        d->brush[i] = QBrush( d->color[i] );

    for( int i = 0; i < 16; ++i )
    {
        m_screen12.setColor( i, d->color[i].rgb() );
        m_render12.setColor( i, d->color[i].rgb() );
        m_screen0D.setColor( i, d->color[i].rgb() );
        m_render0D.setColor( i, d->color[i].rgb() );
    }
}

static int currentX = 0;
static int currentY = 0;

void
Screen::mouseMoveEvent( QMouseEvent *e )
{
    {
        QMutexLocker l( &d->mouseLock );
        currentX = e->x();
        currentY = e->y();
    }

    QWidget::mouseMoveEvent( e );

    busmouse_event();
}

void
Screen::mousePressEvent( QMouseEvent *e )
{
    {
        QMutexLocker l( &d->mouseLock );
        currentX = e->x();
        currentY = e->y();
    }

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
    {
        QMutexLocker l( &d->mouseLock );
        currentX = e->x();
        currentY = e->y();
    }

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
    QMutexLocker l( &s_self->d->mouseLock );
    return currentX;
}

int
get_current_y()
{
    QMutexLocker l( &s_self->d->mouseLock );
    return currentY;
}

void
Screen::setTinted( bool t )
{
    m_tinted = t;
    repaint();
}

// This sucks, any suggestions?

#include "screen.h"
#include <QHash>
#include <QKeyEvent>
#include <QDebug>
#include <QMutexLocker>

static QHash<int, word> normals;
static QHash<int, word> shifts;
static QHash<int, word> ctrls;
static QHash<int, word> alts;
static QHash<int, byte> makeCode;
static QHash<int, byte> breakCode;
static QHash<int, bool> extended;

static void addKey(int key, word normal, word shift, word ctrl, word alt, bool isExtended = false)
{
    normals.insert(key, normal);
    shifts.insert(key, shift);
    ctrls.insert(key, ctrl);
    alts.insert(key, alt);
    extended.insert(key, isExtended);
    makeCode.insert(key, (normal & 0xFF00) >> 8);
    breakCode.insert(key, ((normal & 0xFF00) >> 8) | 0x80);
}

#define K_A 0x26
#define K_B 0x38
#define K_C 0x36
#define K_D 0x28
#define K_E 0x1A
#define K_F 0x29
#define K_G 0x2A
#define K_H 0x2B
#define K_I 0x1F
#define K_J 0x2C
#define K_K 0x2D
#define K_L 0x2E
#define K_M 0x3A
#define K_N 0x39
#define K_O 0x20
#define K_P 0x21
#define K_Q 0x18
#define K_R 0x1B
#define K_S 0x27
#define K_T 0x1C
#define K_U 0x1E
#define K_V 0x37
#define K_W 0x19
#define K_X 0x35
#define K_Y 0x1D
#define K_Z 0x34

#define K_1 0x0A
#define K_2 0x0B
#define K_3 0x0C
#define K_4 0x0D
#define K_5 0x0E
#define K_6 0x0F
#define K_7 0x10
#define K_8 0x11
#define K_9 0x12
#define K_0 0x13

#define K_F1 0x43
#define K_F2 0x44
#define K_F3 0x45
#define K_F4 0x46
#define K_F5 0x47
#define K_F6 0x48
#define K_F7 0x49
#define K_F8 0x4A
#define K_F9 0x4B
#define K_F10 0x4C
#define K_F11 0x5F
#define K_F12 0x60

#define K_Minus 0x14
#define K_Slash 0x3D

#define K_Up 0x62
#define K_Down 0x68
#define K_Left 0x64
#define K_Right 0x66

#define K_PageUp 0x63
#define K_PageDown 0x69

#define K_LAlt 0x40
#define K_LCtrl 0x25
#define K_LShift 0x32

#define K_RShift 0x3E
#define K_RCtrl 0x69
#define K_RAlt 0x38

#define K_Tab 0x17
#define K_Backspace 0x16
#define K_Return 0x24
#define K_Space 0x41
#define K_Escape 0x09

#define K_Comma 0x3B
#define K_Period 0x3C
#define K_Semicolon 0x2F

#define K_LeftBracket 0x22
#define K_RightBracket 0x23
#define K_Apostrophe 0x30
#define K_Backslash 0x33

void Screen::loadKeymap(const QString &filename)
{
    // Load keymap file
    QDataStream keymap;

    makeCode[K_LShift] = 0x2A;
    makeCode[K_LCtrl] = 0x1D;
    makeCode[K_LAlt] = 0x38;
    makeCode[K_RShift] = 0x36;
    makeCode[K_RCtrl] = 0x1D;
    makeCode[K_RAlt] = 0x38;

    breakCode[K_LShift] = 0xAA;
    breakCode[K_LCtrl] = 0x9D;
    breakCode[K_LAlt] = 0xB8;

    breakCode[K_RShift] = 0xB6;
    breakCode[K_RCtrl] = 0x9D;
    breakCode[K_RAlt] = 0xB8;

    // Windows key == Alt for haxx
    makeCode[0x85] = 0x38;
    breakCode[0x85] = 0xB8;

    addKey( K_A, 0x1E61, 0x1E41, 0x1E01, 0x1E00 );
    addKey( K_B, 0x3062, 0x3042, 0x3002, 0x3000 );
    addKey( K_C, 0x2E63, 0x2E43, 0x2E03, 0x2E00 );
    addKey( K_D, 0x2064, 0x2044, 0x2004, 0x2000 );
    addKey( K_E, 0x1265, 0x1245, 0x1205, 0x1200 );
    addKey( K_F, 0x2166, 0x2146, 0x2106, 0x2100 );
    addKey( K_G, 0x2267, 0x2247, 0x2207, 0x2200 );
    addKey( K_H, 0x2368, 0x2348, 0x2308, 0x2300 );
    addKey( K_I, 0x1769, 0x1749, 0x1709, 0x1700 );
    addKey( K_J, 0x246A, 0x244A, 0x240A, 0x2400 );
    addKey( K_K, 0x256B, 0x254B, 0x250B, 0x2500 );
    addKey( K_L, 0x266C, 0x264C, 0x260C, 0x2600 );
    addKey( K_M, 0x326D, 0x324D, 0x320D, 0x3200 );
    addKey( K_N, 0x316E, 0x314E, 0x310E, 0x3100 );
    addKey( K_O, 0x186F, 0x184F, 0x180F, 0x1800 );
    addKey( K_P, 0x1970, 0x1950, 0x1910, 0x1900 );
    addKey( K_Q, 0x1071, 0x1051, 0x1011, 0x1000 );
    addKey( K_R, 0x1372, 0x1352, 0x1312, 0x1300 );
    addKey( K_S, 0x1F73, 0x1F53, 0x1F13, 0x1F00 );
    addKey( K_T, 0x1474, 0x1454, 0x1414, 0x1400 );
    addKey( K_U, 0x1675, 0x1655, 0x1615, 0x1600 );
    addKey( K_V, 0x2F76, 0x2F56, 0x2F16, 0x2F00 );
    addKey( K_W, 0x1177, 0x1157, 0x1117, 0x1100 );
    addKey( K_X, 0x2D78, 0x2D58, 0x2D18, 0x2D00 );
    addKey( K_Y, 0x1579, 0x1559, 0x1519, 0x1500 );
    addKey( K_Z, 0x2C7A, 0x2C5A, 0x2C1A, 0x2C00 );

    addKey( K_1, 0x0231, 0x0221, 0,      0x7800 );
    addKey( K_2, 0x0332, 0x0340, 0x0300, 0x7900 );
    addKey( K_3, 0x0433, 0x0423, 0,      0x7A00 );
    addKey( K_4, 0x0534, 0x0524, 0,      0x7B00 );
    addKey( K_5, 0x0635, 0x0625, 0,      0x7C00 );
    addKey( K_6, 0x0736, 0x075E, 0x071E, 0x7D00 );
    addKey( K_7, 0x0837, 0x0826, 0,      0x7E00 );
    addKey( K_8, 0x0938, 0x092A, 0,      0x7F00 );
    addKey( K_9, 0x0A39, 0x0a28, 0,      0x8000 );
    addKey( K_0, 0x0B30, 0x0B29, 0,      0x8100 );

    addKey( K_F1, 0x3B00, 0x5400, 0x5E00, 0x6800 );
    addKey( K_F2, 0x3C00, 0x5500, 0x5F00, 0x6900 );
    addKey( K_F3, 0x3D00, 0x5600, 0x6000, 0x6A00 );
    addKey( K_F4, 0x3E00, 0x5700, 0x6100, 0x6B00 );
    addKey( K_F5, 0x3F00, 0x5800, 0x6200, 0x6C00 );
    addKey( K_F6, 0x4000, 0x5900, 0x6300, 0x6D00 );
    addKey( K_F7, 0x4100, 0x5A00, 0x6400, 0x6E00 );
    addKey( K_F8, 0x4200, 0x5B00, 0x6500, 0x6F00 );
    addKey( K_F9, 0x4300, 0x5C00, 0x6600, 0x7000 );
    addKey( K_F10, 0x4400, 0x5D00, 0x6700, 0x7100 );
    addKey( K_F11, 0x8500, 0x8700, 0x8900, 0x8B00 );
    addKey( K_F12, 0x8600, 0x8800, 0x8A00, 0x8C00 );

    addKey( K_Slash, 0x352F, 0x353F, 0, 0 );
    addKey( K_Minus, 0x0C2D, 0x0C5F, 0xC1F, 0x8200 );
    addKey( K_Period, 0x342E, 0x343E, 0, 0 );
    addKey( K_Comma, 0x332C, 0x333C, 0, 0 );
    addKey( K_Semicolon, 0x273B, 0x273A, 0, 0x2700 );

    addKey( K_LeftBracket, 0x1A5B, 0x1A7B, 0x1A1B, 0x1A00 );
    addKey( K_RightBracket, 0x1B5D, 0x1B7D, 0x1B1D, 0x1B00 );
    addKey( K_Apostrophe, 0x2827, 0x2822, 0, 0 );
    addKey( K_Backslash, 0x2B5C, 0x2B7C, 0x2B1C, 0x2600 );

    addKey( K_Tab, 0x0F09, 0x0F00, 0x9400, 0xA500 );
    addKey( K_Backspace, 0x0E08, 0x0E08, 0x0E7F, 0x0E00 );
    addKey( K_Return, 0x1C0D, 0x1C0D, 0x1C0A, 0xA600 );
    addKey( K_Space, 0x3920, 0x3920, 0x3920, 0x3920 );
    addKey( K_Escape, 0x011B, 0x011B, 0x011B, 0x0100 );

    addKey( K_Up, 0x4800, 0x4838, 0x8D00, 0x9800, true );
    addKey( K_Down, 0x5000, 0x5032, 0x9100, 0xA000, true );
    addKey( K_Left, 0x4B00, 0x4B34, 0x7300, 0x9B00, true );
    addKey( K_Right, 0x4D00, 0x4D36, 0x7400, 0x9D00, true );

    addKey( K_PageUp, 0x4900, 0x4B34, 0x7300, 0x9B00 );
    addKey( K_PageDown, 0x5100, 0x5133, 0x7600, 0xA100 );
}

void
Screen::init()
{
    loadKeymap("keymaps/default.vkmap");
}

word
keyToScanCode( Qt::KeyboardModifiers mod, int key )
{
    if( key == Qt::Key_unknown )
        return 0xffff;

    if( mod == Qt::NoModifier )
        return normals[key];

    if( mod & Qt::ShiftModifier )
        return shifts[key];

    if( mod & Qt::AltModifier )
        return alts[key];

    if( mod & Qt::ControlModifier )
        return ctrls[key];

    qDebug() << Q_FUNC_INFO << "Unhandled key" << mod << key;
}

void
Screen::keyPressEvent( QKeyEvent *e )
{
    QMutexLocker l( &d->keyQueueLock );

    qDebug() << "nativeVirtualKey" << e->nativeVirtualKey() << "key" << e->key() << "modifiers" << e->modifiers();

    int nativeScanCode = e->nativeScanCode();
    //printf( "native scancode = %04X\n", nativeScanCode );

    word scancode = keyToScanCode( e->modifiers(), nativeScanCode );

    if( nativeScanCode == K_F11 )
    {
        grabMouse( QCursor( Qt::BlankCursor ));
    }

    if( nativeScanCode == K_F12 )
    {
        releaseMouse();
    }


    if( scancode != 0 )
    {
        d->keyQueue.enqueue( scancode );
        //printf( "Queued %04X (%02X)\n", scancode, e->key() );
    }

    if( extended[nativeScanCode] )
    {
        d->rawQueue.enqueue( 0xE0 );
    }

    d->rawQueue.enqueue( makeCode[nativeScanCode] );

#if 0
    QString s;
    s.sprintf("%02X",makeCode[e->nativeScanCode()]);
    qDebug() << "++++:" << s;
#endif

    irq( 1 );
}

void
Screen::keyReleaseEvent( QKeyEvent *e )
{
    QMutexLocker l( &d->keyQueueLock );

    int nativeScanCode = e->nativeScanCode();

    if( extended[nativeScanCode] )
        d->rawQueue.enqueue( 0xE0 );

    d->rawQueue.enqueue( breakCode[nativeScanCode] );

#if 0
    QString s;
    s.sprintf("%02X",breakCode[e->nativeScanCode()]);
    qDebug() << "----:"<< s;
#endif

    irq( 1 );

    e->ignore();
}

word
Screen::nextKey()
{
    QMutexLocker l( &d->keyQueueLock );

    d->rawQueue.clear();
    if( !d->keyQueue.isEmpty() )
        return d->keyQueue.dequeue();

    return 0;
}

word
Screen::peekKey()
{
    QMutexLocker l( &d->keyQueueLock );

    d->rawQueue.clear();
    if( !d->keyQueue.isEmpty() )
        return d->keyQueue.head();

    return 0;
}

byte
Screen::popKeyData()
{
    QMutexLocker l( &d->keyQueueLock );

    byte key = 0;
    if( !d->rawQueue.isEmpty() )
        key = d->rawQueue.dequeue();
    #if 0
    QString s;
    s.sprintf("%02X", key);
    qDebug() << "pop " << s;
    #endif
    //qDebug() << "Keys queued:" << d->rawQueue.size();
    return key;
}

void
Screen::flushKeyBuffer()
{
    QMutexLocker l( &d->keyQueueLock );

    if( !d->rawQueue.isEmpty() && d->cpu->getIF())
        irq( 1 );
}
