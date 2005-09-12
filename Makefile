RM = rm -f
OBJS = vomit.o ui.o config.o dump.o debug.o vmcalls.o vga.o 186.o 8086/cpu.o 8086/memory.o 8086/misc.o 8086/bcd.o 8086/interrupt.o 8086/modrm.o 8086/stack.o 8086/io.o 8086/mov.o 8086/loop.o 8086/flags.o 8086/math.o 8086/string.o 8086/jump.o 8086/bitwise.o 8086/wrap.o bios/floppy.o
DEFS = -DVM_UNIX -DVM_CURSES -DVM_DISASM -DVM_TRAPFLAG -DVM_DEBUG -DVM_BREAK #-DVM_NOPFQ -DVM_EXPENDABLE
CFLAGS = -ggdb3 -pipe -O1 -W -Wall -pedantic #-Wpointer-arith -Wcast-qual -Wuninitialized -Wshadow -Wformat -Wimplicit -Wunused -Wundef -Wformat-security -ggdb3
INCLUDES = -Iinclude
LIBS = -lcurses
PRG = vmachine

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFS) -o $@ -c $<

all:	$(PRG)

$(PRG): $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) -o $@ $(OBJS)

clean:
	$(RM) $(PRG) $(OBJS)
