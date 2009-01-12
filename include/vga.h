#ifndef __vga_h__
#define __vga_h__

void vga_init();
void vga_kill();

byte vga_read_register( byte number );
void vga_write_register( byte number, byte value );
void vga_scrollup(byte, byte, byte, byte, byte, byte);

typedef struct {
	byte r;
	byte g;
	byte b;
} rgb_t;

extern rgb_t vga_color_register[];
extern byte vga_palette_register[];

#endif /* __vga_h__ */
