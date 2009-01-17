// This sucks, any suggestions?

#include "screen.h"
#include <QMap>
#include <QKeyEvent>
#include <QDebug>

static QMap<int, word> normals;
static QMap<int, word> shifts;
static QMap<int, word> ctrls;
static QMap<int, word> alts;
static QMap<int, byte> makeCode;
static QMap<int, byte> breakCode;
static QMap<int, bool> extended;

void
addKey( int key, word normal, word shift, word ctrl, word alt, bool isExtended = false )
{
	normals[key] = normal;
	shifts[key] = shift;
	ctrls[key] = ctrl;
	alts[key] = alt;

	extended[key] = isExtended;

	makeCode[key] = (normal & 0xFF00) >> 8;
	breakCode[key] = ((normal & 0xFF00) >> 8) | 0x80;
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

#define K_Up 0x6F
#define K_Down 0x74
#define K_Left 0x71
#define K_Right 0x72

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

void
Screen::init()
{
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

	//grabKeyboard();
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

	//printf( "EH!\n" );
}

void
Screen::keyPressEvent( QKeyEvent *e )
{
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
		m_keyQueue.enqueue( scancode );
		//printf( "Queued %04X (%02X)\n", scancode, e->key() );
	}

	if( extended[nativeScanCode] )
	{
		m_rawQueue.enqueue( 0xE0 );
	}

	m_rawQueue.enqueue( makeCode[nativeScanCode] );

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
	int nativeScanCode = e->nativeScanCode();

	if( extended[nativeScanCode] )
		m_rawQueue.enqueue( 0xE0 );

	m_rawQueue.enqueue( breakCode[nativeScanCode] );

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
	m_rawQueue.clear();
	if( !m_keyQueue.isEmpty() )
		return m_keyQueue.dequeue();

	return 0;
}

word
Screen::peekKey()
{
	m_rawQueue.clear();
	if( !m_keyQueue.isEmpty() )
		return m_keyQueue.head();

	return 0;
}

byte
Screen::popKeyData()
{
	byte key = 0;
	if( !m_rawQueue.isEmpty() )
		key = m_rawQueue.dequeue();
	#if 0
	QString s;
	s.sprintf("%02X", key);
	qDebug() << "pop " << s;
	#endif
	//qDebug() << "Keys queued:" << m_rawQueue.size();
	return key;
}

void
Screen::flushKeyBuffer()
{
	if( !m_rawQueue.isEmpty() && cpu.IF )
		irq( 1 );
}
