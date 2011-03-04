/*
 * Copyright (C) 2003-2011 Andreas Kling <kling@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ANDREAS KLING ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANDREAS KLING OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "vomit.h"
#include "debug.h"

#define ERROR 0x01
#define INDEX 0x02
#define CORR  0x04
#define DRQ   0x08
#define DSC   0x10
#define DWF   0x20
#define DRDY  0x40
#define BUSY  0x80

#define CONTROLLER (((port) & 0x1f0) == 0x170)

static WORD cylinder[2];
static BYTE head[2];
static BYTE sector[2];
static BYTE nsectors[2];
static BYTE error[2];

static BYTE ide_status(VCpu*, WORD);
static void ide_command(VCpu*, WORD, BYTE);
static BYTE get_sector_count(VCpu*, WORD);
static void set_sector_count(VCpu*, WORD, BYTE);
static BYTE get_sector(VCpu*, WORD);
static void set_sector(VCpu*, WORD, BYTE);
static BYTE get_cylinder_lsb(VCpu*, WORD);
static void set_cylinder_lsb(VCpu*, WORD, BYTE);
static BYTE get_cylinder_msb(VCpu*, WORD);
static void set_cylinder_msb(VCpu*, WORD, BYTE);
static BYTE get_head(VCpu*, WORD);
static void set_head(VCpu*, WORD, BYTE);
static BYTE ide_error(VCpu*, WORD port);

void ide_init()
{
    vm_listen(0x171, ide_error, 0L);
    vm_listen(0x172, get_sector_count, set_sector_count);
    vm_listen(0x173, get_sector, set_sector);
    vm_listen(0x174, get_cylinder_lsb, set_cylinder_lsb);
    vm_listen(0x175, get_cylinder_msb, set_cylinder_msb);
    vm_listen(0x176, get_head, set_head);
    vm_listen(0x177, ide_status, ide_command);
    vm_listen(0x1f1, ide_error, 0L);
    vm_listen(0x1f2, get_sector_count, set_sector_count);
    vm_listen(0x1f3, get_sector, set_sector);
    vm_listen(0x1f4, get_cylinder_lsb, set_cylinder_lsb);
    vm_listen(0x1f5, get_cylinder_msb, set_cylinder_msb);
    vm_listen(0x1f6, get_head, set_head);
    vm_listen(0x1f7, ide_status, ide_command);
}

static void ide_command(VCpu*, WORD port, BYTE data)
{
    vlog(VM_DISKLOG, "ide%d received cmd %02X", CONTROLLER, data);
}

static BYTE ide_status(VCpu*, WORD port)
{
    vlog(VM_DISKLOG, "ide%d status queried", CONTROLLER);
    return INDEX | DRDY;
}

static BYTE ide_error(VCpu*, WORD port)
{
    vlog(VM_DISKLOG, "ide%d error queried", CONTROLLER);
    return error[CONTROLLER];
}

static BYTE get_sector_count(VCpu*, WORD port)
{
    vlog(VM_DISKLOG, "ide%d sector count queried", CONTROLLER);
    return nsectors[CONTROLLER];
}

static void set_sector_count(VCpu*, WORD port, BYTE data)
{
    vlog(VM_DISKLOG, "ide%d sector count set to %d", CONTROLLER, data);
    nsectors[CONTROLLER] = data;
}

static BYTE get_sector(VCpu*, WORD port)
{
    vlog(VM_DISKLOG, "ide%d sector queried", CONTROLLER);
    return sector[CONTROLLER];
}

static void set_sector(VCpu*, WORD port, BYTE data)
{
    vlog(VM_DISKLOG, "ide%d sector set to %d", CONTROLLER, data);
    sector[CONTROLLER] = data;
}

static BYTE get_cylinder_lsb(VCpu*, WORD port)
{
    vlog(VM_DISKLOG, "ide%d cylinder LSB queried", CONTROLLER);
    return cylinder[CONTROLLER] & 0xFF;
}

static void set_cylinder_lsb(VCpu*, WORD port, BYTE data)
{
    vlog(VM_DISKLOG, "ide%d cylinder LSB set to 0x%02X", CONTROLLER, data);
    cylinder[CONTROLLER] &= 0xFF00;
    cylinder[CONTROLLER] |= data;
}

static BYTE get_cylinder_msb(VCpu*, WORD port)
{
    vlog(VM_DISKLOG, "ide%d cylinder MSB queried", CONTROLLER);
    return (cylinder[CONTROLLER] >> 8) & 0xFF;
}

static void set_cylinder_msb(VCpu*, WORD port, BYTE data)
{
    vlog(VM_DISKLOG, "ide%d cylinder MSB set to 0x%02X", CONTROLLER, data);
    cylinder[CONTROLLER] &= 0x00FF;
    cylinder[CONTROLLER] |= (data << 8);
}

static BYTE get_head(VCpu*, WORD port)
{
    vlog(VM_DISKLOG, "ide%d head queried", CONTROLLER);
    return head[CONTROLLER];
}

static void set_head(VCpu*, WORD port, BYTE data)
{
    vlog(VM_DISKLOG, "ide%d head set to %d", CONTROLLER, data);
    head[CONTROLLER] = data;
}
