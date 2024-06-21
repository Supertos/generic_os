![Generic OS logo](https://github.com/Supertos/generic_os/blob/main/logo.png?raw=true)  
# Generic OS official repo 
### Introduction
Generic OS - is minimalistic 32bit operating system written in Assembly and C languages made as a learning project. Generic OS will contain bootloader and a kernel.

### Generic Bootloader features
* A20 Line enable.
* Memory map write at 0x500.
* Entering 32bit protected mode with paging.
* Loading 8KiB of floppy drive at 0x8000 and giving control to it.

### Generic OS features
*TBA

### How to build.
Run make in root directory. main.img will be produced. This is the Generic OS image. Drop it in virtual machine and run.
