#ifndef __codeview_h__
#define __codeview_h__

#include <QWidget>
#include <stdint.h>

class CodeView : public QWidget
{
	Q_OBJECT
public:
	CodeView( QWidget *parent = 0L );
	~CodeView();

	QSize sizeHint() const;

public slots:
	void setAddress( uint16_t segment, uint16_t offset );

protected:
	void paintEvent( QPaintEvent * );

private:
	struct Private;
	Private *d;
};

#endif
