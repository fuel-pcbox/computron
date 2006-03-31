#ifndef __keyboard_h__
#define __keyboard_h__

#include <QWidget>
#include <QQueue>
#include <QMap>
#include <QKeyEvent>

extern "C" {
	#include "../include/vomit.h"
}

class Keyboard : public QWidget
{
	Q_OBJECT
public:
	static void init();
	static word nextKey();
	static word peekKey();

private:
	Keyboard();
	~Keyboard();

	static Keyboard *s_instance;
	void keyPressEvent( QKeyEvent *e );
	void keyReleaseEvent( QKeyEvent *e );

	QQueue<word> m_keyQueue;
};

#endif // __keyboard_h__
