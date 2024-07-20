// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// *Supertos Industries
// Author: Supertos, 2024
// Contains setup values
#pragma once

#define IDT_SIZE 128				// IDT Size in entries. Actual occupied memory for 32 bit is IDT_SIZE * 8.
#define IDT_SYSCALL_ID 64			// Interrupt ID to which all system calls are hooked.
#define IDT_ADDR 0x100000			// Addr where IDT shall be placed.

#define MEM_STACK_ADDR 0x100800		// Addr where Free Memory Stack shall be placed.
#define MEM_MAP_ADDR 0x500			// Where Bootloader places Memory Map. For Generic Bootloader it is 0x500.
#define MEM_KERNEL_RESERVE 0x400000	// How much memory does kernel have. It is reserved by Bootloader and is never freed.

#define MEM_PAGE_SIZE 4096			// NO PAE