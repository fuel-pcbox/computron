#include "vomit.h"
#include "debug.h"

#define ERROR 0x01
#define INDEX 0x02
#define CORR  0x04
#define DRQ   0x08
#define DSC   0x10
#define DWF   0x20
#define DRDY  0x40
#define BUSY  0x80

#define CONTROLLER (((port) & 0x1f0) == 0x170)

static word cylinder[2];
static byte head[2];
static byte sector[2];
static byte nsectors[2];
static byte error[2];

static byte ide_status( word );
static void ide_command( word, byte );
static byte get_sector_count( word );
static void set_sector_count( word, byte );
static byte get_sector( word );
static void set_sector( word, byte );
static byte get_cylinder_lsb( word );
static void set_cylinder_lsb( word, byte );
static byte get_cylinder_msb( word );
static void set_cylinder_msb( word, byte );
static byte get_head( word );
static void set_head( word, byte );
static byte ide_error( word port );

void
ide_init()
{
	vm_listen( 0x171, ide_error, 0L );
	vm_listen( 0x172, get_sector_count, set_sector_count );
	vm_listen( 0x173, get_sector, set_sector );
	vm_listen( 0x174, get_cylinder_lsb, set_cylinder_lsb );
	vm_listen( 0x175, get_cylinder_msb, set_cylinder_msb );
	vm_listen( 0x176, get_head, set_head );
	vm_listen( 0x177, ide_status, ide_command );
	vm_listen( 0x1f1, ide_error, 0L );
	vm_listen( 0x1f2, get_sector_count, set_sector_count );
	vm_listen( 0x1f3, get_sector, set_sector );
	vm_listen( 0x1f4, get_cylinder_lsb, set_cylinder_lsb );
	vm_listen( 0x1f5, get_cylinder_msb, set_cylinder_msb );
	vm_listen( 0x1f6, get_head, set_head );
	vm_listen( 0x1f7, ide_status, ide_command );
}

static void
ide_command( word port, byte data )
{
	vlog( VM_DISKLOG, "ide%d received cmd %02X", CONTROLLER, data );
}

static byte
ide_status( word port )
{
	vlog( VM_DISKLOG, "ide%d status queried", CONTROLLER );
	(void) port;
	return INDEX | DRDY;
}

static byte
ide_error( word port )
{
	vlog( VM_DISKLOG, "ide%d error queried", CONTROLLER );
	return error[CONTROLLER];
}


static byte
get_sector_count( word port )
{
	vlog( VM_DISKLOG, "ide%d sector count queried", CONTROLLER );
	return nsectors[CONTROLLER];
}

static void
set_sector_count( word port, byte data )
{
	vlog( VM_DISKLOG, "ide%d sector count set to %d", CONTROLLER, data );
	nsectors[CONTROLLER] = data;
}

static byte
get_sector( word port )
{
	vlog( VM_DISKLOG, "ide%d sector queried", CONTROLLER );
	return sector[CONTROLLER];
}

static void
set_sector( word port, byte data )
{
	vlog( VM_DISKLOG, "ide%d sector set to %d", CONTROLLER, data );
	sector[CONTROLLER] = data;
}

static byte
get_cylinder_lsb( word port )
{
	vlog( VM_DISKLOG, "ide%d cylinder LSB queried", CONTROLLER );
	return cylinder[CONTROLLER] & 0xFF;
}

static void
set_cylinder_lsb( word port, byte data )
{
	vlog( VM_DISKLOG, "ide%d cylinder LSB set to 0x%02X", CONTROLLER, data );
	cylinder[CONTROLLER] &= 0xFF00;
	cylinder[CONTROLLER] |= data;
}

static byte
get_cylinder_msb( word port )
{
	vlog( VM_DISKLOG, "ide%d cylinder MSB queried", CONTROLLER );
	return (cylinder[CONTROLLER] >> 8) & 0xFF;
}

static void
set_cylinder_msb( word port, byte data )
{
	vlog( VM_DISKLOG, "ide%d cylinder MSB set to 0x%02X", CONTROLLER, data );
	cylinder[CONTROLLER] &= 0x00FF;
	cylinder[CONTROLLER] |= (data << 8);
}

static byte
get_head( word port )
{
	vlog( VM_DISKLOG, "ide%d head queried", CONTROLLER );
	return head[CONTROLLER];
}

static void
set_head( word port, byte data )
{
	vlog( VM_DISKLOG, "ide%d head set to %d", CONTROLLER, data );
	head[CONTROLLER] = data;
}
