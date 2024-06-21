generic_bootloader: 
	cd generic_bootloader
	nasm main.asm -o main.img
	truncate -s 1474560 main.img
	cd ..
	
clean:
	cd generic_bootloader
	rm -f main.img
	cd ..