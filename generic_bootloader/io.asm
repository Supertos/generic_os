; -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
; *Supertos Industries
; Author: Supertos, 2024
; Generic OS BIOS interrupt wrapper
section .data
temp db 0
sector db 0
head db 0
cylinder db 0


; Sets VGA Mode to 0x03 and clears screen
_screenClear:
	pusha
	
	mov ah, 0x0
	mov al, 0x03
	int 0x10
	
	popa
	ret

	
; Reads one sector from lba
; AX - lba address
; ES:BX - Buffer
; DL - Drive ID
_readLBA:
	pusha
		push es
		push ax
		push bx
			mov ah, 0x8 ; Getting drive geometry
			int 0x13
			
			and cl, 0x3f ; Sectors Per Track
			add dh, 1 ; Number of Heads
		pop bx
		pop ax
		pop es
		
		div cl ; AL is now LBA / Sectors Per Track, AH is LBA % Sectors Per Track		
		add ah, 1
		
		mov byte [temp], al
		mov byte [sector], ah
		
		mov ah, 0x0 ; Clear upper half of AX. AX is now LBA / Sectors Per Track	
		div dh
		
		mov byte [head], ah
		
		mov byte [cylinder], al
		
		mov ax, 0x0201
		mov ch, byte [cylinder]
		mov byte [temp], ch
		and ch, 0xff
		mov cl, byte [sector]
		
		shr byte [temp], 2
		and byte [temp], 0xc0
		
		or cl, byte [temp]
		
		mov dh, byte [head]	
		mov dl, [driveID]
		int 0x13
	popa
	ret


; Waits for keyboard to respond
_waitForKeyboard:
	pusha
	
	_rep_1:
		in al, 0x64
		test al, 2
		jnz _rep_1
		
	popa
	ret
	