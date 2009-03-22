#include "worker.h"

extern "C" {
#include "../include/vomit.h"
}

Worker::Worker( QObject *parent )
	: QThread( parent )
{
}

Worker::~Worker()
{
}

void
Worker::run()
{
	cpu_main();
}

void
Worker::shutdown()
{
	vm_exit( 0 );
	emit finished();
}
