;
; Copyright (C) 2003-2018 Andreas Kling <awesomekling@gmail.com>
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions
; are met:
; 1. Redistributions of source code must retain the above copyright
;    notice, this list of conditions and the following disclaimer.
; 2. Redistributions in binary form must reproduce the above copyright
;    notice, this list of conditions and the following disclaimer in the
;    documentation and/or other materials provided with the distribution.
;
; THIS SOFTWARE IS PROVIDED BY ANDREAS KLING ``AS IS'' AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
; PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANDREAS KLING OR
; CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
; EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
; PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
; OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
; OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;

; vim:set syntax=nasm et sts=4 sw=4:

%define VOMCTL_REGISTER 0xD6
%define VOMCTL_CONSOLE_WRITE 0xD7
%define LEGACY_VM_CALL 0xE6

%define CMOS_REGISTER 0x70
%define CMOS_DATA 0x71

%define CMOS_REG_RTC_SECOND 0x00
%define CMOS_REG_RTC_MINUTE 0x02
%define CMOS_REG_RTC_HOUR 0x04
%define CMOS_REG_RTC_DAY_OF_WEEK 0x06
%define CMOS_REG_RTC_DAY 0x07
%define CMOS_REG_RTC_MONTH 0x08
%define CMOS_REG_RTC_YEAR 0x09
%define CMOS_REG_STATUS_B 0x0B
%define CMOS_REG_RTC_CENTURY 0x32

%define VGA_PALETTE_REGISTER 0x3C0
%define VGA_DAC_READ_ADDRESS 0x3C7
%define VGA_DAC_WRITE_ADDRESS 0x3C8
%define VGA_DAC_READ_DATA 0x3C9
%define VGA_DAC_WRITE_DATA 0x3C9
%define VGA_STATUS_REGISTER 0x3DA

%define BDA_COM1_IOBASE 0x400
%define BDA_COM2_IOBASE 0x402
%define BDA_COM3_IOBASE 0x404
%define BDA_COM4_IOBASE 0x406
%define BDA_LPT1_IOBASE 0x408
%define BDA_LPT2_IOBASE 0x40A
%define BDA_LPT3_IOBASE 0x40C
%define BDA_BASE_MEMORY_SIZE 0x413
%define BDA_CURRENT_VIDEO_MODE 0x449
%define BDA_NUMBER_OF_SCREEN_COLUMNS 0x44A
%define BDA_CURSOR_LOCATION_ARRAY 0x450
%define BDA_CURSOR_ENDING_SCANLINE 0x460
%define BDA_CURSOR_STARTING_SCANLINE 0x461
%define BDA_CURRENT_VIDEO_PAGE 0x462
%define BDA_VGA_IOBASE 0x463
%define BDA_NUMBER_OF_SCREEN_ROWS 0x484
%define BDA_VIDEO_MODE_OPTIONS 0x487

%define COM1_IOBASE 0x3F8
%define COM2_IOBASE 0x2F8
%define COM3_IOBASE 0x3E8
%define COM4_IOBASE 0x2E8
%define LPT1_IOBASE 0x3BC
%define LPT2_IOBASE 0x378
%define LPT3_IOBASE 0x278
%define VGA_IOBASE 0x3D4

%macro stub 1
    push    ax
    push    bx

    mov     bl, %1
    out     0xE0, al

    pop     bx
    pop     ax
%endmacro

[org 0]
[bits 16]

    jmp     0xF000:_bios_post

reboot_on_any_key:
    mov     si, msg_press_any_key
    call    safe_putString
.loop:
    mov     ax, 0x1600
    out     LEGACY_VM_CALL, al
    or      ax, 0
    jz      .loop
    jmp     0xF000:0

_cpux_dividebyzero:                 ; Interrupt 0x00 - Divide by zero
    push    si
    mov     si, msg_divide_by_zero
    call    safe_putString
    pop     si
    jmp     reboot_on_any_key

_cpux_singlestep:                   ; Interrupt 0x01 - Single step
    iret                            ; IRET. Write your own handler.

; == NOT IMPLEMENTED ==
;_cpux_nmi:                         ; Interrupt 0x02 - NMI
;   push    ax                      ; Preserve AX
;   mov     ax, 1                   ; AX = 1
;   out     0xE8, al                ; Set cpu_state=CPU_ALIVE (exit halt)
;   pop     ax                      ; Restore AX

_cpux_breakpoint:                   ; Interrupt 0x03 - Breakpoint
    push    si
    mov     si, msg_breakpoint
    call    safe_putString
    pop     si
    jmp     reboot_on_any_key

_cpux_overflow:                     ; Interrupt 0x04 - Overflow
    push    si
    mov     si, msg_overflow
    call    safe_putString
    pop     si
    jmp     reboot_on_any_key

_cpux_invalidop:
    push    si
    mov     si, msg_invalid_opcode
    call    safe_putString
    pop     si
    jmp     reboot_on_any_key

; INT 08 - PIT interrupt
;
; Increments DWORD at BDA:6C, which should be 0 at midnight (a TODO.)

_cpu_timer:
    push    ds
    push    bx
    xor     bx, bx
    mov     ds, bx
    inc     word [0x46c]
    jnz     .skipHighWord
    inc     word [0x46e]
.skipHighWord:
    pop     bx
    pop     ds
    int     0x1C

    push    ax
    mov     al, 0x20
    out     0x20, al
    pop     ax
    iret

_cpu_default_softtimer:
    iret

_kbd_interrupt:
    ;int     0x1B
    iret

_bios_ctrl_break:
    iret

_unimplemented_isr:
    iret
    push    si
    mov     si, msg_unimplemented
    call    safe_putString
    pop     si
    iret

_bios_post:                         ; Power On Self-Test ;-)
    cli
    mov     ax, 0x9000              ; actual default
    mov     ss, ax                  ; POST stack location
    mov     sp, 0x00FF              ; (whee)
    
    push    cs
    pop     ds

    xor     ax, ax
    mov     es, ax

    mov     [es:0x500], byte 0xFF   ; PrintScreen error.

    call    vga_clear               ; Clear screen and move cursor to
    xor     ax, ax                  ; upper left corner.
    call    vga_store_cursor

    mov     si, msg_version
    call    safe_putString          ; Print BIOS Version string

    call    _bios_setup_ints        ; Install BIOS Interrupts
    
    call    _bios_init_data         ; Initialize BIOS data area (0040:0000)

    mov     si, msg_crlf
    call    safe_putString

    call    ide_init

    mov     si, msg_crlf
    call    safe_putString

    call    _bios_find_bootdrv      ; Find a boot drive 
    jc      .nobootdrv

    call    _bios_load_bootsector   ; Load boot sector from boot drive
    jc      .bootloadfail

    sti

    jmp     0x0000:0x7C00           ; JMP to software entry

.nobootdrv:    
    mov     si, msg_no_boot_drive
    call    safe_putString
    hlt

.bootloadfail:
    mov     si, msg_boot_failed
    call    safe_putString
    cli
    hlt

