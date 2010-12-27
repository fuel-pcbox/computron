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
