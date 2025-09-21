# Supertos Industries
#   generic-os makefile

ARCH = x86_64
CFLAGS = -pedantic -Wall -Wextra --ansi -std=c99 -O2
USE_GCC = 1
OUTDIR = build

.PHONY: debug all run clean
all: run

debug: $(OUTDIR)/generic-os.img
	qemu-system-x86_64 -bios /usr/share/qemu/OVMF.fd -drive format=raw,file=$(OUTDIR)/generic-os.img -serial stdio -d int,cpu_reset

run: $(OUTDIR)/generic-os.img
	qemu-system-x86_64 -bios /usr/share/qemu/OVMF.fd -drive format=raw,file=$(OUTDIR)/generic-os.img

$(OUTDIR):
	mkdir -p $(OUTDIR)
		
$(OUTDIR)/loader.efi: loader/*.c | $(OUTDIR)
	$(MAKE) -f ./Makefile_loader SRCS="$(wildcard loader/*.c)" \
		ARCH=$(ARCH) TARGET=loader.efi \
		EXTRA_CFLAGS="$(CFLAGS)" USE_GCC=1 && \
	mv loader.efi $(OUTDIR)/loader.efi 

$(OUTDIR)/generic-os.img: $(OUTDIR)/kernel.elf $(OUTDIR)/loader.efi | $(OUTDIR)
	dd if=/dev/zero of=$(OUTDIR)/generic-os.img bs=1M count=64 && \
	mkfs.vfat $(OUTDIR)/generic-os.img && \
	mmd -i $(OUTDIR)/generic-os.img ::/EFI && \
	mmd -i $(OUTDIR)/generic-os.img ::/EFI/BOOT && \
	mmd -i $(OUTDIR)/generic-os.img ::/ELF && \
	mmd -i $(OUTDIR)/generic-os.img ::/ELF/CORE && \
	mcopy -i $(OUTDIR)/generic-os.img $(OUTDIR)/loader.efi ::/EFI/BOOT/BOOTX64.EFI && \
	mcopy -i $(OUTDIR)/generic-os.img $(OUTDIR)/kernel.elf ::/ELF/CORE/KERNEL.ELF

# === CORE ===
KERNEL_SRCS := $(wildcard kernel/*.c)
KERNEL_OBJS := $(patsubst kernel/%.c, $(OUTDIR)/%.o, $(KERNEL_SRCS))

$(OUTDIR)/kernel.elf: $(KERNEL_OBJS) | $(OUTDIR)
	gcc -nostdlib -ffreestanding -fPIC -pie -Wl,-e,KernelEntry -o $@ $(KERNEL_OBJS)

$(OUTDIR)/%.o: kernel/%.c | $(OUTDIR)
	gcc $(CFLAGS) -nostdlib -ffreestanding -fPIC -c $< -o $@

clean:
	find . -name '*.o' -type f -exec rm -f {} +
	find . -name '*.img' -type f -exec rm -f {} +
	find . -name '*.elf' -type f -exec rm -f {} +
	find . -name '*.efi' -type f -exec rm -f {} +
	find . -name '*.out' -type f -exec rm -f {} +
	find . -name '*.a' -type f -exec rm -f {} +
