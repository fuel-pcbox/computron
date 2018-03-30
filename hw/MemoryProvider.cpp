#include "MemoryProvider.h"

BYTE* MemoryProvider::memoryPointer(DWORD)
{
    return nullptr;
}

void MemoryProvider::write8(DWORD, BYTE)
{
}

void MemoryProvider::write16(DWORD address, WORD data)
{
    write8(address, getLSB(data));
    write8(address + 1, getMSB(data));
}

void MemoryProvider::write32(DWORD address, DWORD data)
{
    write16(address, getLSW(data));
    write16(address + 2, getMSW(data));
}

BYTE MemoryProvider::read8(DWORD)
{
}

WORD MemoryProvider::read16(DWORD address)
{
    return makeWORD(read8(address + 1), read8(address));
}

DWORD MemoryProvider::read32(DWORD address)
{
    return makeDWORD(read16(address + 2), read16(address));
}
