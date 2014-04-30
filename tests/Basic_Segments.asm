[bits 16]

mov ax, 0x9abc
mov es, ax
mov ax, 0xdef0
mov ss, ax

mov ax, 0x1234
mov ds, ax

; Plant suicide instruction at 1234:0000
mov [0], byte 0xf1

mov ax, 0x5678
mov ds, ax

jmp 0x1234:0000
