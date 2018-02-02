[bits 16]

%macro rcr_test 3

    mov %1, %2
    rcr %1, 1
    rcr %1, 2
    mov cl, 3
    rcr %1, cl
    rcr %1, 2

%endmacro

rcr_test al, 0x12, 8
rcr_test ah, 0x12, 8
rcr_test ax, 0x1234, 16
rcr_test eax, 0x12345678, 32

rcr_test bl, 0x12, 8
rcr_test bh, 0x12, 8
rcr_test bx, 0x1234, 16
rcr_test ebx, 0x12345678, 32

rcr_test cl, 0x12, 8
rcr_test ch, 0x12, 8
rcr_test cx, 0x1234, 16
rcr_test ecx, 0x12345678, 32

rcr_test dl, 0x12, 8
rcr_test dh, 0x12, 8
rcr_test dx, 0x1234, 16
rcr_test edx, 0x12345678, 32

rcr_test si, 0x5678, 16
rcr_test esi, 0x56789ABC, 32

rcr_test di, 0x5678, 16
rcr_test edi, 0x56789ABC, 32

rcr_test bp, 0x5678, 16
rcr_test ebp, 0x56789ABC, 32

rcr_test sp, 0x5678, 16
rcr_test esp, 0x56789ABC, 32

db 0xf1
