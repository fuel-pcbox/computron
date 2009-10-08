#ifndef __worker_h__
#define __worker_h__

#include <QThread>
#include <QMutex>

class Worker : public QThread
{
	Q_OBJECT
public:
	Worker( QObject *parent = 0L );
	~Worker();

	void startMachine();
	void stopMachine();
	void rebootMachine();

public slots:
	void run();
	void shutdown();

private:
	QMutex m_mutex;
	bool m_active;
};

#endif
