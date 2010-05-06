TEMPLATE = app
TARGET = 
DEPENDPATH += . 8086 bios disasm gui hw include disasm/include
INCLUDEPATH += . include disasm/include gui hw
CXXFLAGS += -std=gnu++0x -W -Wall -Wshadow
DEFINES += VOMIT_DEBUG
DEFINES += VOMIT_TRACE
DEFINES += VOMIT_DOS_ON_LINUX_IDLE_HACK
#DEFINES += VOMIT_PREFETCH_QUEUE
CONFIG += silent
CONFIG += debug

CONFIG -= app_bundle

RESOURCES = vomit.qrc

HEADERS += gui/codeview.h \
           gui/console.h \
           gui/cpuview.h \
           gui/hexspinbox.h \
           gui/mainwindow.h \
           gui/memview.h \
           gui/screen.h \
           gui/worker.h \
           hw/iodevice.h \
           hw/vomctl.h \
           hw/vga_memory.h \
           include/186.h \
           include/8086.h \
           include/debug.h \
           include/floppy.h \
           include/templates.h \
           include/vga.h \
           include/vomit.h \
           disasm/include/disasm.h \
           disasm/include/insn-types.h
SOURCES += 186.cpp \
           config.cpp \
           debug.cpp \
           dump.cpp \
           fpu.cpp \
           vmcalls.cpp \
           vomit.cpp \
           8086/bcd.cpp \
           8086/bitwise.cpp \
           8086/cpu.cpp \
           8086/flags.cpp \
           8086/interrupt.cpp \
           8086/io.cpp \
           8086/jump.cpp \
           8086/loop.cpp \
           8086/math.cpp \
           8086/memory.cpp \
           8086/misc.cpp \
           8086/modrm.cpp \
           8086/mov.cpp \
           8086/stack.cpp \
           8086/string.cpp \
           8086/wrap.cpp \
           bios/floppy.cpp \
           bios/video.cpp \
           disasm/disasm.cpp \
           disasm/disasm_modrm.cpp \
           disasm/tables.cpp \
           gui/codeview.cpp \
           gui/console.cpp \
           gui/cpuview.cpp \
           gui/hexspinbox.cpp \
           gui/main.cpp \
           gui/mainwindow.cpp \
           gui/memview.cpp \
           gui/screen.cpp \
           gui/worker.cpp \
           hw/busmouse.cpp \
           hw/dma.cpp \
           hw/fdc.cpp \
           hw/gameport.cpp \
           hw/ide.cpp \
           hw/keyboard.cpp \
           hw/pic.cpp \
           hw/pit.cpp \
           hw/vga.cpp \
           hw/vomctl.cpp \
           hw/iodevice.cpp \
           hw/vga_memory.cpp
