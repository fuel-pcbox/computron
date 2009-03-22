#include "screen.h"
#include "worker.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QTimer>
#include <QSplitter>
//#include "cpuview.h"
#include "console.h"

extern "C" {
#include "../include/vomit.h"
}

Screen *scr = 0L;

int
main( int argc, char **argv )
{
	QApplication app( argc, argv );

	Console *console = new Console;
	//console->show();

	int rc = vomit_init( argc, argv );

	if( rc != 0 )
	{
		fprintf( stderr, "vomit_init() returned %d\n", rc );
		return rc;
	}

	QWidget *win = new QWidget;
	win->setWindowTitle( "VOMIT" );

	QVBoxLayout *l = new QVBoxLayout;
	l->setSpacing( 0 );
	l->setMargin( 0 );

	scr = new Screen;

	l->addWidget( scr );
	//l->addWidget( activityBar );
	l->addWidget( console );

	win->setLayout( l );
	scr->setFocus();
	win->show();

//	CPUView *cpuView = new CPUView;
//	cpuView->show();

	QTimer syncTimer;
	QObject::connect( &syncTimer, SIGNAL( timeout() ), scr, SLOT( refresh() ));
	syncTimer.start( 50 );

	QTimer flushKeyboardTimer;
	QObject::connect( &flushKeyboardTimer, SIGNAL( timeout() ), scr, SLOT( flushKeyBuffer() ));
	flushKeyboardTimer.start( 50 );

	Worker w;
	w.start();

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

byte
kbd_pop_raw()
{
	return scr->popKeyData();
}