safe_putString:
    push    ds
    push    cs
    pop     ds
    push    ax
    push    bx

    mov     bl, 0x02
.nextchar:
    lodsb
    or      al, 0x00
    jz      .end
    pushf
    push    cs
    call    vga_ttyecho
    jmp     .nextchar
.end:
    pop     bx
    pop     ax
    pop     ds
    ret

ct_console_write:
    push    ds
    push    cs
    pop     ds
    push    ax
    push    bx

    mov     bl, 0x02
.nextchar:
    lodsb
    or      al, 0x00
    jz      .end
    out     VOMCTL_CONSOLE_WRITE, al
    jmp     .nextchar
.end:
    in      al, VOMCTL_CONSOLE_WRITE
    pop     bx
    pop     ax
    pop     ds
    ret

; print_integer
;
;    Prints a 16-bit unsigned integer in decimal.
;
; Parameters:
;
;    AX = Integer to print
;

print_integer:
    push    ax
    push    bx

    or      ax, 0               ; If AX == 0, no need to loop through the digits
    jnz     .nonZero

    mov     al, '0'
    pushf
    push    cs
    call    vga_ttyecho         ; vga_ttyecho returns via IRET

    pop     bx
    pop     ax
    ret

.nonZero:
    push    cx
    push    dx
    push    bp

    mov     bx, 0x0002          ; Green on black
    xor     bp, bp              ; BP keeps track of whether we've started output
    mov     cx, 10000           ; Start with 10K digit

.loop:
    xor     dx, dx
    div     cx

    or      bp, ax              ; OR result into BP
    jz      .nextDigit          ; If BP == 0, skip printing this digit
    
    add     al, '0'

    pushf
    push    cs
    call    vga_ttyecho         ; vga_ttyecho returns via IRET

.nextDigit:
    push    dx                  ; Store away remainder
    xor     dx, dx
    mov     ax, cx
    mov     cx, 10
    div     cx
    mov     cx, ax              ; CX /= 10
    pop     ax                  ; AX = remainder
    
    jcxz    .end
    jmp     .loop

.end:
    pop     bp
    pop     dx
    pop     cx
    pop     bx
    pop     ax
    ret

_bios_setup_ints:
    push    ds

    mov     dx, _unimplemented_isr
    xor     al, al
.loop:
    call    .install
    inc     al
    jnz     .loop

    mov     al, 0x00
    mov     dx, _cpux_dividebyzero
    call    .install

    mov     al, 0x01
    mov     dx, _cpux_singlestep
    call    .install

;   == NOT IMPLEMENTED ==
;   mov     al, 0x02
;   mov     dx, _cpux_nmi
;   call    .install

    mov     al, 0x03
    mov     dx, _cpux_breakpoint
    call    .install

    mov     al, 0x04
    mov     dx, _cpux_overflow
    call    .install

    mov     al, 0x06
    mov     dx, _cpux_invalidop
    call    .install

    mov     al, 0x08
    mov     dx, _cpu_timer
    call    .install

    mov     al, 0x09
    mov     dx, _kbd_interrupt
    call    .install

    mov     al, 0x10
    mov     dx, _bios_interrupt10
    call    .install

    mov     al, 0x11
    mov     dx, _bios_interrupt11
    call    .install

    mov     al, 0x12
    mov     dx, _bios_interrupt12
    call    .install

    mov     al, 0x13
    mov     dx, _bios_interrupt13
    call    .install

    mov     al, 0x14
    mov     dx, _bios_interrupt14
    call    .install

    mov     al, 0x15
    mov     dx, _bios_interrupt15
    call    .install

    mov     al, 0x16
    mov     dx, _bios_interrupt16
    call    .install

    mov     al, 0x17
    mov     dx, _bios_interrupt17
    call    .install

    mov     al, 0x19
    mov     dx, 0x0000
    call    .install

    mov     al, 0x1a
    mov     dx, _bios_interrupt1a
    call    .install

    mov     al, 0x1b
    mov     dx, _bios_ctrl_break
    call    .install

    mov     al, 0x1c
    mov     dx, _cpu_default_softtimer
    call    .install

    pop     ds

    ret

.install:
    push    ax
    mov     cl, 4
    mul     byte cl
    xor     bx, bx
    mov     ds, bx
    mov     bx, ax
    mov     ax, cs
    mov     word [bx], dx
    mov     word [bx+2], ax
    pop     ax
    ret

check_for_8086:
    mov     al, 0xff
    mov     cl, 0x80
    shr     al, cl
    test    al, al
    ret

check_for_80186:
    pushf
    pop    ax
    and    ah, 0x0f
    push   ax
    popf
    pushf
    pop    ax
    not    ah
    test   ah, 0xf0
    ret
    
    

; INITALIZE BIOS DATA AREA -----------------------------
;
;    This section sets various values in the BDA,
;    depending on the current configuration.
;    Most importantly, it probes the floppy drives.
;

_bios_init_data:
    push    ds
    xor     ax, ax
    mov     ds, ax

    mov     [BDA_COM1_IOBASE], word COM1_IOBASE
    mov     [BDA_COM2_IOBASE], word COM2_IOBASE
    mov     [BDA_COM3_IOBASE], word COM3_IOBASE
    mov     [BDA_COM4_IOBASE], word COM4_IOBASE
    mov     [BDA_LPT1_IOBASE], word LPT1_IOBASE
    mov     [BDA_LPT2_IOBASE], word LPT2_IOBASE
    mov     [BDA_LPT3_IOBASE], word LPT3_IOBASE
    mov     [BDA_VGA_IOBASE], word VGA_IOBASE

    call    check_for_8086
    je      .print8086

    call    check_for_80186
    je      .print80186

    mov     si, msg_unknown
.cCend:
    call    safe_putString
    mov     si, msg_cpu
    call    safe_putString
    jmp     .checkMem

.print8086:
    mov     si, msg_8086
    jmp     .cCend
.print80186:
    mov     si, msg_80186
    jmp     .cCend

.checkMem:
    mov     al, 0x16            ; CMOS[16] = Base memory size MSB
    out     CMOS_REGISTER, al
    in      al, CMOS_DATA
    xchg    al, ah

    mov     al, 0x15            ; CMOS[15] = Base memory size LSB
    out     CMOS_REGISTER, al
    in      al, CMOS_DATA

    mov     [BDA_BASE_MEMORY_SIZE], ax

    call    print_integer

    mov     si, msg_kb_memory
    call    safe_putString
        
    mov     byte [0x0496], 0x0E ; 0x0E = CTRL & ALT depressed;
                                ; 101/102 ext. kbd.
    mov     byte [0x0417], 0x00
    mov     byte [0x0418], 0x00
    mov     byte [0x0497], 0x00

    mov     byte [BDA_CURRENT_VIDEO_MODE], 0x03
    mov     word [BDA_NUMBER_OF_SCREEN_COLUMNS], 80
    mov     byte [BDA_NUMBER_OF_SCREEN_ROWS], 24
    mov     byte [BDA_CURSOR_ENDING_SCANLINE], 0x0E
    mov     byte [BDA_CURSOR_STARTING_SCANLINE], 0x0D

    ; Video displays (active: color VGA, inactive: none)
    mov     word [0x048a], 0x0008

