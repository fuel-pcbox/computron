%include "32bit.inc"

mov eax, 0x11223344
rol eax, 4

mov ebx, 0x55667788
rol ebx, 4

mov ecx, 0x99aabbcc
rol ecx, 4

mov edx, 0xddeeff00
rol edx, 4

mov ebp, 0x11223344
rol ebp, 12

mov esp, 0x55667788
rol esp, 12

mov esi, 0x99aabbcc
rol esi, 12

mov edi, 0xddeeff00
rol edi, 12

db 0xf1
