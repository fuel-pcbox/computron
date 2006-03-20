PROGRAM = disasm
OBJS = disasm.o modrm.o tables.o test.o
RM = rm -f

CPPFLAGS = -Iinclude
CFLAGS = -pipe -W -Wall -pedantic -ggdb -std=c99

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

clean:
	$(RM) $(PROGRAM) $(OBJS)
