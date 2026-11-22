# Supertos Industries
#   generic-os makefile
ARCH = x86_64

SHRDIR    = shared
OUTDIR    = build

CFLAGS    = -std=gnu2x -ffreestanding -fPIC -MMD -MP -mno-red-zone -fno-stack-protector
WFLAGS    = -Wall -Wextra -Wformat=2 -Wstrict-prototypes -Wmissing-prototypes -Wredundant-decls -Wnested-externs -Wswitch-default -Wswitch-enum
LDFLAGS   = -nostdlib -nodefaultlibs -e KernelEntry -export-dynamic
DBG_FLAGS = $(if $(DEBUG),-g -O0,-O3) -fno-delete-null-pointer-checks
LIB_FLAGS = -L$(OUTDIR) -lshared -I$(SHRDIR) 
EFI_FLAGS = -Wall -Wextra -O2 -DGENERIC_LOADER -std=gnu2x -L../$(OUTDIR) -lshared -I../$(SHRDIR)

CC = gcc


all: $(OUTDIR)/generic-os.img
image: $(OUTDIR)/generic-os.img
kernel: $(OUTDIR)/kernel.elf
loader: $(OUTDIR)/loader.efi
files: kernel loader

.PHONY: clean


SHR_SRCS := $(shell find $(SHRDIR) -name "*.c")
SHR_OBJS := $(patsubst $(SHRDIR)/%.c, $(OUTDIR)/%.o, $(SHR_SRCS))


# === build === #
$(OUTDIR):
	@mkdir -p $(OUTDIR)


# === libshared.a === #
$(OUTDIR)/%.o: $(SHRDIR)/%.c | $(OUTDIR)
	@mkdir $(dir $@) -p
	@$(CC) $(CFLAGS) $(WFLAGS) -I$(SHRDIR) -c $< -o $@
	@echo "    CC $@"

$(OUTDIR)/libshared.a: $(SHR_OBJS)
	@ar rcs $@ $^


# === kernel.elf === #
$(OUTDIR)/kernel.elf: $(OUTDIR)/libshared.a
	@$(MAKE) -C ./kernel/ 	   \
		CFLAGS="$(CFLAGS)" 	   \
		WFLAGS="$(WFLAGS)" 	   \
		LDFLAGS="$(LDFLAGS)" 	   \
		DBG_FLAGS="$(DBG_FLAGS)" \
		OUTDIR="../$(OUTDIR)"  \
		SHRDIR="../$(SHRDIR)" \
		CC="$(CC)"

# === loader.efi === #
$(OUTDIR)/loader.efi: loader/*.c | $(SHR_OBJS)
	@mkdir $(OUTDIR)/loader -p
	@$(MAKE) -C uefi		 			\
		INDIR="../loader" 	\
		TARGET=loader.efi				\
		EXTRA_CFLAGS="$(EFI_FLAGS) $(DBG_FLAGS)" 	\
		OUTDIR="../$(OUTDIR)" 1>/dev/null \
		CC="$(CC)"
	@echo "    MAKE uefi/Makefile"



# === generic-os.img === #
$(OUTDIR)/generic-os.img: $(OUTDIR)/kernel.elf $(OUTDIR)/loader.efi | $(OUTDIR)
	@dd if=/dev/zero of=$(OUTDIR)/generic-os.img bs=1M count=64 2>/dev/null
	@mkfs.vfat $(OUTDIR)/generic-os.img 1>/dev/null
	@echo "    MKFS $@"
	@mmd -D o -i $(OUTDIR)/generic-os.img ::/EFI 2>/dev/null
	@echo "    MKFS ::/EFI"
	@mmd -D o -i $(OUTDIR)/generic-os.img ::/EFI/BOOT 2>/dev/null
	@echo "    MKFS ::/EFI/BOOT"
	@mmd -D o -i $(OUTDIR)/generic-os.img ::/ELF 2>/dev/null
	@echo "    MKFS ::/ELF"
	@mmd -D o -i $(OUTDIR)/generic-os.img ::/ELF/CORE 2>/dev/null
	@echo "    MKFS ::/ELF/CORE"
	@mcopy -i $(OUTDIR)/generic-os.img $(OUTDIR)/loader.efi ::/EFI/BOOT/BOOTX64.EFI 2>/dev/null
	@echo "    MCOPY ::/EFI/BOOT/BOOTX64.EFI "
	@mcopy -i $(OUTDIR)/generic-os.img $(OUTDIR)/kernel.elf ::/ELF/CORE/KERNEL.ELF 2>/dev/null
	@echo "    MCOPY ::/ELF/CORE/KERNEL.ELF"
	@echo "    Outputed to $(OUTDIR)/generic-os.img"

qemu:
	@echo "    QEMU"
	@qemu-system-x86_64 \
		-enable-kvm \
		-cpu host \
		-m 128M \
		-numa node,nodeid=0,cpus=0-1,memdev=mem0 \
		-object memory-backend-ram,id=mem0,size=64M \
		-object memory-backend-ram,id=mem1,size=64M \
		-numa node,nodeid=1,cpus=2-3,memdev=mem1 \
		-smp 4,sockets=1,cores=2,threads=2 \
		-bios /usr/share/ovmf/OVMF.fd \
		-d int \
		-drive file=$(OUTDIR)/generic-os.img,format=raw,if=virtio \
		-netdev user,id=net0 \
		-device virtio-net-pci,netdev=net0 \
		-vga virtio \
		-serial stdio \
		-display gtk,gl=on

clean:
	@find . -name '*.o' -type f -exec rm -f {} +
	@find . -name '*.out' -type f -exec rm -f {} +
	@find . -name '*.a' -type f -exec rm -f {} +
	@find . -name '*.d' -type f -exec rm -f {} +
	@find . -name '*.so' -type f -exec rm -f {} +
	@find . -name '*.img' -type f -exec rm -f {} +
	@find . -name '*.elf' -type f -exec rm -f {} +
	@find . -name '*.efi' -type f -exec rm -f {} +
	@rm -rf ./build
	@echo "    Cleaned"


