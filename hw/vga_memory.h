#ifndef __vga_memory_h__
#define __vga_memory_h__

#include "vomit.h"
#include <QtCore/QRect>

class QImage;

class VgaMemory
{
public:
    VgaMemory(VCpu *);
    ~VgaMemory();

    void write8(DWORD address, BYTE value);
    void write16(DWORD address, WORD value);
    BYTE read8(DWORD address);
    WORD read16(DWORD address);

    /*!
        Ask the VgaMemory object to synchronize its internal
        palette against the 6845 palette.
     */
    void syncPalette();

    /*!
        Returns the specified pixel plane.
	Valid indices are 0 through 3.
     */
    BYTE *plane(int index) const;

    QImage *modeImage(BYTE mode) const;

    bool isDirty() const;

    QRect dirtyRect() const;

    void clearDirty();

    QT_DEPRECATED void setDirty();

private:
    struct Private;
    Private *d;
};

#endif
