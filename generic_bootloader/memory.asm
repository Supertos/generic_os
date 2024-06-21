; -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
; *Supertos Industries
; Author: Supertos, 2024
; Version 3
; Generic OS Bootloader Stage 1

%include "io.asm"

_enableA20:
	pusha
	
	mov ax, 0
	mov ds, ax
	mov si, 0x7DFE
	
	mov ax, 0xFFFF
	mov es, ax
	mov di, 0x7E0E
	
	mov ah, byte [DS:SI]
	cmp byte [ES:DI], ah
	jne _a20Enabled
	
	mov byte [ES:DI], 0x0
	mov ah, byte [DS:SI]
	cmp byte [ES:DI], ah
	jne _a20Enabled
	
	; Enable A20 Line ( 0x64 - PS/2 command port, 0x60 - PS/2 data port )
	call _waitForKeyboard
	mov al, 0xAD			; Disable Keyboard
	out 0x64, al
	
	call _waitForKeyboard
	mov al, 0xD0			; Read output port
	out 0x64, al
	
	call _waitForKeyboard
	mov al, 0x60			; Read configuration byte
	out 0x64, al
	call _waitForKeyboard
	in al, 0x60				; Move byte to al
	push eax
	
	call _waitForKeyboard
	mov al, 0xD1			; Write to output port	
	out 0x64, al
	
	call _waitForKeyboard
	pop eax
	or al, 2				; Enable A20 Gate
	out 0x60, al
	
	call _waitForKeyboard
	mov al, 0xAE			; Enable Keyboard
	out 0x64, al
	
	call _waitForKeyboard
	
	_a20Enabled:
	
	popa
	ret


_readMemMap:
	pusha
	
	mov ax, 0x50
	mov es, ax
		
	mov di, 0
	
	xor ebx, ebx
	xor eax, eax
	mov edx, 0x534D4150
	mov eax, 0xE820
	mov ecx, 24
	
	_memoryMapLoad: ; Dump memory map at 0x500
		int 0x15
		
		add di, 24
		mov eax, 0xE820
		mov ecx, 24
		
		cmp ebx, 0
	jne _memoryMapLoad
	
	popa
	ret
	

_readDisk:
	pusha
	
		mov ax, 0x0800
		mov es, ax
		mov bx, 0
		mov ax, 3
		_romLoad:
			call _readLBA
			
			add bx, 512 ; Jump by another sector
			inc ax
			cmp ax, 32 ; Load 16 KiB of memory
		jne _romLoad
	
	popa
	ret