; EQUIPMENT LIST ---------------------------------------
;                                 v DMA (1 if not)
;   mov     word [0x0410], 0000000100100000b
;                                  xx     x   floppies
    mov     cx, 0000000100100000b
    mov     cx, 0000000100100101b
    ; No DMA
    ; 80x25 color
; ------------------------------------------------------

    xor     bp, bp
    mov     dl, 0x00
    mov     ax, 0x1300
    out     LEGACY_VM_CALL, al
    or      ah, 0x00
    jz      .setDisk00
.check01:   
    mov     dl, 0x01
    mov     ax, 0x1300
    out     LEGACY_VM_CALL, al
    or      ah, 0x00
    jz      .setDisk01
.check80:                                       ; We'll skip this for now.
    or      bp, 0x0000
    jz      .noFloppies
    mov     word [0x410], cx
.end:
    pop     ds
    ret
    
.setDisk00:
    or      cx, 0x01
    mov     si, msg_floppy_a
    call    safe_putString
    inc     bp
    jmp     .check01
.setDisk01:
    or      cx, 0x40
    mov     si, msg_floppy_b
    call    safe_putString
    inc     bp
    jmp     .check80
.noFloppies:
    mov     si, msg_no_floppies
    call    safe_putString
    jmp     .end

_bios_find_bootdrv:
    mov     dl, 0x00
    mov     ax, 0x3333
    out     LEGACY_VM_CALL, al
    jnc     .end
    mov     dl, 0x01
    mov     ax, 0x3333
    out     LEGACY_VM_CALL, al
    jnc     .end
    mov     dl, 0x80
    mov     ax, 0x3333
    out     LEGACY_VM_CALL, al
    jnc     .end
    mov     dl, 0x81
    mov     ax, 0x3333
    out     LEGACY_VM_CALL, al
    jnc     .end
.error:
    ret
.end:
    ret

_bios_load_bootsector:
    mov     ax, 0x0201
    mov     cx, 0x0001
    xor     dh, dh
    xor     bx, bx
    mov     es, bx
    mov     bx, 0x7C00
    out     0xE2, al
    jc      .end
    cmp     word [es:0x7dfe], 0xaa55
    jne     .noboot
.end:
    ret
.noboot:
    mov     si, msg_not_bootable
    call    safe_putString
    jmp     .end

; Interrupt 10
; BIOS Video Interrupt

_bios_interrupt10:
    or      ah, 0x00
    jz      .setVideoMode
    cmp     ah, 0x01
    jz      .setCursorType
    cmp     ah, 0x02
    je      .setCursor
    cmp     ah, 0x03
    je      .getCursor
    cmp     ah, 0x05
    je      .selectActiveDisplayPage
    cmp     ah, 0x06
    je      .scrollWindowUp
    cmp     ah, 0x07
    je      .scrollWindowDown
    cmp     ah, 0x08
    je      .readChar
    cmp     ah, 0x09 ; Write Character and Attribute at Cursor Position
    je      .outCharStill
    cmp     ah, 0x0a ; Write Character Only at Current Cursor Position (FIXME)
    je      .outCharStill
    cmp     ah, 0x11
    je      .characterGeneratorRoutine
    cmp     ah, 0x10
    je      .setGetPaletteRegisters
    cmp     ah, 0x0e
    je      .outChar
    cmp     ah, 0x0f
    je      .getVideoState
    cmp     ah, 0x12
    je      .videoSubsystemConfiguration
    cmp     ah, 0x1a
    je      .video_display_combination

    stub    0x10
.characterGeneratorRoutine:
    jmp     vga_character_generator_routine
.setGetPaletteRegisters:
    jmp     vga_set_get_palette_registers
.video_display_combination:
    jmp     video_display_combination
.readChar:
    jmp     vga_readchr
.outChar:
    jmp     vga_ttyecho
.outCharStill:
    jmp     vga_putc
.setVideoMode:
    jmp     vga_set_video_mode
.setCursorType:
    jmp     vga_set_cursor_type
.setCursor:
    jmp     vga_set_cursor_position
.getCursor:
    jmp     vga_get_cursor_position_and_size
.getVideoState:
    jmp     vga_get_video_state
.videoSubsystemConfiguration:
    jmp     vga_video_subsystem_configuration
.scrollWindowUp:
    out     0xE7, al
    jmp     .end
.scrollWindowDown:
    out     0xE8, al
    jmp     .end
.selectActiveDisplayPage:
    jmp     vga_select_active_display_page
.end:
    iret

video_display_combination:
    push    ds
    xor     bx, bx
    mov     ds, bx
    cmp     al, 0x01

    je      .set

.get:
    mov     bx, [0x048a]
    jmp     .end

.set:
    mov     [0x048a], bx

.end:
    pop     ds

    ; XXX: HelpPC says AL contains 0x1a if valid function requested in AH.
    ;      Needs verification.
    mov     al, 0x1a

    iret

; Interrupt 10, 11
; Character Generator Routine
;
; Input:
;    AH = 11
;    AL = function
;
; Functions:
;    30 = Get current character generator information
;    XX = Unimplemented

vga_character_generator_routine:

    cmp     al, 0x30
    je      .getCurrentCharacterGeneratorInformation
    stub    0x10
    jmp     .end

.getCurrentCharacterGeneratorInformation:
    ; BH = Pointer desired (0=Int1F, 1=Int44, 6=8x16 chartable, X=Unimplemented)
    ;
    ; Output:
    ;    ES:BP = pointer to table
    ;    CX = bytes per character
    ;    DL = rows (less 1)

    cmp     bh, 0x00
    je      .getInt1FPointer
    cmp     bh, 0x01
    je      .getInt44Pointer
    cmp     bh, 0x06
    je      .get8x16CharacterTablePointer
    stub    0x10
    jmp     .end

.getInt1FPointer:
    mov     dl, 0x1f
    jmp     .returnInterruptPointer

.getInt44Pointer:
    mov     dl, 0x44
    jmp     .returnInterruptPointer

.returnInterruptPointer:
    push    bx

    xor     cx, cx
    mov     es, cx

    xor     bx, bx
    mov     bl, dl
    shl     bl, 1
    shl     bl, 1

    mov     bp, [es:bx]
    mov     cx, [es:bx + 2]
    mov     es, cx

    xor     cx, cx
    xor     dl, dl

    pop     bx
    jmp     .end

.get8x16CharacterTablePointer:
    mov     cx, 0xC400
    mov     es, cx
    xor     bp, bp
    mov     cx, 16
    mov     dl, 8

    jmp     .end

.end:
    iret


; Interrupt 10, 05
; Select Active Display Page
;
; Input:
;    AH = 05
;    AL = new page number

