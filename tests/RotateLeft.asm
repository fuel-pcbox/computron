[bits 16]

%macro rol_test 3

    mov %1, %2
    rol %1, 1
    rol %1, 2
    mov cl, 3
    rol %1, cl
    rol %1, 2
    rol %1, %3 - 8

%endmacro

rol_test al, 0x12, 8
rol_test ah, 0x12, 8
rol_test ax, 0x1234, 16
rol_test eax, 0x12345678, 32

rol_test bl, 0x12, 8
rol_test bh, 0x12, 8
rol_test bx, 0x1234, 16
rol_test ebx, 0x12345678, 32

rol_test cl, 0x12, 8
rol_test ch, 0x12, 8
rol_test cx, 0x1234, 16
rol_test ecx, 0x12345678, 32

rol_test dl, 0x12, 8
rol_test dh, 0x12, 8
rol_test dx, 0x1234, 16
rol_test edx, 0x12345678, 32

rol_test si, 0x5678, 16
rol_test esi, 0x56789ABC, 32

rol_test di, 0x5678, 16
rol_test edi, 0x56789ABC, 32

rol_test bp, 0x5678, 16
rol_test ebp, 0x56789ABC, 32

rol_test sp, 0x5678, 16
rol_test esp, 0x56789ABC, 32

db 0xf1
