/*
 * Copyright (C) 2003-2011 Andreas Kling <kling@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ANDREAS KLING ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANDREAS KLING OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <disasm.h>
#include <string.h>
#include <stdio.h>

int main (int argc, char *argv[]) {
    FILE *fp;
    int w = 0, i, j;
    char buf[256];
    size_t end;
    BYTE data[65536];
    BYTE *p = (BYTE *)data;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <file>\n", argv[0]);
        return 1;
    }

    memset(data, 0xFF, sizeof(data) / sizeof(char));

    fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        perror(argv[1]);
        return 1;
    }

    end = fread(data, sizeof(char), sizeof(data) / sizeof(char), fp);
    end /= sizeof(char);

    fclose(fp);

    while (w >= 0) {
        printf("%08X  ", (unsigned int)(p - data));
        for (i = 0; i < insn_width(p); i++)
            printf("%02X", p[i]);
        for (j = 0; j < (8-i); j++)
            printf("  ");
        if (disassemble(p, (p - data), buf, sizeof(buf))) {
            printf("  %s\n", buf);
            p += insn_width(p);
        } else {
            printf("\nUnknown instruction %02X at offset 0x%08x\n", p[0], (unsigned int)(p - data));
            return 0;
        }
        if ((unsigned int)(p - data) >= end)
            break;
    }

    return 0;
}