vga_select_active_display_page:

    pusha
    push    ds

    xor     bx, bx
    mov     ds, bx

    mov     [BDA_CURRENT_VIDEO_PAGE], al

    cmp     [BDA_CURRENT_VIDEO_MODE], byte 0x0d
    je      .updateStartAddressForMode0D

    mov     si, msg_page_changed
    call    ct_console_write
    jmp     .end

.updateStartAddressForMode0D:
    xor     ah, ah
    shl     ax, 13
    mov     bx, ax

    mov     dx, 0x3d4
    mov     al, 0x0d
    out     dx, al
    inc     dx
    mov     al, bl
    out     dx, al

    dec     dx
    mov     al, 0x0c
    out     dx, al
    inc     dx
    mov     al, bh
    out     dx, al

    jmp     .end

.end:
    pop     ds
    popa
    iret

; Interrupt 10, 10
; Set/Get Palette Registers (EGA/VGA)
;
; Input:
;    AH = 10
;    AL = function
;
; Functions:
;    00 = Set individual palette registers
;    02 = Set all palette registers and border
;    10 = Set DAC color register
;    XX = Unimplemented

vga_set_get_palette_registers:

    cmp     al, 0x00
    je      .setIndividualPaletteRegisters
    cmp     al, 0x02
    je      .setAllPaletteRegistersAndBorder
    cmp     al, 0x10
    je      .setDACColorRegister
    cmp     al, 0x17
    je      .readBlockOfDACColorRegisters
    stub    0x10
    jmp     .end

.setIndividualPaletteRegisters:
    ; BH = color value
    ; BL = palette register

    push    ax
    push    dx

    ; Reading from VGA_STATUS_REGISTER means that the next write
    ; to VGA_PALETTE_REGISTER contains an index.
    mov     dx, VGA_STATUS_REGISTER
    in      al, dx

    mov     dx, VGA_PALETTE_REGISTER
    mov     al, bl
    out     dx, al

    mov     al, bh
    out     dx, al

    pop     dx
    pop     ax
    jmp     .end

.setAllPaletteRegistersAndBorder:
    ; ES:DX = pointer to 17-byte table (0-15 palette registers, 16 border color register)

    push    ax
    push    bx
    push    dx

    mov     bx, dx

    ; Reading from VGA_STATUS_REGISTER means that the next write
    ; to VGA_PALETTE_REGISTER contains an index.
    mov     dx, VGA_STATUS_REGISTER
    in      al, dx

    mov     ah, 0
    mov     dx, VGA_PALETTE_REGISTER
.setAllPaletteRegistersAndBorderLoop:
    mov     al, ah
    out     dx, al

    mov     al, [es:bx]
    out     dx, al
    inc     bx
    inc     ah

    cmp     ah, 16
    jne     .setAllPaletteRegistersAndBorderLoop

    pop     dx
    pop     bx
    pop     dx

    jmp     .end

.setDACColorRegister:
    ; BX = color register
    ; CH = green
    ; CL = blue
    ; DH = red

    push    ax
    push    dx

    mov     dx, VGA_DAC_WRITE_ADDRESS
    mov     al, bl
    out     dx, al

    mov     dx, VGA_DAC_WRITE_DATA
    pop     ax
    push    ax
    mov     al, ah
    out     dx, al
    mov     al, ch
    out     dx, al
    mov     al, cl
    out     dx, al

    pop     dx
    pop     ax
    jmp     .end

.readBlockOfDACColorRegisters:
    ; BX = first color register to read
    ; CX = number of color registers to read
    ; ES:DX = pointer to buffer for color registers

    push    ax
    push    bx
    push    cx
    push    dx

    mov     dx, VGA_DAC_READ_ADDRESS
    mov     al, bl
    pop     dx
    push    dx
    mov     bx, dx
    mov     dx, VGA_DAC_READ_DATA
.readBlockOfDACColorRegistersLoop:
    in      al, dx
    mov     [es:bx], al
    inc     bx
    in      al, dx
    mov     [es:bx], al
    inc     bx
    in      al, dx
    mov     [es:bx], al
    inc     bx
    dec     cx
    jnz     .readBlockOfDACColorRegistersLoop

    pop     dx
    pop     cx
    pop     bx
    pop     ax

    jmp     .end

.end:
    iret

; Interrupt 10, 00
; Set Video Mode
;
; Input:
;    AH = 00
;    AL = Video mode
;
; Notes:
;    If AL bit 7=1, prevents EGA, MCGA & VGA from clearing display.

vga_set_video_mode:

    push    ds
    push    bx
    push    ax

    mov     ah, al
    and     al, 0x80
    jnz     .dontClear
    call    vga_clear

.dontClear:
    xor     bx, bx
    mov     ds, bx
    mov     [BDA_CURRENT_VIDEO_MODE], ah

    ; Update bit 7 of BDA_VIDEO_MODE_OPTIONS
    and     byte [BDA_VIDEO_MODE_OPTIONS], 0x7f
    or      [BDA_VIDEO_MODE_OPTIONS], al

    pop     ax
    pop     bx
    pop     ds
    iret

; Interrupt 10, 12
; Video Subsystem Configuration (EGA/VGA)
;
; Input:
;    AL = 12
;    BL = function
;
; Functions:
;    10 = Return video configuration information
;    XX = Unimplemented

vga_video_subsystem_configuration:

    cmp     bl, 0x10
    je      .returnVideoConfigurationInformation
    stub    0x10
    jmp     .end

.returnVideoConfigurationInformation:
    ; BH = color mode (0=color, 1=mono)
    ; BL = EGA memory size (0=64k, 1=128k, 2=192k, 3=256k)
    ; CH = feature bits
    ; CL = switch settings

    mov     bh, 0
    mov     bl, 3
    mov     ch, 0
    mov     cl, 0
    jmp     .end

.end:
    iret

; Interrupt 10, 0F
; Get Video State
;
; Input:
;    AH = 0F
;
; Output:
;    AH = number of screen columns
;    AL = mode currently set
;    BH = current display page

vga_get_video_state:

    push    ds
    push    dx
    xor     dx, dx
    mov     ds, dx

    mov     al, [BDA_CURRENT_VIDEO_MODE]
    mov     ah, [BDA_NUMBER_OF_SCREEN_COLUMNS]
    mov     bh, [BDA_CURRENT_VIDEO_PAGE]
    
    pop     dx
    pop     ds
    iret

vga_clear:
    push    es
    push    di
    push    ax
    push    cx
    mov     ax, 0xb800
    mov     es, ax
    xor     di, di
    mov     ax, 0x0720
    mov     cx, (80*25)
    rep     stosw
    pop     cx
    pop     ax
    pop     di
    pop     es
    ret

vga_putc:
    push    es
    push    dx
    push    di
    push    ax

    mov     dx, 0x3d4
    mov     al, 0x0e
    out     dx, al
    inc     dx
    in      al, dx
    xchg    al, ah
    dec     dx
    mov     al, 0x0f
    out     dx, al
    inc     dx
    in      al, dx
    
    shl     ax, 1
    mov     di, ax

    mov     ax, 0xb800
    mov     es, ax

    pop     ax
    push    ax

    mov     ah, bl

    stosw
    
    pop     ax
    pop     di
    pop     dx
    pop     es
    iret

