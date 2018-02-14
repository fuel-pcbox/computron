[bits 16]

mov eax, 0x01234567
mov ebx, 0x89ABCDEF
shrd eax, ebx, 4

db 0xf1
