#ifndef __VGA_H__
#define __VGA_H__

	void	vga_init();
	void	vga_kill();
	void	vga_dump();

	void	vga_selreg(word, byte);
	void	vga_setreg(word, byte);
	word	vga_getreg(byte);

	byte	vga_reg[0x1F];
	void	vga_scrollup(byte, byte, byte, byte, byte, byte);

#endif

