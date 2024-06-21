![Generic OS logo](https://github.com/Supertos/generic_os/blob/main/logo.png?raw=true)  
# Generic OS official repo 
### Introduction
Generic OS - is the minimalistic 32bit operating system written in Assembly and C languages made for learning purpose. Generic OS contains bootloader and a kernel.

### Generic Bootloader features
* A20 Line enable.
* Memory map write at 0x500.
* Entering 32bit protected mode with paging.
* Loading 8KiB of floppy drive at 0x8000 and passing control to it.

### Generic OS features
*TBA

### How to build.
Run make in root directory. Generic OS image called main.img will be produced. Image may be burned onto the drive or used in virtual machines.
