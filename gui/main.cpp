#include "screen.h"
#include "keyboard.h"
#include "worker.h"
#include <QApplication>
#include <QTimer>

extern "C" {
#include "../include/vomit.h"
}

int
main( int argc, char **argv )
{
	QApplication app( argc, argv );

	vomit_init( argc, argv );

	Keyboard::init();

	Screen s;

	s.setWindowTitle( "VOMIT" );
	s.show();

	QTimer syncTimer;
	QObject::connect( &syncTimer, SIGNAL( timeout() ), &s, SLOT( update() ));
	syncTimer.start( 50 );

	Worker w;

	QObject::connect( &w, SIGNAL( finished() ), &s, SLOT( close() ));

	return app.exec();
}

word
kbd_getc()
{
	return Keyboard::nextKey();
}

word
kbd_hit()
{
	return Keyboard::peekKey();
}
