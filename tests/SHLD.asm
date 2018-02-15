[bits 16]

mov eax, 0x01234567
mov ebx, 0x89ABCDEF
shld eax, ebx, 4

mov esi, 0x01234567
mov edi, 0x89ABCDEF
mov cl, 4
shld esi, edi, cl

db 0xf1
