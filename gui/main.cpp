#include "mainwindow.h"
#include <QApplication>
#include "screen.h"
//#include "cpuview.h"

extern "C" {
#include "../include/vomit.h"
}

static MainWindow *mw = 0L;

int
main( int argc, char **argv )
{
	QApplication app( argc, argv );

	int rc = vomit_init( argc, argv );

	if( rc != 0 )
	{
		fprintf( stderr, "vomit_init() returned %d\n", rc );
		return rc;
	}

	mw = new MainWindow;

	mw->show();

//	CPUView *cpuView = new CPUView;
//	cpuView->show();

	return app.exec();
}

word
kbd_getc()
{
	if( !mw || !mw->screen() )
		return 0x0000;
	return mw->screen()->nextKey();
}

word
kbd_hit()
{
	if( !mw || !mw->screen() )
		return 0x0000;
	return mw->screen()->peekKey();
}

byte
kbd_pop_raw()
{
	if( !mw || !mw->screen() )
		return 0x00;
	return mw->screen()->popKeyData();
}

