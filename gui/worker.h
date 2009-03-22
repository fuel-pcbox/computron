#ifndef __worker_h__
#define __worker_h__

#include <QThread>

class Worker : public QThread
{
	Q_OBJECT
public:
	Worker( QObject *parent = 0L );
	~Worker();

signals:
	void finished();

public slots:
	void run();
	void shutdown();
};

#endif
