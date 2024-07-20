; -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
; *Supertos Industries
; Author: Supertos, 2024
; Version 3
; Generic OS Bootloader Stage 1

; This routine places Page Directory Table on top of memory map, allocates first 4MBs and places PT on the top of PDT.
_enablePaging:
	pusha
	cld
	xor ax, ax
	mov es, ax
	mov di, 0x2000
	xor eax, eax
	mov ecx, 2048
	rep stosd
	; Generate Paging Tables
	mov eax, 0x2000
	mov dword [eax], 0x3000 | 0x3; Allocate first 4MiBs
	
	mov ecx, 1024 	; 1024 Entries in PT
	mov ebx, 0		; Physical address
	or ebx, 0x3 ; Set writable and present
	mov eax, 0x3000 ; Write target
	
	_pagingSetup:
		mov dword [eax], ebx ; Record Identity Paging
		
		add ebx, 0x1000
		add eax, 4
		loop _pagingSetup
	
	; Setup protected Mode
	mov eax, 0x2000
	mov cr3, eax
	
	lgdt [GDTDesc]
	
	mov eax, cr0
	or eax, 0x80000001
	mov cr0, eax
	
	popa
	ret