; vga_ttyecho
;
;    Prints a character to screen and advances cursor.
;    Handles CR, NL, TAB and backspace.
;    Scrolls video viewport if cursor position overflows.
;
;    Called by POST routines and INT 10,0E
;
; Parameters:
;
;    AL = Character to write
;    BH = Page number ( not implemented )
;    BL = Foreground pixel color

vga_ttyecho:
    push    dx
    push    es
    push    di
    push    ax

    mov     ax, 0xb800              ; Point ES to video memory for STOS access.
    mov     es, ax

    call    vga_load_cursor
    
    shl     ax, 1
    mov     di, ax                  ; DI = offset*2
    
    pop     ax                      ; original AX, outchar in AL
    push    ax

    mov     ah, 0x07                ; Gray on black (BL is only used in graphics modes.)

    cmp     al, 0x0d
    je      .cr
    cmp     al, 0x0a
    je      .lf
    cmp     al, 0x08
    je      .bs
    cmp     al, 0x09
    je      .tab
.regular:
    stosw
    call    vga_load_cursor
    inc     ax                      ; advance
    call    .locate
    jmp     .end

.cr:
    mov     ax, di
    shr     ax, 1
    jz      .zero               ; XXX: Necessary? Why not straight to .end?
    xor     dx, dx
    mov     di, 80
    div     word di
    mov     di, 80
    mul     word di
.zero:
    call    .locate
    jmp     .end
.lf:
    mov     ax, di
    shr     ax, 1               ; AX = Normal offset
    add     ax, 80              ; One row down
    call    .locate
    jmp     .end
.bs:
    mov     ax, di
    shr     ax, 1
    dec     ax
    call    vga_store_cursor    ; so we walk around .locate (v_s_c RETs for us)
    jmp     .end                ; Backspace isn't going to overflow the cursor,
.tab:
    mov     al, 0x20
    stosw
    stosw
    stosw
    stosw
    mov     ax, di
    shr     ax, 1
    add     ax, 4
    call    .locate
    jmp     .end

.locate:                        ; Move cursor to AX
    cmp     ax, 2000
    jl      vga_store_cursor    ; If cursor doesn't overflow, no need to scroll

    push    ds
    push    si
    push    cx

    push    es
    pop     ds

    xor     di, di              ; Scroll the contents of videomemory one row up
    mov     si, 160
    mov     cx, (80*24)
    rep     movsw
    
    mov     ax, 0x0720
    mov     cx, 80
    mov     di, (4000-160)
    rep     stosw
    
    pop     cx
    pop     si
    pop     ds

    mov     ax, (80*24)         ; Go to last row, first column.
    jmp     vga_store_cursor    ; vga_store_cursor will RET for us.

.end:
    pop     ax
    pop     di
    pop     es
    pop     dx
    
    iret

; Interrupt 10, 01
; Set Cursor Type
;
; Input:
;    AH = 01
;    CH = cursor starting scan line (top) (low order 5 bits)
;    CL = cursor ending scan line (bottom) (low order 5 bits)

vga_set_cursor_type:

    push    ds
    push    ax
    push    cx
    push    dx

    and     cx, 0x1f1f                      ; Mask 5 LSB's of CH and CL

    mov     dx, 0x3d4                       ; Select register
    mov     al, 0x0a                        ; 6845 register 0A: Starting scanline
    out     dx, al

    inc     dx
    mov     al, ch
    out     dx, al

    dec     dx                              ; Select register
    mov     al, 0x0b                        ; 6845 register 0B: End scanline
    out     dx, al

    inc     dx
    mov     al, cl
    out     dx, al

    xor     ax, ax
    mov     ds, ax
    mov     [BDA_CURSOR_STARTING_SCANLINE], ch
    mov     [BDA_CURSOR_ENDING_SCANLINE], cl

    pop     dx
    pop     cx
    pop     ax
    pop     ds

    iret

; vga_load_cursor
;
;    Reads the current VGA cursor location from 6845.
;    Breaks DX.
;
;    6845 registers are read in reverse order and XCHG:ed to save
;    space and time.
;
; Returns:
;
;    AX = Cursor location
;

vga_load_cursor:
    mov     dx, 0x3d4           ; Select register
    mov     al, 0x0e            ; (Cursor MSB)
    out     dx, al
    inc     dx                  ; Read register
    in      al, dx
    xchg    al, ah
    dec     dx                  ; Select register
    mov     al, 0x0f            ; (Cursor LSB)
    out     dx, al
    inc     dx
    in      al, dx
    ret

; vga_store_cursor
;
;    Writes AL/AH to the VGA cursor location registers.
;    Breaks AX and DX.
;

vga_store_cursor:
    push    ax
    push    bx
    push    dx
    mov     bx, ax

    mov     dx, 0x3d4
    mov     al, 0x0e
    out     dx, al

    mov     dx, 0x3d5
    mov     al, bh
    out     dx, al

    mov     dx, 0x3d4
    mov     al, 0x0f
    out     dx, al

    mov     dx, 0x3d5
    mov     al, bl
    out     dx, al

    pop     dx
    pop     bx
    pop     ax
    ret

; Interrupt 10, 02
; Set Cursor Position
;
; Input:
;    AH = 02
;    BH = display page
;    DH = row
;    DL = column

vga_set_cursor_position:

    push    ds
    push    ax
    push    bx
    push    dx
    push    si

    xor     ax, ax
    mov     ds, ax

    mov     bl, bh
    xor     bh, bh
    mov     si, BDA_CURSOR_LOCATION_ARRAY
    mov     [bx + si], dl
    mov     [bx + si + 1], dh

    mov     al, dh
    mov     dh, 80
    mul     byte dh             ; AX = row offset
    xor     dh, dh
    add     ax, dx

    call    vga_store_cursor
    
    pop     si
    pop     dx
    pop     bx
    pop     ax
    pop     ds
    iret 

; Interrupt 10, 03
; Get Cursor Position And Size
;
; Input:
;    AH = 03
;    BH = display page
;
; Output:
;    CH = cursor starting scan line (top) (low order 5 bits)
;    CL = cursor ending scan line (bottom) (low order 5 bits)
;    DH = row
;    DL = column
    
vga_get_cursor_position_and_size:

    push    ds
    push    ax
    push    bx
    push    si

    xor     ax, ax
    mov     ds, ax

    mov     bl, bh
    xor     bh, bh
    mov     si, BDA_CURSOR_LOCATION_ARRAY
    mov     dl, [bx + si]
    mov     dh, [bx + si + 1]

    mov     ch, [BDA_CURSOR_STARTING_SCANLINE]
    mov     cl, [BDA_CURSOR_ENDING_SCANLINE]

    pop     si
    pop     bx
    pop     ax
    pop     ds
    
    iret

