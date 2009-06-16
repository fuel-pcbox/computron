#ifndef __memview_h__
#define __memview_h__

#include <QWidget>
#include <stdint.h>

class MemoryView : public QWidget
{
	Q_OBJECT
public:
	MemoryView( QWidget *parent = 0L );
	~MemoryView();

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
