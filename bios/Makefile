all: bios.bin

bios.bin: bios.asm
	nasm -f bin -o $@ $<

clean:
	rm bios.bin
