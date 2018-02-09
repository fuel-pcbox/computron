/*
 * Copyright (C) 2003-2018 Andreas Kling <awesomekling@gmail.com>
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
#include "vcpu.h"
#include "debug.h"

void VCpu::_ESCAPE(Instruction& insn)
{
    vlog(LogFPU, "FPU escape via %02X %02X (or %02X /%u)",
        insn.op(), insn.rm(), insn.op(), insn.slash());

    return;
    // FIXME: Instruction should take care of skipping bytes here!
    VM_ASSERT(false);

    // Even though we don't dispatch to an FPU, we still have to decode the full
    // opcode, or the instruction stream will fall out of sync.
    //BYTE rm = fetchOpcodeByte();
    //(void) readModRM16(rm);

    // FIXME: If the code below is enabled, MSD.EXE hangs on startup trying to
    //        communicate with the FPU.
#if 0
    // 80286+: Coprocessor not available exception.
    exception(7);
#endif
}
