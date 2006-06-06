// This sucks, any suggestions?

#include "keyboard.h"

Keyboard *Keyboard::s_self = 0L;

QMap<int, word> normals;
QMap<int, word> shifts;
QMap<int, word> ctrls;
QMap<int, word> alts;

void
addKey( int key, word normal, word shift, word ctrl, word alt )
{
	normals[key] = normal;
	shifts[key] = shift;
	ctrls[key] = ctrl;
	alts[key] = alt;
}

void
Keyboard::init()
{
	if( s_self != 0L )
	{
		return;
	}

	s_self = new Keyboard;

	addKey( Qt::Key_A, 0x1E61, 0x1E41, 0x1E01, 0x1E00 );
	addKey( Qt::Key_B, 0x3062, 0x3042, 0x3002, 0x3000 );
	addKey( Qt::Key_C, 0x2E63, 0x2E42, 0x2E03, 0x2E00 );
	addKey( Qt::Key_D, 0x2064, 0x2044, 0x2004, 0x2000 );
	addKey( Qt::Key_E, 0x1265, 0x1245, 0x1205, 0x1200 );
	addKey( Qt::Key_F, 0x2166, 0x2146, 0x2106, 0x2100 );
	addKey( Qt::Key_G, 0x2267, 0x2247, 0x2207, 0x2200 );
	addKey( Qt::Key_H, 0x2368, 0x2348, 0x2308, 0x2300 );
	addKey( Qt::Key_I, 0x1769, 0x1749, 0x1709, 0x1700 );
	addKey( Qt::Key_J, 0x246A, 0x244A, 0x240A, 0x2400 );
	addKey( Qt::Key_K, 0x256B, 0x254B, 0x250B, 0x2500 );
	addKey( Qt::Key_L, 0x266C, 0x264C, 0x260C, 0x2600 );
	addKey( Qt::Key_M, 0x326D, 0x324D, 0x320D, 0x3200 );
	addKey( Qt::Key_N, 0x316E, 0x314E, 0x310E, 0x3100 );
	addKey( Qt::Key_O, 0x186F, 0x184F, 0x180F, 0x1800 );
	addKey( Qt::Key_P, 0x1970, 0x1950, 0x1910, 0x1900 );
	addKey( Qt::Key_Q, 0x1071, 0x1051, 0x1011, 0x1000 );
	addKey( Qt::Key_R, 0x1372, 0x1352, 0x1312, 0x1300 );
	addKey( Qt::Key_S, 0x1F73, 0x1F53, 0x1F13, 0x1F00 );
	addKey( Qt::Key_T, 0x1474, 0x1454, 0x1414, 0x1400 );
	addKey( Qt::Key_U, 0x1675, 0x1655, 0x1615, 0x1600 );
	addKey( Qt::Key_V, 0x2F76, 0x2F56, 0x2F16, 0x2F00 );
	addKey( Qt::Key_W, 0x1177, 0x1157, 0x1117, 0x1100 );
	addKey( Qt::Key_X, 0x2D78, 0x2D58, 0x2D18, 0x2D00 );
	addKey( Qt::Key_Y, 0x1579, 0x1559, 0x1519, 0x1500 );
	addKey( Qt::Key_Z, 0x2C7A, 0x2C5A, 0x2C1A, 0x2C00 );

	addKey( Qt::Key_1, 0x0231, 0x0221, 0,      0x7800 );
	addKey( Qt::Key_2, 0x0332, 0x0340, 0x0300, 0x7900 );
	addKey( Qt::Key_3, 0x0433, 0x0423, 0,      0x7A00 );
	addKey( Qt::Key_4, 0x0534, 0x0524, 0,      0x7B00 );
	addKey( Qt::Key_5, 0x0635, 0x0625, 0,      0x7C00 );
	addKey( Qt::Key_6, 0x0736, 0x075E, 0x071E, 0x7D00 );
	addKey( Qt::Key_7, 0x0837, 0x0826, 0,      0x7E00 );
	addKey( Qt::Key_8, 0x0938, 0x092A, 0,      0x7F00 );
	addKey( Qt::Key_9, 0x0A39, 0x0a28, 0,      0x8000 );
	addKey( Qt::Key_0, 0x0B30, 0x0B29, 0,      0x8100 );

	addKey( Qt::Key_F1, 0x3B00, 0x5400, 0x5E00, 0x6800 );
	addKey( Qt::Key_F2, 0x3C00, 0x5500, 0x5F00, 0x6900 );
	addKey( Qt::Key_F3, 0x3D00, 0x5600, 0x6000, 0x6A00 );
	addKey( Qt::Key_F4, 0x3E00, 0x5700, 0x6100, 0x6B00 );
	addKey( Qt::Key_F5, 0x3F00, 0x5800, 0x6200, 0x6C00 );
	addKey( Qt::Key_F6, 0x4000, 0x5900, 0x6300, 0x6D00 );
	addKey( Qt::Key_F7, 0x4100, 0x5A00, 0x6400, 0x6E00 );
	addKey( Qt::Key_F8, 0x4200, 0x5B00, 0x6500, 0x6F00 );
	addKey( Qt::Key_F9, 0x4300, 0x5C00, 0x6600, 0x7000 );
	addKey( Qt::Key_F10, 0x4400, 0x5D00, 0x6700, 0x7100 );
	addKey( Qt::Key_F11, 0x8500, 0x8700, 0x8900, 0x8B00 );
	addKey( Qt::Key_F12, 0x8600, 0x8800, 0x8A00, 0x8C00 );

	addKey( Qt::Key_Minus, 0x352F, 0x353F, 0, 0 );
	addKey( Qt::Key_Plus, 0x0C2D, 0x0C5F, 0xC1F, 0x8200 );
	addKey( Qt::Key_Period, 0x342E, 0x343E, 0, 0 );
	addKey( Qt::Key_Comma, 0x332C, 0x333C, 0, 0 );

	addKey( Qt::Key_Tab, 0x0F09, 0x0F00, 0x9400, 0xA500 );
	addKey( Qt::Key_Backspace, 0x0E08, 0x0E08, 0x0E7F, 0x0E00 );
	addKey( Qt::Key_Return, 0x1C0D, 0x1C0D, 0x1C0A, 0xA600 );
	addKey( Qt::Key_Space, 0x3920, 0x3920, 0x3920, 0x3920 );
	addKey( Qt::Key_Escape, 0x011B, 0x011B, 0x011B, 0x0100 );

	addKey( Qt::Key_Up, 0x4800, 0x4838, 0x8D00, 0x9800 );
	addKey( Qt::Key_Down, 0x5000, 0x5032, 0x9100, 0xA000 );
	addKey( Qt::Key_Left, 0x4B00, 0x4B34, 0x7300, 0x9B00 );
	addKey( Qt::Key_Right, 0x4D00, 0x4D36, 0x7400, 0x9D00 );

	addKey( Qt::Key_PageUp, 0x4900, 0x4B34, 0x7300, 0x9B00 );
	addKey( Qt::Key_PageDown, 0x5100, 0x5133, 0x7600, 0xA100 );

	s_self->grabKeyboard();
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

	printf( "EH!\n" );
}

void
Keyboard::keyPressEvent( QKeyEvent *e )
{
	word scancode = keyToScanCode( e->modifiers(), e->key() );

	if( scancode != 0 )
	{
		m_keyQueue.enqueue( scancode );
//		printf( "Queued %04X (%02X)\n", scancode, e->key() );
	}
}

void
Keyboard::keyReleaseEvent( QKeyEvent *e )
{
	e->ignore();
}

word
Keyboard::nextKey()
{
	if( !s_self->m_keyQueue.isEmpty() )
		return s_self->m_keyQueue.dequeue();

	return 0;
}

word
Keyboard::peekKey()
{
	if( !s_self->m_keyQueue.isEmpty() )
		return s_self->m_keyQueue.head();

	return 0;
}

Keyboard::Keyboard() {}
Keyboard::~Keyboard() {}
