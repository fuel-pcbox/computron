#ifndef __iodevice_h__
#define __iodevice_h__

#include <stdint.h>
#include <QList>
#include <QHash>

namespace Vomit
{

class IODevice
{
public:
	IODevice( const char *name );
	virtual ~IODevice();

	const char *name() const;

	virtual uint8_t in8();
	virtual void out8( uint8_t );

	static QList<IODevice *> & devices();
	static QHash<uint16_t, IODevice *> & readDevices();
	static QHash<uint16_t, IODevice *> & writeDevices();

	QList<uint16_t> ports() const;

protected:
	enum ListenMask {
		Read = 1,
		Write = 2,
		ReadWrite = 3
	};
	virtual void listen( uint16_t port, ListenMask mask );

private:
	const char *m_name;
	QList<uint16_t> m_ports;
};

}

#endif