vga_readchr:
    push    ds
    push    dx
    push    si

    call    vga_load_cursor

    shl     ax, 1
    mov     si, ax
    
    mov     ax, 0xb800
    mov     ds, ax
    lodsw

    pop     si
    pop     dx
    pop     ds

    iret

_bios_interrupt11:
    push    ds
    xor     ax, ax
    mov     ds, ax
    mov     ax, [0x0410]
    pop     ds
    iret

_bios_interrupt12:
    push    ds
    xor     ax, ax
    mov     ds, ax
    mov     ax, [0x413]
    pop     ds
    iret

_bios_interrupt13:
    or      ah, 0x00
    jz      .resetDisk
    cmp     ah, 0x01
    je      .getDiskStatus
    cmp     ah, 0x02
    je      .readSectors
    cmp     ah, 0x03
    je      .writeSectors
    cmp     ah, 0x04
    je      .verifySectors
    cmp     ah, 0x05
    je      .formatTrack
    cmp     ah, 0x08
    je      .getDriveParams
    cmp     ah, 0x15
    je      .readDASDType
    cmp     ah, 0x18
    je      .setMediaType
    stub    0x13
    jmp     .end
.resetDisk:
    and     dl, 0x80
    jz      .reset_floppy_drive
    call    reset_ide_drive
    jmp     .end
.reset_floppy_drive:
    mov     ax, 0x1300
    out     LEGACY_VM_CALL, al
    jmp     .end
.getDiskStatus:
    push    ds
    xor     ax, ax
    mov     ds, ax
    mov     al, [0x0441]
    pop     ds
    jmp     .end
.readSectors:
    out     0xE2, al                ; VM call 0xE2 - int13h clone (al is dummy)
    jmp     .end
.verifySectors:
    out     0xE4, al
    jmp     .end
.writeSectors:
    out     0xE3, al
    jmp     .end
.getDriveParams:
    mov     byte [cs:temp], al
    mov     ax, 0x1308
    out     LEGACY_VM_CALL, al
    mov     al, byte [cs:temp]
    jmp     .end
.readDASDType:
    mov     byte [cs:temp], al
    mov     ax, 0x1315
    out     LEGACY_VM_CALL, al
    mov     al, byte [cs:temp]
    jmp     .end
.setMediaType:
    mov     byte [cs:temp], al
    mov     ax, 0x1318
    out     LEGACY_VM_CALL, al
    mov     al, byte [cs:temp]
    jmp     .end
.formatTrack:
    mov     byte [cs:temp], al
    mov     ax, 0x1305
    out     LEGACY_VM_CALL, al
    mov     al, byte [cs:temp]
    jmp     .end
.end:
    jmp     iret_with_carry

_bios_interrupt14:
    or      ah, 0x00
    jz      .fn0x00
    cmp     ah, 0x03
    je      .fn0x03
    stub    0x14
    jmp     .end
.fn0x00:
    mov     ax, 0x0000              ; That's right. I'm not about to emulate a fucking RS232 port.
    jmp     .end
.fn0x03:
    mov     ax, 0x0000              ; Status is 0. No way.
    jmp     .end
.end:
    iret

; INT 15 - System BIOS Services
;
_bios_interrupt15:
    cmp     ah, 0xc0
    je      .getSystemConfigurationParameters
    cmp     ah, 0x87
    je      .moveExtendedBlock
    cmp     ah, 0x88
    je      .queryExtendedMemorySize
    cmp     ah, 0x24
    je      .a20control

    stub    0x15                    ; Unsupported int 15,%ah
    stc                             ; Return failure.
    mov     ah, 0x86                ; 80 for PC, 86 for XT/AT. FIXME: Change this?
.end:
    jmp     iret_with_carry

.getSystemConfigurationParameters:
    call     bios_get_system_configuration_parameters
    jmp     .end

.moveExtendedBlock:
    call    bios_move_extended_block
    jmp     .end

.queryExtendedMemorySize:

    mov     al, 0x31            ; CMOS[31] = Extended memory size MSB
    out     CMOS_REGISTER, al
    in      al, CMOS_DATA
    xchg    al, ah

    mov     al, 0x30            ; CMOS[30] = Base memory size LSB
    out     CMOS_REGISTER, al
    in      al, CMOS_DATA

    clc
    jmp     .end

.a20control:
    jmp     bios_a20_control

bios_move_extended_block:
    push    bp
    mov     bp, sp

    pusha

    stub 0x15 ; FIXME: remove this when no longer needed for debugging

    mov     bx, cx

    cli
    mov     ax, 0x01
    call    a20_set
    mov     [bp-4], ax

    ; base15_00 = (ES << 4) + regs.u.r16.si;
    mov     ax, es
    mov     cl, 4
    shl     ax, cl
    add     ax, si
    mov     [bp-6], ax

    ; base23_16 = ES >> 12;
    mov     ax, es
    mov     al, ah
    xor     ah, ah
    mov     cl, 4
    shr     ax, cl
    mov     [bp-7], al

    ; if (base15_00 < (ES<<4))
    ;   base23_16++;
    mov     ax, es
    mov     cl, 4
    shl     ax, cl
    cmp     ax, [bp-6]
    jna     .skip
    mov     al, [bp-7]
    inc     ax
    mov     [bp-7], al
.skip:

    ; write_word(ES, regs.u.r16.si+0x08+0, 47);       // limit 15:00 = 6 * 8bytes/descriptor
    mov     [es:si+0x08+0], word 47

    ; write_word(ES, regs.u.r16.si+0x08+2, base15_00);// base 15:00
    mov     ax, [bp-6]
    mov     [es:si+0x08+2], ax
    
    ; write_byte(ES, regs.u.r16.si+0x08+4, base23_16);// base 23:16
    mov     ax, [bp-7]
    mov     [es:si+0x08+4], al

    ; write_byte(ES, regs.u.r16.si+0x08+5, 0x93);     // access
    ; write_word(ES, regs.u.r16.si+0x08+6, 0x0000);   // base 31:24/reserved/limit 19:16
    mov     [es:si+0x08+5], byte 0x93
    mov     [es:si+0x08+6], word 0x0000

    ; // Initialize CS descriptor
    ; write_word(ES, regs.u.r16.si+0x20+0, 0xffff);// limit 15:00 = normal 64K limit
    ; write_word(ES, regs.u.r16.si+0x20+2, 0x0000);// base 15:00
    ; write_byte(ES, regs.u.r16.si+0x20+4, 0x000f);// base 23:16
    ; write_byte(ES, regs.u.r16.si+0x20+5, 0x9b);  // access
    ; write_word(ES, regs.u.r16.si+0x20+6, 0x0000);// base 31:24/reserved/limit 19:16
    mov     [es:si+0x20+0], word 0xffff
    mov     [es:si+0x20+2], word 0x0000
    mov     [es:si+0x20+4], byte 0x0f
    mov     [es:si+0x20+5], byte 0x9b
    mov     [es:si+0x20+6], word 0x0000

    ; // Initialize SS descriptor
    ; ss = get_SS();
    mov     ax, ss
    ; base15_00 = ss << 4;
    mov     cl, 4
    shl     ax, cl
    mov     [bp-6], ax

    ; ss = get_SS();
    mov     ax, ss
    ; base23_16 = ss >> 12;
    mov     al, ah
    xor     ah, ah
    mov     cl, 4
    shr     ax, cl
    mov     [bp-7], al

    ; write_word(ES, regs.u.r16.si+0x28+0, 0xffff);   // limit 15:00 = normal 64K limit
    mov     [es:si+0x28+0], word 0xffff

    ; write_word(ES, regs.u.r16.si+0x08+2, base15_00);// base 15:00
    mov     ax, [bp-6]
    mov     [es:si+0x28+2], ax
    
    ; write_byte(ES, regs.u.r16.si+0x08+4, base23_16);// base 23:16
    mov     ax, [bp-7]
    mov     [es:si+0x28+4], al

    ; write_byte(ES, regs.u.r16.si+0x08+5, 0x93);     // access
    ; write_word(ES, regs.u.r16.si+0x08+6, 0x0000);   // base 31:24/reserved/limit 19:16
    mov     [es:si+0x28+5], byte 0x93
    mov     [es:si+0x28+6], word 0x0000

    ; BX contains moveblock word count
    mov     cx, bx
    
    push    eax
    xor     eax, eax
    mov     ds, ax
    mov     [0x469], ss
    mov     [0x467], sp

    lgdt    [es:si+0x08]
    lidt    [cs:pmode_idt_info]

    mov     eax, cr0
    or      al, 1
    mov     cr0, eax

    jmp     0x0020:.protty_mode
