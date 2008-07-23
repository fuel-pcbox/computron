RM = rm -f
OBJS = vomit.o ui.o config.o dump.o debug.o vmcalls.o fpu.o 186.o 8086/cpu.o 8086/memory.o 8086/misc.o 8086/bcd.o 8086/interrupt.o 8086/modrm.o 8086/stack.o 8086/io.o 8086/mov.o 8086/loop.o 8086/flags.o 8086/math.o 8086/string.o 8086/jump.o 8086/bitwise.o 8086/wrap.o bios/video.o bios/floppy.o hw/ide.o hw/vga.o hw/dma.o hw/floppy.o disasm/disasm.o disasm/tables.o disasm/modrm.o hw/pic.o hw/keyboard.o
DEFS = -DVM_DEBUG -DVM_NOPFQ #-DVOMIT_CORRECTNESS -DVOMIT_CURSES #-DVOMIT_BIG_ENDIAN
#DEFS = -DVM_DEBUG -DVOMIT_CORRECTNESS# -DVOMIT_CURSES #-DVOMIT_BIG_ENDIAN
#DEFS += -DVOMIT_SDL -DVOMIT_CURSES `sdl-config --cflags`
CFLAGS = -std=c99 -ggdb3 -pipe -O3 -W -Wall -pedantic -Wpointer-arith -Wcast-qual -Wuninitialized -Wshadow -Wformat -Wimplicit -Wunused -Wundef -Wformat-security
INCLUDES = -Iinclude -Idisasm/include #`sdl-config --cflags`
LIBS = -lcurses `sdl-config --libs`
PRG = vmachine
LIBRARY = libvomit.so

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFS) -o $@ -c $<

all:	$(PRG) $(LIBRARY)

$(PRG): $(LIBRARY) main.o
	$(CC) -lvomit -L. $(LDFLAGS) $(LIBS) -o $@ main.o

$(LIBRARY): $(OBJS)
	$(CXX) -o $@ -shared -Wl,-soname=$@ $(OBJS)

clean:
	$(RM) $(PRG) $(OBJS)
