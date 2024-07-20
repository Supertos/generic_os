; -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
; *Supertos Industries
; Author: Supertos, 2024
; Version 3
; Generic OS Bootloader Stage 1


bits 16
org 0x7C00 ; BIOS loads bootloader here.

jmp _begin ; Enforce correct CS = 0x7C0

%include "memory.asm"
%include "paging.asm"

driveID db 0
GDT:
	_null dq 0
	_code:
		dw 0xffff 		; Limit
		dw 0x0 			; Base 1
		db 0x0			; Base 2
		db 0b10011010	; Access Byte
		db 0b11001111	; Flags + Limit
		db 0x0			; Base 3 
	_data:
		dw 0xffff 		; Base & Limit
		dw 0x0 			; Base
		db 0x0			; Base 2
		db 0b10010010	; Flags
		db 0b11001111	; Flags + Limit
		db 0x0			; Base 

GDTDesc:
    dw $ - GDT - 1 ; size (16 bit)
    dd GDT ; address (32 bit)
	

; Entry point of bootloader
_begin:
	cli
	mov [driveID], dl 	; Save Boot Drive ID
	
	; Setup stack
	mov ax, 0x500 		; Begin of usable RAM
	mov ss, ax			
	mov sp, 0x76FF 		; ~30KiB
	
	xor ax, ax
	
	call _screenClear 	; Clear Screen
	
	call _enableA20 	; Enable 21th bit for memory addressing
	
	call _readMemMap	; Get Memory Map from BIOS
	call _readDisk		; Read drive
	
	call _enablePaging 	; Switch to 32-bit mode
	
	mov ax, 0b10000
	mov ds, ax
	mov ss, ax
	add sp, 0x500
	jmp 0x08:0x8000 ; Jump to the other part of bootloader
	
	
times 506-($-$$) db 0
dw 0xaa55

times 512 db 0

%include "32bit.asm"