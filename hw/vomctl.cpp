#include "vomctl.h"
#include <stdio.h>
#include "vomit.h"
#include "debug.h"
#include <string.h>

namespace Vomit
{

static VomCtl the;

VomCtl::VomCtl()
    : IODevice( "VomCtl" )
{
    m_registerIndex = 0;
    listen( 0xD6, Vomit::IODevice::ReadWrite );
}

VomCtl::~VomCtl()
{
}

uint8_t
VomCtl::in8()
{
    //vlog( VM_VOMCTL, "Read register %02X", m_registerIndex );

    switch( m_registerIndex )
    {
        case 0x00: /* Always 0 */
            return 0;

        case 0x01: /* Get CPU type */
            return VOMIT_CPU_LEVEL;

        case 0x02: /* RAM size LSB */
            return LSB(g_cpu->memory_size);

        case 0x03: /* RAM size MSB */
            return MSB(g_cpu->memory_size);
    }

    vlog( VM_VOMCTL, "Invalid register %02X read", m_registerIndex );
    return 0;
}

void
VomCtl::out8( uint8_t data )
{
    //vlog( VM_VOMCTL, "Select register %02X", data );
    m_registerIndex = data;
}

}
