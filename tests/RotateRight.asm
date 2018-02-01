[bits 16]

%macro ror_test 3

    mov %1, %2
    ror %1, 1
    ror %1, 2
    mov cl, 3
    ror %1, cl
    ror %1, 2
    ror %1, %3 - 8

%endmacro

ror_test al, 0x12, 8
ror_test ah, 0x12, 8
ror_test ax, 0x1234, 16
ror_test eax, 0x12345678, 32

ror_test bl, 0x12, 8
ror_test bh, 0x12, 8
ror_test bx, 0x1234, 16
ror_test ebx, 0x12345678, 32

ror_test cl, 0x12, 8
ror_test ch, 0x12, 8
ror_test cx, 0x1234, 16
ror_test ecx, 0x12345678, 32

ror_test dl, 0x12, 8
ror_test dh, 0x12, 8
ror_test dx, 0x1234, 16
ror_test edx, 0x12345678, 32

ror_test si, 0x5678, 16
ror_test esi, 0x56789ABC, 32

ror_test di, 0x5678, 16
ror_test edi, 0x56789ABC, 32

ror_test bp, 0x5678, 16
ror_test ebp, 0x56789ABC, 32

ror_test sp, 0x5678, 16
ror_test esp, 0x56789ABC, 32

db 0xf1
