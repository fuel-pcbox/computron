#include "mainwindow.h"
#include "worker.h"
#include "console.h"
#include "screen.h"
#include "memview.h"
#include "codeview.h"
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QTimer>
#include <QTabWidget>
#include <QLabel>
#include <QDebug>
#include "hexspinbox.h"
#include <QApplication>

void vomit_set_drive_image(int drive_id, const char *filename);

struct MainWindow::Private
{
    QToolBar *mainToolBar;

    QAction *startMachine;
    QAction *pauseMachine;
    QAction *stopMachine;

    Worker *worker;
    Screen *screen;
    Console *console;
    MemoryView memview;
    CodeView codeview;

    HexSpinBox *segmentEdit;
    HexSpinBox *offsetEdit;

    QTimer syncTimer;

    VCpu *cpu;
};

MainWindow::MainWindow(VCpu *cpu)
    : d(new Private)
{
    d->worker = 0;
    d->console = 0;
    d->screen = new Screen(cpu);

    setCpu(cpu);

    setWindowTitle( "Vomit" );
    setUnifiedTitleAndToolBarOnMac(true);
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    QWidget *widget = new QWidget( this );

    QVBoxLayout *l = new QVBoxLayout;
    l->setSpacing( 0 );
    l->setMargin( 0 );
    widget->setLayout( l );
    l->addWidget(d->screen);
    //l->addWidget( activityBar );

#if 0
    QTabWidget *tabs = new QTabWidget;
    tabs->setTabPosition( QTabWidget::South );
    l->addWidget( tabs );

    tabs->addTab(d->console, tr("Console"));

    QWidget *memTab = new QWidget;
    QVBoxLayout *ml = new QVBoxLayout;
    ml->setSpacing( 0 );
    ml->setMargin( 0 );

    d->segmentEdit = new HexSpinBox;
    d->offsetEdit = new HexSpinBox;

    QHBoxLayout *mhl = new QHBoxLayout;
    mhl->addWidget( new QLabel( tr("Segment:") ));
    mhl->addWidget( d->segmentEdit );
    mhl->addWidget( new QLabel( tr("Offset:") ));
    mhl->addWidget( d->offsetEdit );
    ml->addLayout( mhl );

    connect( d->segmentEdit, SIGNAL(valueChanged(int)), SLOT(slotUpdateMemView()) );
    connect( d->offsetEdit, SIGNAL(valueChanged(int)), SLOT(slotUpdateMemView()) );

    ml->addWidget( &d->memview );

    memTab->setLayout( ml );
    tabs->addTab( memTab, tr("Memory") );

    tabs->addTab( &d->codeview, tr("Code") );

    //tabs->setCurrentWidget( &d->codeview );
#endif

    d->screen->setFocus();

    setCentralWidget( widget );

    QAction *chooseFloppyAImage = new QAction(QIcon(":/icons/disk.png"), tr("Floppy A:"), this);
    QAction *chooseFloppyBImage = new QAction(QIcon(":/icons/disk.png"), tr("Floppy B:"), this);
    d->pauseMachine = new QAction(QIcon(":/icons/control_pause.png"), tr("Pause"), this);
    d->startMachine = new QAction(QIcon(":/icons/control_play.png"), tr("Start"), this);
    d->stopMachine = new QAction(QIcon(":/icons/control_stop.png"), tr("Stop"), this);

    QAction *rebootMachine = new QAction(QIcon(":/icons/arrow_refresh.png"), tr("Reboot"), this);

    d->startMachine->setEnabled( false );
    d->pauseMachine->setEnabled( true );
    d->stopMachine->setEnabled( true );

    d->mainToolBar = addToolBar( tr("Virtual Machine") );

    d->mainToolBar->addAction( d->startMachine );
    d->mainToolBar->addAction( d->pauseMachine );
    d->mainToolBar->addAction( d->stopMachine );
    d->mainToolBar->addAction( rebootMachine );

    d->mainToolBar->addSeparator();

    d->mainToolBar->addAction( chooseFloppyAImage );
    d->mainToolBar->addAction( chooseFloppyBImage );

    connect( chooseFloppyAImage, SIGNAL(triggered(bool)), SLOT(slotFloppyAClicked()) );
    connect( chooseFloppyBImage, SIGNAL(triggered(bool)), SLOT(slotFloppyBClicked()) );

    connect( rebootMachine, SIGNAL(triggered(bool)), SLOT(slotRebootMachine()) );

    connect( d->pauseMachine, SIGNAL(triggered(bool)), SLOT(slotPauseMachine()) );
    connect( d->startMachine, SIGNAL(triggered(bool)), SLOT(slotStartMachine()) );
    connect( d->stopMachine, SIGNAL(triggered(bool)), SLOT(slotStopMachine()) );

    QObject::connect( &d->syncTimer, SIGNAL( timeout() ), d->screen, SLOT( refresh() ));
    QObject::connect( &d->syncTimer, SIGNAL( timeout() ), d->screen, SLOT( flushKeyBuffer() ));
    d->syncTimer.start( 50 );

    QObject::connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(onAboutToQuit()));
}

MainWindow::~MainWindow()
{
    delete d;
    d = 0L;
}

void MainWindow::slotFloppyAClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Choose floppy B image")
    );
    if (fileName.isNull())
        return;
    vomit_set_drive_image(0, qPrintable(fileName));
}

void MainWindow::slotFloppyBClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Choose floppy B image")
    );
    if (fileName.isNull())
        return;
    vomit_set_drive_image(1, qPrintable(fileName));
}

void MainWindow::slotPauseMachine()
{
    d->pauseMachine->setEnabled(false);
    d->startMachine->setEnabled(true);
    d->stopMachine->setEnabled(true);

    d->screen->setTinted(true);

    Q_ASSERT(d->worker);
    d->worker->stopMachine();
}

void MainWindow::slotStopMachine()
{
    d->pauseMachine->setEnabled(false);
    d->startMachine->setEnabled(true);
    d->stopMachine->setEnabled(false);

    d->screen->setTinted(true);

    Q_ASSERT(d->worker);
    d->worker->stopMachine();
}

void MainWindow::slotStartMachine()
{
    d->pauseMachine->setEnabled(true);
    d->startMachine->setEnabled(false);
    d->stopMachine->setEnabled(true);

    d->screen->setTinted(false);

    Q_ASSERT(d->worker);
    d->worker->startMachine();
}

void MainWindow::slotRebootMachine()
{
    Q_ASSERT(d->worker);
    d->worker->rebootMachine();
}

void MainWindow::setCpu(VCpu *cpu)
{
    d->cpu = cpu;

    delete d->worker;

    d->worker = new Worker(d->cpu, this);
    QObject::connect( d->worker, SIGNAL( finished() ), this, SLOT( close() ));
    d->worker->startMachine();
    d->worker->start();
}

Screen *MainWindow::screen()
{
    return d->screen;
}

VCpu *MainWindow::cpu()
{
    return d->cpu;
}

void MainWindow::slotUpdateMemView()
{
    d->memview.setAddress(d->segmentEdit->value(), d->offsetEdit->value());
}

void MainWindow::onAboutToQuit()
{
    d->worker->stopMachine();
}
