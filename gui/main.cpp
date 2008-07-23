#include "screen.h"
#include "worker.h"
#include <QApplication>
#include <QTimer>

extern "C" {
#include "../include/vomit.h"
}

Screen *scr = 0L;

int
main( int argc, char **argv )
{
	QApplication app( argc, argv );

	vomit_init( argc, argv );

	scr = new Screen;

	scr->setWindowTitle( "VOMIT" );
	scr->show();

	QTimer syncTimer;
	QObject::connect( &syncTimer, SIGNAL( timeout() ), scr, SLOT( refresh() ));
	syncTimer.start( 500 );

	Worker w;

	QObject::connect( &w, SIGNAL( finished() ), scr, SLOT( close() ));

	return app.exec();
}

word
kbd_getc()
{
	return scr->nextKey();
}

word
kbd_hit()
{
	return scr->peekKey();
}
