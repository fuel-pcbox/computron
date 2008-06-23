#include "worker.h"
#include <QTimer>

extern "C" {
#include "../include/vomit.h"
}

Worker::Worker()
{
	m_workTimer = new QTimer;
	connect( m_workTimer, SIGNAL( timeout() ), this, SLOT( work() ));
	m_workTimer->start( 0 );
}

Worker::~Worker()
{
	delete m_workTimer;
}

void
Worker::work()
{
	for( int i= 0; i < 10000; ++i )
		cpu_main();
}

void
Worker::shutdown()
{
	vm_exit( 0 );
	emit finished();
}
