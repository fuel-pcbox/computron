#ifndef __worker_h__
#define __worker_h__

#include <QObject>
#include <QTimer>

extern "C" {
#include "../include/vomit.h"
}

class Worker : public QObject
{
	Q_OBJECT
public:
	Worker();
	~Worker();

signals:
	void finished();

public slots:
	void work();
	void shutdown();

private:
	QTimer *m_workTimer;
};

#endif // __worker_h__
