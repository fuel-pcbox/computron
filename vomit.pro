macx:QMAKE_MAC_SDK = macosx10.9
TEMPLATE = app
TARGET = computron
DEPENDPATH += . x86 bios gui hw include
INCLUDEPATH += . include gui hw x86
QMAKE_CXXFLAGS += -std=c++17 -g -O0 -W -Wall -Wimplicit-fallthrough
DEFINES += VOMIT_DEBUG
DEFINES += VOMIT_TRACE
//DEFINES += VOMIT_DETERMINISTIC
CONFIG += silent
CONFIG += debug
QT += widgets

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

FORMS += gui/statewidget.ui

HEADERS += gui/machinewidget.h \
           gui/statewidget.h \
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
           hw/pit.h \
           hw/vga.h \
           hw/PS2.h \
           include/debugger.h \
           include/types.h \
           include/debug.h \
           include/floppy.h \
           include/machine.h \
           include/settings.h \
           include/templates.h \
           include/vomit.h \
           include/OwnPtr.h \
           x86/CPU.h \
           x86/Descriptor.h \
           x86/Instruction.h

SOURCES += debug.cpp \
           debugger.cpp \
           dump.cpp \
           machine.cpp \
           settings.cpp \
           vmcalls.cpp \
           x86/186.cpp \
           x86/bcd.cpp \
           x86/bitwise.cpp \
           x86/CPU.cpp \
           x86/Descriptor.cpp \
           x86/flags.cpp \
           x86/fpu.cpp \
           x86/Instruction.cpp \
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
           gui/machinewidget.cpp \
           gui/main.cpp \
           gui/mainwindow.cpp \
           gui/palettewidget.cpp \
           gui/statewidget.cpp \
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
           hw/cmos.cpp \
           hw/PS2.cpp