.protty_mode:
    mov     ax, 0x28
    mov     ss, ax
    mov     ax, 0x10
    mov     ds, ax
    mov     ax, 0x18
    mov     es, ax
    xor     si, si
    xor     di, di
    cld
    rep     movsw

    mov     ax, 0x28
    mov     ds, ax
    mov     es, ax

    mov     eax, cr0
    and     al, 0xfe
    mov     cr0, eax
    jmp     0xF000:.real_mode
.real_mode:
    lidt    [cs:rmode_idt_info]
    xor     ax, ax
    mov     ds, ax
    mov     ss, [0x469]
    mov     sp, [0x467]
    pop     eax

    mov     ax, [bp-4]
    call    a20_set

    sti

    popa

    pop     bp
    ret

pmode_idt_info:
dw 0x0000  ;; limit 15:00
dw 0x0000  ;; base  15:00
db 0x0f    ;; base  23:16

rmode_idt_info:
dw 0x03ff  ;; limit 15:00
dw 0x0000  ;; base  15:00
db 0x00    ;; base  23:16

a20_set:
    push    dx
    xchg    ah, al      ; AH = value to set

    mov     dx, 0x92
    in      al, dx
    shr     al, 1       ; AL = A20 enabled? (0/1)

    xchg    ah, al      ; AL = value to set

    shl     al, 1
    out     dx, al
    xchg    ah, al      ; AL = prev value

    pop     dx
    ret

bios_a20_control:
    cmp     al, 0x01
    je      .enableA20

    cmp     al, 0x02
    je      .disableA20

    cmp     al, 0x03
    je      .querySupport

    stub    0x15
    mov     ah, 0x86
    stc
    jmp     iret_with_carry

.enableA20:
    push    ax
    mov     ax, 1
    call    a20_set
    pop     ax
    xor     ah, ah
    jmp     .end

.disableA20:
    push    ax
    xor     ax, ax
    call    a20_set
    pop     ax
    xor     ah, ah
    jmp     .end

.querySupport:
    mov     bx, 0x0003              ; A20 gate controlled both via 8042 and System Control Port A (0x92)
    jmp     .end

.end:
    xor     ah, ah
    clc
    jmp     iret_with_carry

_bios_interrupt16:
    or      ah, 0x00
    jz      .waitKey
    cmp     ah, 0x01
    je      .kbhit
    cmp     ah, 0x10
    je      .waitKey
    cmp     ah, 0x11
    je      .kbhit
    cmp     ah, 0x02
    je      .getFlags
    cmp     ah, 0x22
    je      .getEFlags
    cmp     ah, 0x92
    je      .checkCapa10
    cmp     ah, 0xA2
    je      .checkCapa20
    stub    0x16
    jmp     .end
.getFlags:
    push    ds
    xor     ax, ax
    mov     ds, ax
    mov     al, [0x417]
    pop     ds
    jmp     .end
.getEFlags:
    push    ds
    xor     ax, ax
    mov     ds, ax
    mov     al, [0x417]
    mov     ah, [0x418]
    pop     ds
    jmp     .end
.checkCapa10:
    mov     ah, 0x80                ; Functions 0x10, 0x11 and 0x12 supported
.checkCapa20:
    jmp     .end                    ; Functions 0x20, 0x21 and 0x22 NOT supported
.waitKey:
    sti                             ; Allow clock fucking during this.
    mov     ax, 0x1600
    out     LEGACY_VM_CALL, al      ; Send 0x1600 to port 0xE6, VM handler of assorted crap.
    cmp     ax, 0
    jz      .waitKey
    jmp     .end
.kbhit:
    mov     ax, 0x1601
    out     LEGACY_VM_CALL, al
    push    bp
    mov     bp, sp                  ; This whole mekk can be optimized
    jz      .zero                   ; to make DOS run speedier.
    and     byte [bp+6], 0xbf       ; No ZF
    jmp     .ZFend
.zero:
    or      byte [bp+6], 0x40       ; ZF!
.ZFend:
    pop     bp
.end:
    iret

_bios_interrupt17:
    or      ah, 0x00
    jz      .printChar              ; echo $c > prn (!)
    cmp     ah, 0x01
    je      .fn0x01
    cmp     ah, 0x02
    je      .fn0x02
    stub    0x17
    jmp     .end
.printChar:
    push    cx
    mov     cl, al
    mov     ax, 0x1700
    out     LEGACY_VM_CALL, al
    pop     cx
    jmp     .end
.fn0x01:
    mov     ah, 0x00
    jmp     .end
.fn0x02:
    mov     ah, 0x00                ; Zeroes. No fucking printer here, dude.
.end:
    iret

_bios_interrupt1a:
    or      ah, 0x00
    je      .getticks
    cmp     ah, 0x02
    je      .getCurrentTime
    cmp     ah, 0x04
    je      .getCurrentDate
    cmp     ah, 0x05
    jle     .vmcall
    stub    0x1A
    jmp     .end
.getCurrentTime:
    push    .end
    jmp     rtc_get_current_time
.getCurrentDate:
    push    .end
    jmp     rtc_get_current_date
.vmcall:
    push    ax
    xchg    al, ah
    mov     ah, 0x1A
    out     LEGACY_VM_CALL, al
    pop     ax                      ; there used to be a 'jmp .end' here. phwa.
.end:
    jmp     iret_with_carry
