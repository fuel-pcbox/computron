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

void _ESCAPE(VCpu* cpu)
{
    vlog(VM_CPUMSG, "%04X:%08X FPU escape via %02X /%u",
        cpu->getBaseCS(), cpu->getBaseEIP(),
        cpu->opcode, vomit_modRMRegisterPart(cpu->readMemory8(cpu->getBaseCS(), cpu->getBaseIP() + 1)));

    //vm_exit(0);

    BYTE rm = cpu->fetchOpcodeByte();
    (void) cpu->readModRM16(rm);

    return;

#if 0
    printf("Swallowed %d bytes: ", cpu->IP - cpu->base_IP);
    for (int i = 0; i < cpu.IP - cpu->base_IP; ++i)
        printf("%02X ", vomit_cpu_memory_read8(cpu, cpu->getBaseCS(), cpu->base_IP + i));
    printf("\n");
#endif

#if 0
    /* 80286+: Coprocessor not available exception. */
    cpu->exception(7);
#endif
}
