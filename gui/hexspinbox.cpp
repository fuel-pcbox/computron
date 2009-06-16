#include "hexspinbox.h"

HexSpinBox::HexSpinBox( QWidget *parent )
	: QSpinBox( parent )
{
	setMinimum( 0x0000 );
	setMaximum( 0xFFFF );
	setValue( 0x0000 );
}

HexSpinBox::~HexSpinBox()
{
}

QString
HexSpinBox::textFromValue( int value ) const
{
	QString s;
	s.sprintf( "%04X", value );
	return s;
}

int
HexSpinBox::valueFromText( const QString &text ) const
{
	return text.toUInt( 0L, 16 );
}

QValidator::State
HexSpinBox::validate( QString &input, int &pos ) const
{
	bool b;
	int foo = input.toUInt( &b, 16 );
	return b ? QValidator::Acceptable : QValidator::Invalid;
}
