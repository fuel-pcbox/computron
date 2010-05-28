#ifndef __mainwindow_h__
#define __mainwindow_h__

#include <QtGui/QMainWindow>

#include "vomit.h"

class Screen;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(VCpu *);
    ~MainWindow();

    void setCpu(VCpu *);
    VCpu *cpu();
    Screen *screen();

private slots:
    void slotFloppyAClicked();
    void slotFloppyBClicked();
    void slotPauseMachine();
    void slotStartMachine();
    void slotStopMachine();
    void slotRebootMachine();

    void onAboutToQuit();

private:
    struct Private;
    Private *d;
};

#endif // __mainwindow_h__
