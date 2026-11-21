![GenericOS logo](https://github.com/Supertos/generic_os/blob/main/logo.png?raw=true)  
# GenericOS
GenericOS - Minimalistic NUMA-aware x86-64 operating system written in Assembly and C23.

This repository contains both GenericOS and its loader - Generic Bootloader. Note that GenericOS cannot be loaded using grub or any other standard bootloader by default. However, a fork of the kernel may wrap its ```shared/BootInfo.h``` in compatibility layer to support any bootloader.

GenericOS uses Apache License 2.0, full text can be found in LICENSE at the root of the repository.

## Minimal System Requirements
- Intel - Ivy Bridge CPU/AMD - Zen 3 CPU (PCID, RDRAND and APIC support)
- 23MiB on each NUMA node, or 46MiB total if UMA
- Virtualization (or nested virtualization if using WSL2) support for running on QEMU recommended

## Known Bugs
- On UMA machines, MachineInfo will always report single core since it checks for SRAT table such machines usually lack
- If memory is less than minimal requirements, kernel may trigger #PF instead of calling KernelCrashUndBurn properly

## Development status
Components completed as of 22.11.2026:
| Task | Started | Completed |
|-|-|-|
| NUMA-aware Physical Memory Manager | 10.01.2024 | 12.11.2026 |
| Generic Bootloader | 10.01.2024 | 12.06.2025 |
| NUMA-aware Virtual Memory Manager | 18.01.2026 | 08.07.2026 |

Roadmap is as follows:
| Task | Started | ETA |
|-|-|-|
| Device Manager | 15.11.2026 | Q1 2027 |
| Basic Video/Input Drivers || Q1 2027 |
| Basic Mass Storage Device Driver || Q2 2027 |
| Filesystem Manager || Q2 2027 |
| Scheduler, Userland, Syscalls || Q3 2027 |
| Network Manager || Q1 2028 |
| Proper Booting Sequence + Shell || Q2 2028 |

Roadmap is subject to change.

## Getting started
### Prerequisites

Run ```sudo apt update``` before installing prerequisites.

| Prerequisite | Ubuntu Installation | Note |
|-|-|-|
| ```mtools``` | ```sudo apt install mtools``` | To create ready-to-run image |
| ```qemu``` | ```sudo apt install qemu-system-x86``` | For debugging purposes |
| ```gcc-15``` (or newer) | ```sudo apt install gcc-15``` | To compile kernel & bootloader |
| ```binutils``` | ```sudo apt install binutils``` | Linking and objcopy of the loader |
|```ovmf```| ```sudo apt install ovmf``` | UEFI data and code for QEMU |

### Building
If your default gcc is older than gcc-15, specify exact version via ```CC=gcc-15``` option.
You can also specify output folder with ```BUILD=build```.

1. __Clone this repository__:
> ```bash
> git clone https://github.com/Supertos/generic_os
> cd generic_os
> ```

2. __Run makefile to compile ready-to-run image__:
> ```bash
> make
> ```
> Will create build/generic-os.img
> Alternatively, you may build just the bootloader (build/loader.efi) or just the kernel (build/kernel.elf) using
> ```bash
> make kernel
> ```
> or
> ```bash
> make loader
> ```
> respectively.

3. __You may optionally run test (NUMA) configuration in QEMU__:
> ```bash
> make qemu
> ```

## Contributing Policy
At the moment, I do not accept any contribution to this project. Once main components are implemented and documentation's written, I will begin accepting those.

## Third-Party Code and Notice
- **bzt (https://codeberg.org/bzt/)** - POSIX-UEFI (makefile modified) - https://codeberg.org/bzt/posix-uefi (MIT License)
- **Ivan Sigaev (@ivan-r-sigaev)** - Rust-style RWLock implementation

