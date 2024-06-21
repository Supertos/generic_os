; -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
; *Supertos Industries
; Author: Supertos, 2024
; Generic OS Bootloader Stage 2

bits 32

_begin32:
	xor eax, eax
	xor ebx, ebx
	
	mov al, 0
	mov bl, 0x2f
	mov eax, 0xB8001
	mov byte [eax], bl
	_loop_a:
		add al, 1
		mov byte [0xB8000], al
		jmp _loop_a
	hlt