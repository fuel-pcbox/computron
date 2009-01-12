#ifndef __console_h__
#define __console_h__

#include <QWidget>

class Console : public QWidget
{
	Q_OBJECT
public:
	Console( QWidget *parent = 0L );
	~Console();

public slots:
	void refresh();

private:
	struct Private;
	Private *d;

	void append( const QString & );

	int dump_disasm( unsigned int, unsigned int );

private slots:
	void execute();
};

#endif // __console_h__
