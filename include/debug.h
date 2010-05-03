#ifndef __debug_h__
#define __debug_h__

#ifdef VOMIT_DEBUG
#include <QtCore/qdebug.h>
#define VM_ASSERT Q_ASSERT
#else
#define VM_ASSERT(x)
#endif

extern void vlog( int category, const char *format, ... );

#endif /* __debug_h__ */
