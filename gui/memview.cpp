#include "memview.h"
#include <QPainter>
#include <QTimer>

#include "../include/vomit.h"
#include "../include/8086.h"

struct MemoryView::Private
{
	uint16_t segment;
	uint16_t offset;

	QTimer syncTimer;
};

MemoryView::MemoryView( QWidget *parent )
	: QWidget( parent ),
	  d( new Private )
{
	d->segment = 0;
	d->offset = 0;

	connect( &d->syncTimer, SIGNAL(timeout()), SLOT(update()) );

	d->syncTimer.start( 1000 );
}

MemoryView::~MemoryView()
{
	delete d;
	d = 0L;
}

void
MemoryView::setAddress( uint16_t segment, uint16_t offset )
{
	d->segment = segment;
	d->offset = offset;
	update();
}

void
MemoryView::paintEvent( QPaintEvent * )
{
	QPainter p( this );

	p.fillRect( rect(), Qt::white );

	int ptr = 0;

	QFontMetrics fm(p.font());
	int textY = fm.height();

	p.setFont( QFont( "Courier New" ));

	for( int y = 0; y < 16; ++y )
	{
		QString line;

		QString plain;

		{
			QString s;
			s.sprintf( "%04X:%04X  ", d->segment, d->offset + ptr );
			line += s;
		}

		for( int x = 0; x < 16; ++x )
		{
			uint8_t b = vomit_cpu_memory_read8(&g_cpu, d->segment, d->offset + ptr);
			ptr++;

			QString s;
			s.sprintf( "%02X ", b );
			line += s;

			QChar ch = QChar::fromAscii( b );
			if( ch.isPrint() )
				plain += ch;
			else
				plain += QChar('.');
		}
		p.drawText( 0, textY, QString("%1 %2").arg(line).arg(plain) );

		textY += fm.height();
	}
}

QSize
MemoryView::sizeHint() const
{
	return QSize( 600, 270 );
}
