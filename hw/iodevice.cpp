#include "iodevice.h"
#include <QList>
#include <QDebug>

namespace Vomit
{

QList<IODevice *> IODevice::s_devices;
QMap<uint16_t, IODevice *> IODevice::s_readDevice;
QMap<uint16_t, IODevice *> IODevice::s_writeDevice;

IODevice::IODevice( const char *name )
	: m_name( name )
{
	s_devices.append( this );
}

IODevice::~IODevice()
{
	s_devices.removeAll( this );
}

void
IODevice::listen( uint16_t port, ListenMask mask )
{
	if( mask & Read )
		s_readDevice[port] = this;

	if( mask & Write )
		s_writeDevice[port] = this;

	m_ports.append( port );
}

QList<uint16_t>
IODevice::ports() const
{
	return m_ports;
}

const char *
IODevice::name() const
{
	return m_name;
}

void
IODevice::out8( uint8_t )
{
	qDebug() << "FIXME: IODevice::out8()";
}

uint8_t
IODevice::in8()
{
	qDebug() << "FIXME: IODevice::in8()";
	return 0;
}

}
