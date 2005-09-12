#ifndef __FLOPPY_H__
#define __FLOPPY_H__

	#include "vomit.h"

	#define		MAX_DRV_TITLESIZE		24

	#define     FD_NO_ERROR             0x00
	#define		FD_BAD_COMMAND			0x01
	#define     FD_BAD_ADDRESS_MARK     0x02
	#define     FD_WRITE_PROTECT_ERROR  0x03
	#define     FD_SECTOR_NOT_FOUND     0x04
	#define     FD_FIXED_RESET_FAIL     0x05
	#define     FD_CHANGED_OR_REMOVED   0x06
	#define     FD_SEEK_FAIL            0x40
	#define     FD_TIMEOUT              0x80
	#define     FD_FIXED_NOT_READY      0xAA

	word	chs2lba(byte, word, byte, word);
	byte	floppy_read(byte, word, word, word, word, word, word);

	void	bios_readsectors();
	void	bios_writesectors();
	void	bios_verifysectors();

	extern byte drv_status[];
	extern char drv_imgfile[][MAX_FN_LENGTH];
	extern word drv_spt[];
	extern word drv_heads[];
	extern word drv_sectors[];
	extern word drv_sectsize[];
	extern byte drv_type[];

#endif

