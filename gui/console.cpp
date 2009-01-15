#include "console.h"
#include <QVBoxLayout>
#include <QTimer>
#include <QDebug>
#include <QApplication>
extern "C" {
#include "../include/vomit.h"
#include "../include/8086.h"
#include "../disasm/include/disasm.h"
}
#include <QTextEdit>
#include <QLineEdit>

struct Console::Private
{
	QTextEdit *textEdit;
	QLineEdit *lineEdit;
};

int
Console::dump_disasm( unsigned int segment, unsigned int offset )
{
	char disasm[64];
	int width, i;
	char buf[512];
	char *p = buf;
	byte *opcode;

	opcode = mem_space + (segment << 4) + offset;
	width = insn_width( opcode );
	disassemble( opcode, offset, disasm, sizeof(disasm) );

	p += sprintf( p, "%04X:%04X ", segment, offset );

	for( i = 0; i < (width ? width : 7); ++i )
	{
		p += sprintf( p, "%02X", opcode[i] );
	}
	for( i = 0; i < (14-((width?width:7)*2)); ++i )
	{
		p += sprintf( p, " " );
	}

	p += sprintf( p, " %s", disasm );

	//append( buf );

	/* Recurse if this is a prefix instruction. */
	if( *opcode == 0x26 || *opcode == 0x2E || *opcode == 0x36 || *opcode == 0x3E || *opcode == 0xF2 || *opcode == 0xF3 )
		width += dump_disasm( segment, offset + width );

	return width;
}

static QTextEdit *textEdit;

void
vlog_hook( int category, const char *format, va_list ap )
{
	const char *prefix = 0L;
	char buf[8192];

	QString s;

	switch( category )
	{
		case VM_INITMSG: prefix = "init"; break;
		case VM_DISKLOG: prefix = "disk"; break;
		case VM_KILLMSG: prefix = "kill"; break;
		case VM_IOMSG:   prefix = "i/o"; break;
		case VM_ALERT:   prefix = "alert"; break;
		case VM_PRNLOG:  prefix = "lpt"; break;
		case VM_VIDEOMSG: prefix = "video"; break;
		case VM_CONFIGMSG: prefix = "config"; break;
		case VM_CPUMSG:  prefix = "cpu"; break;
		case VM_MEMORYMSG: prefix = "memory"; break;
		case VM_MOUSEMSG: prefix = "mouse"; break;
		case VM_DOSMSG: prefix = "dos"; break;
		case VM_PICMSG: prefix = "pic"; break;
		case VM_DMAMSG: prefix = "dma";  break;
		case VM_KEYMSG: prefix = "keyb"; break;
	}

	if( prefix )
	{
		s.sprintf( "<font color='green'><b>(%s)</b></font> ", prefix );
	}

	vsnprintf( buf, sizeof(buf), format, ap );
	s += buf;
	textEdit->append( s );
	textEdit->moveCursor( QTextCursor::End );
	textEdit->ensureCursorVisible();
}

#if 0
void
uasm( word seg, word off, int n )
{
	int i;
	for( i = 0; i < n; ++i )
	{
		int w = dump_disasm( seg, off );
		if( !w )
			break;
		off += w;
	}
}
#endif


Console::Console( QWidget *parent )
	: QWidget( parent ),
	  d( new Private )
{
	vomit_set_vlog_handler( vlog_hook );

	resize( 600, 400 );
	move( 700, 200 );

	QVBoxLayout *l = new QVBoxLayout;
	d->textEdit = new QTextEdit;
	QFont f = d->textEdit->font();
	f.setFamily( "monospace" );
	d->textEdit->setFont( f );
	d->textEdit->setReadOnly( true );
	d->textEdit->setFocusPolicy( Qt::NoFocus );
	textEdit = d->textEdit;
	d->lineEdit = new QLineEdit;
	d->lineEdit->setFont( f );
	d->lineEdit->setFocusPolicy( Qt::ClickFocus );
	l->addWidget( d->textEdit );
	l->addWidget( d->lineEdit );
	l->setSpacing( 0 );
	setLayout( l );

	connect( d->lineEdit, SIGNAL(returnPressed()), this, SLOT(execute()) );

	setWindowTitle( "Console" );

	QTimer::singleShot( 0, this, SLOT(refresh()) );
}

Console::~Console()
{
	delete d;
	d = 0L;
}

void
Console::append( const QString &s )
{
	d->textEdit->append( s );
	d->textEdit->moveCursor( QTextCursor::End );
	d->textEdit->ensureCursorVisible();
}

void
Console::refresh()
{
	/*dump_disasm( cpu.CS, cpu.IP );
	QTimer::singleShot( 500, this, SLOT(refresh()) );*/
}

void
Console::execute()
{
	QString cmd = d->lineEdit->text();
	d->lineEdit->clear();

	append( QString("<b>&gt; %1</b> ").arg(cmd) );

	QStringList parts = cmd.split(" ");

	if( parts[0] == "help" || parts[0] == "?" )
	{
		append( "I wish I could help you." );
	}
	else if( parts[0] == "r" || parts[0] == "registers" )
	{
		dump_all();
	}
	else if( parts[0] == "ivt" )
	{
		dump_ivt();
	}
	else if( parts[0] == "kill" )
	{
		QApplication::quit();
	}
	else if( parts[0] == "reconf" )
	{
		config_reload();
	}
	else if( parts[0] == "cpu" )
	{
		dump_cpu();
	}
	else if( parts[0] == "byte" )
	{
		QStringList address = parts[1].split(":");
	}
	else if( parts[0] == "leds" )
	{
		byte led1 = mem_space[0x417];
		byte led2 = mem_space[0x418];
		QString s;
		s.sprintf( "Keyboard flags: %02X %02X", led1, led2 );
		append( s );

		s.sprintf( "Num lock: %s", led1 & 0x20 ? "on" : "off" );
		append( s );
		s.sprintf( "Caps lock: %s", led1 & 0x40 ? "on" : "off" );
		append( s );
		s.sprintf( "Insert: %s", led1 & 0x80 ? "on" : "off" );
		append( s );
		s.sprintf( "Scroll lock: %s", led1 & 0x10 ? "on" : "off" );
		append( s );
		s.sprintf( "Right shift: %s", led1 & 0x01 ? "on" : "off" );
		append( s );
		s.sprintf( "Left shift: %s", led1 & 0x02 ? "on" : "off" );
		append( s );
		s.sprintf( "Right CTRL: %s", led1 & 0x04 ? "on" : "off" );
		append( s );
		s.sprintf( "Right ALT: %s", led1 & 0x08 ? "on" : "off" );
		append( s );

		s.sprintf( "Left CTRL: %s", led2 & 0x01 ? "on" : "off" );
		append( s );
		s.sprintf( "Left ALT: %s", led2 & 0x02 ? "on" : "off" );
		append( s );
		s.sprintf( "System: %s", led2 & 0x04 ? "on" : "off" );
		append( s );
		s.sprintf( "Suspend: %s", led2 & 0x08 ? "on" : "off" );
		append( s );

	}
	else
	{
		append( "Unknown command." );
	}
}
