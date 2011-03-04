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

#ifndef __debug_h__
#define __debug_h__

#ifdef VOMIT_DEBUG
#include <QtCore/qdebug.h>
#include <assert.h>
#define VM_ASSERT assert
#else
#define VM_ASSERT(x)
#endif

#define VM_INITMSG      100
#define VM_KILLMSG      101
#define VM_ERRORMSG     102
#define VM_EXITMSG		103
#define VM_LOADMSG		104
#define VM_FPUMSG       105
#define VM_CPUMSG       106
#define VM_BREAKMSG		107
#define VM_LOGMSG		108
#define VM_IOMSG		109
#define VM_ALERT		110
#define VM_OTHER		111
#define VM_OUTPUT		112
#define VM_DISKLOG		113
#define VM_PRNLOG		114
#define VM_VIDEOMSG		115
#define VM_KEYMSG		116
#define VM_CONFIGMSG	117
#define VM_MEMORYMSG    118
#define VM_DMAMSG       119
#define VM_FDCMSG       120
#define VM_DUMPMSG      121
#define VM_MOUSEMSG     122
#define VM_DOSMSG       123
#define VM_PICMSG       124
#define VM_VOMCTL       125
#define VLOG_CMOS       126


extern void vlog( int category, const char *format, ... );

#endif /* __debug_h__ */
