[bits 16]

%macro rcl_test 3

    mov %1, %2
    rcl %1, 1
    rcl %1, 2
    mov cl, 3
    rcl %1, cl
    rcl %1, 2

%endmacro

rcl_test al, 0x12, 8
rcl_test ah, 0x12, 8
rcl_test ax, 0x1234, 16
rcl_test eax, 0x12345678, 32

rcl_test bl, 0x12, 8
rcl_test bh, 0x12, 8
rcl_test bx, 0x1234, 16
rcl_test ebx, 0x12345678, 32

rcl_test cl, 0x12, 8
rcl_test ch, 0x12, 8
rcl_test cx, 0x1234, 16
rcl_test ecx, 0x12345678, 32

rcl_test dl, 0x12, 8
rcl_test dh, 0x12, 8
rcl_test dx, 0x1234, 16
rcl_test edx, 0x12345678, 32

rcl_test si, 0x5678, 16
rcl_test esi, 0x56789ABC, 32

rcl_test di, 0x5678, 16
rcl_test edi, 0x56789ABC, 32

rcl_test bp, 0x5678, 16
rcl_test ebp, 0x56789ABC, 32

rcl_test sp, 0x5678, 16
rcl_test esp, 0x56789ABC, 32

db 0xf1
