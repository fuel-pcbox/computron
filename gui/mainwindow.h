#ifndef __mainwindow_h__
#define __mainwindow_h__

#include <QMainWindow>

class Screen;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow();
	~MainWindow();

	Screen *screen();

private slots:
	void slotFloppyAClicked();
	void slotFloppyBClicked();
	void slotPauseMachine();
	void slotStartMachine();
	void slotStopMachine();

	void slotUpdateMemView();

private:
	struct Private;
	Private *d;
};

#endif // __mainwindow_h__
