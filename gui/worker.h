#ifndef __worker_h__
#define __worker_h__

#include <QThread>
#include <QMutex>

#include "vomit.h"

class Worker : public QThread
{
    Q_OBJECT
public:
    Worker(VCpu *cpu, QObject *parent = 0);
    ~Worker();

    void startMachine();
    void stopMachine();
    void rebootMachine();

public slots:
    void run();
    void shutdown();

private:
    struct Private;
    Private *d;
};

#endif
