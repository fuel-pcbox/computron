/*
 * busmouse... based on... guesswork and bochs peeking
 */

#include "vomit.h"
#include "debug.h"

static BYTE busmouse_read(VCpu* cpu, WORD port);
static void busmouse_write(VCpu* cpu, WORD port, BYTE data);

static bool interrupts = true;
static BYTE busmouse_command = 0;
static BYTE busmouse_buttons = 0;

void busmouse_init()
{
    vm_listen(0x23c, busmouse_read, busmouse_write);
    vm_listen(0x23d, busmouse_read, 0L);
    vm_listen(0x23e, busmouse_read, busmouse_write);
    vm_listen(0x23f, 0L, 0L);
}

void
busmouse_write(VCpu*, WORD port, BYTE data)
{
    switch (port) {
    case 0x23e:
        busmouse_command = data;
        break;

    case 0x23f:
        switch (data) {
        case 0x10:
            vlog(VM_MOUSEMSG, "Bus mouse interrupt disabled");
            interrupts = false;
            break;
        case 0x11:
            vlog(VM_MOUSEMSG, "Bus mouse interrupt enabled");
            interrupts = true;
            break;
        default:
            vlog(VM_MOUSEMSG, "Write %02X to %03X, don't know what to do", data, port);
        }
        break;

    case 0x23c:
    case 0x23d:
    default:
        vlog(VM_MOUSEMSG, "Write %02X to %03X, don't know what to do", data, port);
        break;
    }
}

static int delta_x = 0;
static int delta_y = 0;

static int last_x = 0;
static int last_y = 0;

static int current_x = 0;
static int current_y = 0;

void busmouse_event()
{
    current_x = get_current_x();
    current_y = get_current_y();

    delta_x = current_x - last_x;
    delta_y = current_y - last_y;

    //vlog(VM_MOUSEMSG, "busmouse_event(): dX = %d, dY = %d", delta_x, delta_y);

    if (interrupts)
        irq(5);
}

void busmouse_press(int button)
{
    if (button == 1)
        busmouse_buttons &= ~(1 << 7);
    else
        busmouse_buttons &= ~(1 << 5);

    delta_x = 0;
    delta_y = 0;

    if (interrupts)
        irq(5);
}

void busmouse_release(int button)
{
    if (button == 1)
        busmouse_buttons |= (1 << 7);
    else
        busmouse_buttons |= (1 << 5);

    delta_x = 0;
    delta_y = 0;

    if (interrupts)
        irq(5);
}

BYTE busmouse_read(VCpu*, WORD port)
{
    static BYTE interrupt_val = 0x01;

    BYTE ret = 0;

    switch (port) {
    case 0x23c:
        switch (busmouse_command) {
        case 0x90:
        case 0x80: // X LSB
            ret = delta_x & 0xF;
            break;
        case 0xb0:
        case 0xa0: // X MSB
            ret = (delta_x >> 4) & 0xF;
            last_x = current_x;
            break;
        case 0xd0:
        case 0xc0: // Y LSB
            ret = delta_y & 0xF;
            break;
        case 0xf0:
        case 0xe0: // Y MSB
            ret = (delta_y >> 4) & 0xF;
            ret |= busmouse_buttons;
            last_y = current_y;
            break;
        default:
            vlog(VM_MOUSEMSG, "Unknown BusMouse command %02X", busmouse_command);
        }
        break;

    case 0x23d:
        // Signature byte
        // I believe some OS's expect this to alternate between 0xDE and 0xA5...
        ret = 0xa5;
        break;

    case 0x23e:
        // Stolen from NeXTStep-on-QEMU patches
        ret = interrupt_val;
        interrupt_val = (interrupt_val << 1) & 0xff;
        if (interrupt_val == 0)
            interrupt_val = 1;
        break;

    case 0x23f:
    default:
        vlog(VM_MOUSEMSG, "Read from %03X, don't know what to do", port);
        break;
    }

    return ret;
}

