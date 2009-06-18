#include "vomctl.h"
#include <stdio.h>

namespace Vomit
{

VomCtl::VomCtl()
{
	listen( 0xD6, Vomit::IODevice::ReadWrite );
}

VomCtl::~VomCtl()
{
}

const char *
VomCtl::name() const
{
	return "VomCtl";
}

uint8_t
VomCtl::in8()
{
	return 0xAA;
}

void
VomCtl::out8( uint8_t data )
{
	printf( "VomCtl: %02X\n", data );
}

}
