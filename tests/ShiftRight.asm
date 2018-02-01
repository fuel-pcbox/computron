[bits 16]

%macro shr_test 2

    mov %1, %2
    shr %1, 1
    shr %1, 2
    mov cl, 3
    shr %1, cl

%endmacro

shr_test al, 0x82
shr_test ah, 0x82
shr_test ax, 0x8234
shr_test eax, 0x82345678

shr_test bl, 0x82
shr_test bh, 0x82
shr_test bx, 0x8234
shr_test ebx, 0x82345678

shr_test cl, 0x82
shr_test ch, 0x82
shr_test cx, 0x8234
shr_test ecx, 0x82345678

shr_test dl, 0x82
shr_test dh, 0x82
shr_test dx, 0x8234
shr_test edx, 0x82345678

shr_test si, 0x5678
shr_test esi, 0x56789ABC

shr_test di, 0x5678
shr_test edi, 0x56789ABC

shr_test bp, 0x5678
shr_test ebp, 0x56789ABC

shr_test sp, 0x5678
shr_test esp, 0x56789ABC

db 0xf1
