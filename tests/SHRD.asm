[bits 16]

mov eax, 0x01234567
mov ebx, 0x89ABCDEF
shrd eax, ebx, 4

mov esi, 0x01234567
mov edi, 0x89ABCDEF
mov cl, 4
shrd esi, edi, cl

db 0xf1
