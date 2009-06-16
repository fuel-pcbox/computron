#ifndef __hexspinbox_h__
#define __hexspinbox_h__

#include <QSpinBox>

class HexSpinBox : public QSpinBox
{
	Q_OBJECT
public:
	HexSpinBox( QWidget *parent = 0L );
	virtual ~HexSpinBox();

protected:
	virtual QString textFromValue( int value ) const;
	virtual int valueFromText( const QString &text ) const;

	virtual QValidator::State validate( QString &input, int &pos ) const;
};

#endif
