all: 
	cd generic_bootloader; nasm main.asm -o ../bootloader.bin
	truncate -s 1024 bootloader.bin
	
	cd generic_os; i686-elf-gcc -c kernel.c -std=gnu99 -ffreestanding -Wall -Wextra -o kernel.o
	cd generic_os; i686-elf-gcc -T link.ld -ffreestanding -O2 -nostdlib -lgcc
	
	cat bootloader.bin kernel.bin > image.img
	
	truncate -s 1474560 image.img
	rm -f bootloader.bin
	rm -f kernel.bin
	
clean:
	rm -f image.img