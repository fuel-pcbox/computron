#ifndef __iodevice_h__
#define __iodevice_h__

#include <stdint.h>

namespace Vomit
{

class IODevice
{
public:
	IODevice();
	virtual ~IODevice();

	virtual const char *name() const = 0;

	virtual uint8_t in8() = 0;
	virtual void out8( uint8_t ) = 0;

protected:
	enum ListenMask {
		Read = 1,
		Write = 2,
		ReadWrite = 3
	};
	virtual void listen( uint16_t port, ListenMask mask );
};

}

#endif
