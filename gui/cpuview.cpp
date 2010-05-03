#include "cpuview.h"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QTimer>
#include <QDebug>
#include "../include/vomit.h"
#include "../include/8086.h"
#include "../disasm/include/disasm.h"
#include <QHeaderView>

struct CPUView::Private
{
    QTableWidget *qtw;
};

CPUView::CPUView( QWidget *parent )
    : QWidget( parent ),
      d( new Private )
{
    resize( 200, 300 );

    QVBoxLayout *l = new QVBoxLayout;
    d->qtw = new QTableWidget( 9, 2 );
    l->addWidget( d->qtw );
    l->setSpacing( 0 );
    setLayout( l );

    setWindowTitle( "CPU State" );

    d->qtw->verticalHeader()->hide();
    d->qtw->horizontalHeader()->hide();

    d->qtw->setColumnWidth( 0, 50 );
    d->qtw->setSelectionMode( QAbstractItemView::NoSelection );

    d->qtw->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    d->qtw->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    d->qtw->setAlternatingRowColors( true );

    for( int row = 0; row < d->qtw->rowCount(); ++row )
    {
        for( int column = 0; column < d->qtw->columnCount(); ++column )
        {
            QTableWidgetItem *item = new QTableWidgetItem;

            if( column == 0 )
            {
                QFont f( item->font() );
                f.setBold( true );
                item->setFont( f );
            }
            d->qtw->setItem( row, column, item );
        }
    }

    QTimer::singleShot( 0, this, SLOT(refresh()) );
}

CPUView::~CPUView()
{
    delete d;
    d = 0L;
}

void
CPUView::refresh()
{
    int row = 0;
    QString s;

    d->qtw->item( row, 0 )->setText( "CS:IP" );
    d->qtw->item(row++, 1)->setText(s.sprintf("%04X:%04X", g_cpu->getCS(), g_cpu->getIP()));

    d->qtw->item( row, 0 )->setText( "SS:SP" );
    d->qtw->item( row++, 1 )->setText( s.sprintf( "%04X:%04X", g_cpu->SS, g_cpu->regs.W.SP ));

    d->qtw->item( row, 0 )->setText( "AX" );
    d->qtw->item( row++, 1 )->setText( s.sprintf( "%04X", g_cpu->regs.W.AX ));

    d->qtw->item( row, 0 )->setText( "BX" );
    d->qtw->item( row++, 1 )->setText( s.sprintf( "%04X", g_cpu->regs.W.BX ));

    d->qtw->item( row, 0 )->setText( "CX" );
    d->qtw->item( row++, 1 )->setText( s.sprintf( "%04X", g_cpu->regs.W.CX ));

    d->qtw->item( row, 0 )->setText( "DX" );
    d->qtw->item( row++, 1 )->setText( s.sprintf( "%04X", g_cpu->regs.W.DX ));

    d->qtw->item( row, 0 )->setText( "DS" );
    d->qtw->item( row++, 1 )->setText( s.sprintf( "%04X", g_cpu->DS ));

    d->qtw->item( row, 0 )->setText( "ES" );
    d->qtw->item( row++, 1 )->setText( s.sprintf( "%04X", g_cpu->ES ));

    QTimer::singleShot( 500, this, SLOT(refresh()) );
}
