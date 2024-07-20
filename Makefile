all: 
	cd generic_bootloader; nasm main.asm -o ../bootloader.bin
	truncate -s 1024 bootloader.bin
	
	cd generic_os; i686-elf-gcc -c *.c -std=gnu99 -ffreestanding -Wall -Wextra -fsanitize=undefined -fsanitize-undefined-trap-on-error
	cd generic_os/lib; i686-elf-gcc -c *.c -std=gnu99 -ffreestanding -Wall -Wextra -fsanitize=undefined -fsanitize-undefined-trap-on-error
	cd generic_os/drv; i686-elf-gcc -c *.c -std=gnu99 -ffreestanding -Wall -Wextra -fsanitize=undefined -fsanitize-undefined-trap-on-error
	cd generic_os/int; i686-elf-gcc -c *.c -std=gnu99 -ffreestanding -Wall -Wextra -mgeneral-regs-only -fsanitize=undefined -fsanitize-undefined-trap-on-error
	cd generic_os/lib; mv *.o ../
	cd generic_os/drv; mv *.o ../
	cd generic_os/int; mv *.o ../
	
	
	cd generic_os; i686-elf-gcc -T link.ld -ffreestanding -O2 -nostdlib -lgcc
	
	cd generic_os/; rm -f *.o
	cat bootloader.bin kernel.bin > image.img
	
	truncate -s 1474560 image.img
	rm -f bootloader.bin
	rm -f kernel.bin
	
clean:
	rm -f image.img