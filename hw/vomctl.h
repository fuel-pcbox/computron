#ifndef __vomctl_h__
#define __vomctl_h__

#include "iodevice.h"

namespace Vomit
{

class VomCtl : public Vomit::IODevice
{
public:
	VomCtl();
	~VomCtl();

	const char *name() const;

	void out8( uint8_t );
	uint8_t in8();
};

}

#endif