.getticks:                          ; This thingie is placed separately for
    mov     ax, 0x1A00              ; optimization. DOS calls it all the time
    out     LEGACY_VM_CALL, al      ; and a speed boost can't hurt :-)
    iret

; Interrupt 1A, 02
; Read Time From RTC
;
; Input:
;    AH = 02
;
; Output:
;    CF = 0 if successful
;    CH = hour in BCD
;    CL = minute in BCD
;    DH = second in BCD
;    DL = 1 if DST option

rtc_get_current_time:
    push    ax

    mov     al, CMOS_REG_RTC_SECOND
    out     CMOS_REGISTER, al
    in      al, CMOS_DATA
    mov     dh, al

    mov     al, CMOS_REG_RTC_MINUTE
    out     CMOS_REGISTER, al
    in      al, CMOS_DATA
    mov     dh, al

    mov     al, CMOS_REG_RTC_HOUR
    out     CMOS_REGISTER, al
    in      al, CMOS_DATA
    mov     ch, al

    mov     al, CMOS_REG_STATUS_B
    out     CMOS_REGISTER, al
    in      al, CMOS_DATA
    mov     dl, al
    and     dl, 0x01

    clc
    pop     ax
    ret

; Interrupt 15, C0
; Get System Configuration Parameters
;
; Input:
;   AH = C0
;
; Output:
;   CF = 0 if successful
;   AH = only relevant if CF is set
;   ES:BX = pointer to system descriptor table in ROM

bios_get_system_configuration_parameters:

    clc

    mov     bx, cs
    mov     es, bx
    mov     bx, system_descriptor_table

    ret

; Interrupt 1A, 04
; Read Date From RTC
;
; Input:
;   AH = 04
;
; Output:
;   CF = 0 if successful
;   CH = century in BCD
;   CL = year in BCD
;   DH = month in BCD
;   DL = day in BCD

rtc_get_current_date:
    push    ax

    mov     al, CMOS_REG_RTC_CENTURY
    out     CMOS_REGISTER, al
    in      al, CMOS_DATA
    mov     ch, al

    mov     al, CMOS_REG_RTC_YEAR
    out     CMOS_REGISTER, al
    in      al, CMOS_DATA
    mov     cl, al

    mov     al, CMOS_REG_RTC_MONTH
    out     CMOS_REGISTER, al
    in      al, CMOS_DATA
    mov     dh, al

    mov     al, CMOS_REG_RTC_DAY
    out     CMOS_REGISTER, al
    in      al, CMOS_DATA
    mov     dl, al

    clc
    pop     ax
    ret


ide_init:
    push    ax
    push    bx
    push    dx
    push    si

    xor     bx, bx

    mov     si, msg_ide0
    call    safe_putString

    mov     dx, 0x1f7
    in      al, dx
    and     al, 0x40
    jnz     .drive0ready

    mov     si, msg_not
    call    safe_putString

    jmp    .drive0notready

.drive0ready:
    inc     bl

.drive0notready:
    mov     si, msg_ready
    call    safe_putString

    mov     si, msg_ide1
    call    safe_putString

    mov     dx, 0x177
    in      al, dx
    and     al, 0x40
    jnz     .drive1ready

    mov     si, msg_not
    call    safe_putString

    jmp     .drive1notready

.drive1ready:
    inc     bl

.drive1notready:
    mov     si, msg_ready
    call    safe_putString

; BDA:0075 - Number of hard disks attached
    push    ds
    xor     ax, ax
    mov     ds, ax
    mov     [0x475], bl
    pop     ds

    pop     si
    pop     dx
    pop     bx
    pop     ax
    ret

reset_ide_drive:
    push    dx
    mov     dx, 0x177           ; drive 1 base addr is 0x170
    cmp     dl, 0
    jz      .drive0
    add     dx, 0x80            ; drive 0 base addr is 0x1F0

.drive0:
    mov     al, 0x90            ; 90 - Execute Drive Diagnostic
    out     dx, al

    sub     dx, 6

    in      al, dx
    cmp     al, 0x00

    je      .ok
    stc
    mov     al, 0x05
    jmp     .end

.ok:
    xor     ah, ah
    mov     al, 0x00
    clc

.end:
    mov     ah, 0xAA
    pop     dx
    ret

iret_with_carry:
    push    bp
    mov     bp, sp
    jc      .carry
    and     byte [bp+6], 0xfe
    pop     bp
    iret
.carry:
    or      byte [bp+6], 0x01
    pop     bp
    iret

; DATA

    msg_version        db "Computron PC Emulator - https://github.com/awesomekling/computron", 0x0d, 0x0a
                       db "(C) 2003-2018 Andreas Kling <awesomekling@gmail.com>", 0x0d, 0x0a, 0x0d, 0x0a, 0

    msg_8086           db "8086", 0
    msg_80186          db "80186", 0
    msg_unknown        db "Unknown", 0
    msg_cpu            db " CPU", 0x0d, 0x0a, 0

    msg_kb_memory      db " kB memory", 0x0d, 0x0a, 0

    msg_floppy_a       db "Floppy A: present.", 0x0d, 0x0a, 0
    msg_floppy_b       db "Floppy B: present.", 0x0d, 0x0a, 0
    msg_no_floppies    db "No floppy drives present.", 0x0d, 0x0a, 0

    msg_divide_by_zero db "Divide by zero.", 0x0d, 0x0a, 0
    msg_overflow       db "Overflow.", 0x0d, 0x0a, 0
    msg_breakpoint     db "Breakpoint", 0x0d, 0x0a, 0
    msg_invalid_opcode db "Invalid opcode.", 0x0d, 0x0a, 0

    msg_unimplemented  db "Unimplemented ISR.", 0x0d, 0x0a, 0

    msg_no_boot_drive  db "No bootable media found.", 0x0d, 0x0a, 0
    msg_not_bootable   db "Warning: Boot signature missing.", 0x0d, 0x0a, 0
    msg_boot_failed    db "Could not load boot sector.", 0x0d, 0x0a, 0

    msg_press_any_key  db  "Hit any key to reboot.", 0x0d, 0x0a, 0

    msg_crlf           db  0x0d, 0x0a, 0

    msg_ide0           db  "IDE0", 0
    msg_ide1           db  "IDE1", 0
    msg_not            db  " not", 0
    msg_ready          db  " ready.", 0x0d, 0x0a, 0

    msg_page_changed   db "Display page changed (not fully supported)", 0

system_descriptor_table:
    dw 8               ; Length of table
    db 0xFC            ; IBM AT (same as F000:FFFE)
    db 0x01            ;        (same as F000:FFFF)
    db 0x01            ; BIOS revision level
    db 01100000b       ; Feature information
    dw 0

    ; this pretty much violates the whole ROM concept
    temp               dw  0x0000

times 0xfff0-($-$$) nop ; pad up to fff0

jmp 0xf000:0x0000 ; fff0: We start here after CPU reset

bios_date db "11/02/03" ; fff5: BIOS release date
db 0xfc ; IBM AT
db 0x01
