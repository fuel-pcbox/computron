#include "worker.h"
#include <QMutexLocker>
#include <QDebug>
#include "../include/vomit.h"
extern unsigned int g_vomit_exit_main_loop;
extern bool vomit_reboot;

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
	while( m_active )
	{
		cpu_main();
		while( !m_active )
			msleep( 50 );
	}
}

void
Worker::shutdown()
{
	vm_exit( 0 );
	emit finished();
}

void
Worker::startMachine()
{
	g_vomit_exit_main_loop = false;
	m_active = true;
}

void
Worker::stopMachine()
{
	m_active = false;
	g_vomit_exit_main_loop = true;
}

void
Worker::rebootMachine()
{
	// :(
}
