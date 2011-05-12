TEMPLATE = app
TARGET = 
DEPENDPATH += . x86 bios disasm gui hw include disasm/include
INCLUDEPATH += . include disasm/include gui hw
CXXFLAGS += -std=gnu++0x -O0 -W -Wall -Wshadow
DEFINES += VOMIT_DEBUG
DEFINES += VOMIT_TRACE
CONFIG += silent
CONFIG += debug

CONFIG -= app_bundle

unix {
    LIBS += -lreadline
    DEFINES += HAVE_READLINE
    DEFINES += HAVE_USLEEP
}

OBJECTS_DIR = .obj
RCC_DIR = .rcc
MOC_DIR = .moc
UI_DIR = .ui

RESOURCES = vomit.qrc

HEADERS += gui/machinewidget.h \
           gui/mainwindow.h \
           gui/palettewidget.h \
           gui/screen.h \
           gui/worker.h \
           hw/fdc.h \
           hw/ide.h \
           hw/iodevice.h \
           hw/keyboard.h \
           hw/vomctl.h \
           hw/vga_memory.h \
           hw/cmos.h \
           hw/pic.h \
           hw/vga.h \
           include/debugger.h \
           include/types.h \
           include/vcpu.h \
           include/debug.h \
           include/floppy.h \
           include/machine.h \
           include/settings.h \
           include/templates.h \
           include/vomit.h \
           disasm/include/disasm.h \
           disasm/include/insn-types.h

SOURCES += debug.cpp \
           debugger.cpp \
           dump.cpp \
           fpu.cpp \
           machine.cpp \
           settings.cpp \
           vmcalls.cpp \
           x86/186.cpp \
           x86/bcd.cpp \
           x86/bitwise.cpp \
           x86/cpu.cpp \
           x86/flags.cpp \
           x86/interrupt.cpp \
           x86/io.cpp \
           x86/jump.cpp \
           x86/loop.cpp \
           x86/math.cpp \
           x86/modrm.cpp \
           x86/mov.cpp \
           x86/pmode.cpp \
           x86/stack.cpp \
           x86/string.cpp \
           x86/wrap.cpp \
           bios/floppy.cpp \
           disasm/disasm.cpp \
           disasm/disasm_modrm.cpp \
           disasm/tables.cpp \
           gui/machinewidget.cpp \
           gui/main.cpp \
           gui/mainwindow.cpp \
           gui/palettewidget.cpp \
           gui/screen.cpp \
           gui/worker.cpp \
           hw/busmouse.cpp \
           hw/fdc.cpp \
           hw/ide.cpp \
           hw/keyboard.cpp \
           hw/pic.cpp \
           hw/pit.cpp \
           hw/vga.cpp \
           hw/vomctl.cpp \
           hw/iodevice.cpp \
           hw/vga_memory.cpp \
           hw/cmos.cpp
