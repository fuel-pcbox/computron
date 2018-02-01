[bits 16]

%macro shl_test 2

    mov %1, %2
    shl %1, 1
    shl %1, 2
    mov cl, 3
    shl %1, cl

%endmacro

shl_test al, 0x82
shl_test ah, 0x82
shl_test ax, 0x8234
shl_test eax, 0x82345678

shl_test bl, 0x82
shl_test bh, 0x82
shl_test bx, 0x8234
shl_test ebx, 0x82345678

shl_test cl, 0x82
shl_test ch, 0x82
shl_test cx, 0x8234
shl_test ecx, 0x82345678

shl_test dl, 0x82
shl_test dh, 0x82
shl_test dx, 0x8234
shl_test edx, 0x82345678

shl_test si, 0x5678
shl_test esi, 0x56789ABC

shl_test di, 0x5678
shl_test edi, 0x56789ABC

shl_test bp, 0x5678
shl_test ebp, 0x56789ABC

shl_test sp, 0x5678
shl_test esp, 0x56789ABC

db 0xf1
