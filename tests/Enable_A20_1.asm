[bits 16]

mov dx, 0x92
mov al, 2
out dx, al

call check_a20

db 0xf1

%include "check_a20.inc"
