/* vomit.c
 * Main initialization procedures
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "vomit.h"
#include "debug.h"
#include "iodevice.h"
#include <QDebug>
#ifdef VOMIT_DEBUG
#include <signal.h>
#endif

vomit_options_t options;

#ifdef VOMIT_DEBUG
static void sigint_handler(int)
{
    g_cpu->attachDebugger();
}
#endif

void vomit_init()
{
    vlog( VM_INITMSG, "Initializing video BIOS" );
    video_bios_init();

    for (dword i = 0; i <= 0xFFFF; ++i)
        vm_listen(i, 0L, 0L);

    for (BYTE i = 0xE0; i <= 0xEF; ++i)
        vm_listen(i, 0L, vm_call8);

    vlog( VM_INITMSG, "Registering I/O devices" );
    foreach (Vomit::IODevice *device, Vomit::IODevice::devices()) {
        vlog(VM_INITMSG, "%s at 0x%p", device->name(), device);
    }

    pic_init();
    dma_init();
    vga_init();
    fdc_init();
    ide_init();
    pit_init();
    busmouse_init();
    keyboard_init();
    gameport_init();

    vm_loadconf();

    g_cpu->registerDefaultOpcodeHandlers();

#ifdef VOMIT_DEBUG
    signal(SIGINT, sigint_handler);
#endif
}

void vm_kill()
{
    vlog(VM_KILLMSG, "Killing VM");
    vga_kill();
    g_cpu->kill();
}

void vm_exit(int exit_code)
{
    vm_kill();
    exit(exit_code);
}
