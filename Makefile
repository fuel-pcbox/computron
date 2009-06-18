RM = rm -f
OBJS = vomit.o config.o dump.o debug.o vmcalls.o fpu.o 186.o 8086/cpu.o 8086/memory.o 8086/misc.o 8086/bcd.o 8086/interrupt.o 8086/modrm.o 8086/stack.o 8086/io.o 8086/mov.o 8086/loop.o 8086/flags.o 8086/math.o 8086/string.o 8086/jump.o 8086/bitwise.o 8086/wrap.o bios/video.o bios/floppy.o hw/ide.o hw/vga.o hw/dma.o hw/floppy.o disasm/disasm.o disasm/tables.o disasm/modrm.o hw/pic.o hw/keyboard.o hw/busmouse.o hw/pit.o hw/gameport.o
DEFS = -DVOMIT_FOREVER -DVM_NOPFQ #-DVOMIT_CORRECTNESS #-DVOMIT_BIG_ENDIAN
#DEFS = -DVM_DEBUG -DVOMIT_CORRECTNESS #-DVOMIT_BIG_ENDIAN
CFLAGS = -std=c99 -ggdb -pipe -O3 -funroll-loops -fprefetch-loop-arrays -fforce-addr -fomit-frame-pointer -W -Wall -pedantic -Wcast-qual -Wuninitialized -Wshadow -Wformat -Wimplicit -Wunused -Wundef -Wformat-security -Wpointer-arith -fPIC -Wwrite-strings
INCLUDES = -Iinclude -Idisasm/include
LIBS =
LIBRARY = libvomit.so
MACLIBRARY = libvomit.dylib

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFS) -o $@ -c $<

all:	$(LIBRARY)

$(LIBRARY): $(OBJS)
	$(CXX) -o $@ -shared -Wl,-soname=$@ $(OBJS)

maclib: $(MACLIBRARY)

$(MACLIBRARY): $(OBJS)
	$(CXX) -dynamiclib -Wl,-headerpad_max_install_names,-undefined,dynamic_lookup -install_name,$(MACLIBRARY) -o $(MACLIBRARY) $(OBJS)

clean:
	$(RM) $(OBJS)
