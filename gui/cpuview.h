#ifndef __cpuview_h__
#define __cpuview_h__

#include <QWidget>

class CPUView : public QWidget
{
	Q_OBJECT
public:
	CPUView( QWidget *parent = 0L );
	~CPUView();

public slots:
	void refresh();

private:
	struct Private;
	Private *d;
};

#endif // __cpuview_h__
