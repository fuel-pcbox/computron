#ifndef __vga_h__
#define __vga_h__

void vga_init();
void vga_kill();

byte vga_read_register( byte index );
void vga_write_register( byte index, byte value );
void vga_scrollup(byte, byte, byte, byte, byte, byte);

#endif /* __vga_h__ */
