# !!!AI-Generated fragment!!!
# Configuration

ARCH = x86_64
INDIR = .
OUTDIR = build
TARGET = loader.efi

CC = $(ARCH)-elf-gcc
LD = $(ARCH)-elf-ld
AR = $(ARCH)-elf-ar
OBJCOPY = $(ARCH)-elf-objcopy

ifeq ($(ARCH),x86_64)
CC = gcc
LD = ld
AR = ar
OBJCOPY = objcopy
endif

OUTDIR := $(patsubst %/,%,$(OUTDIR))

# Compiler flags

CFLAGS += -fshort-wchar
CFLAGS += -fno-strict-aliasing
CFLAGS += -ffreestanding
CFLAGS += -fno-stack-protector
CFLAGS += -fno-stack-check
CFLAGS += -fpic -fPIC
CFLAGS += -Wno-builtin-declaration-mismatch
CFLAGS += -I.
CFLAGS += -I./uefi
CFLAGS += -I/usr/include
CFLAGS += -I/usr/include/efi
CFLAGS += -I/usr/include/efi/protocol
CFLAGS += -I/usr/include/efi/$(ARCH)
CFLAGS += -D__$(ARCH)__

ifeq ($(ARCH),x86_64)
CFLAGS += -DHAVE_USE_MS_ABI
CFLAGS += -mno-red-zone
CFLAGS += -maccumulate-outgoing-args
endif

LDFLAGS += -nostdlib
LDFLAGS += -shared
LDFLAGS += -Bsymbolic
LDFLAGS += -L$(OUTDIR)

EFI_LDS := elf_$(ARCH)_efi.lds
CRT_SRC := crt_$(ARCH).c

# EFI formats

ifeq ($(ARCH),x86_64)
ELF_FORMAT = elf64-x86-64
EFI_FORMAT = efi-app-x86_64
else ifeq ($(ARCH),ia32)
ELF_FORMAT = elf32-i386
EFI_FORMAT = efi-app-ia32
else ifeq ($(ARCH),aarch64)
ELF_FORMAT = elf64-littleaarch64
EFI_FORMAT = pei-aarch64-little
else ifeq ($(ARCH),riscv64)
ELF_FORMAT = elf64-littleriscv
EFI_FORMAT = pei-riscv64-little
else
$(error Unsupported ARCH: $(ARCH))
endif

# Sources

APP_C_SRCS := $(wildcard $(INDIR)/*.c)
APP_S_SRCS := $(wildcard $(INDIR)/*.S)

LIB_C_SRCS := $(filter-out $(wildcard crt_*.c),$(wildcard *.c))
LIB_S_SRCS := $(wildcard *.S)

APP_OBJS := $(patsubst $(INDIR)/%.c,$(OUTDIR)/app/%.c.o,$(APP_C_SRCS))
APP_OBJS += $(patsubst $(INDIR)/%.S,$(OUTDIR)/app/%.S.o,$(APP_S_SRCS))

LIB_OBJS := $(patsubst %.c,$(OUTDIR)/lib/%.c.o,$(LIB_C_SRCS))
LIB_OBJS += $(patsubst %.S,$(OUTDIR)/lib/%.S.o,$(LIB_S_SRCS))

CRT_OBJ := $(OUTDIR)/uefi/crt_$(ARCH).o
SO_FILE := $(OUTDIR)/$(TARGET).so
EFI_FILE := $(OUTDIR)/$(TARGET)

# Build

.PHONY: all

all: $(EFI_FILE)

$(OUTDIR)/app $(OUTDIR)/lib $(OUTDIR)/uefi:
	@mkdir -p $@

# Application objects

$(OUTDIR)/app/%.c.o: $(INDIR)/%.c | $(OUTDIR)/app
	@$(CC) $(EXTRA_CFLAGS) $(CFLAGS) -c $< -o $@

$(OUTDIR)/app/%.S.o: $(INDIR)/%.S | $(OUTDIR)/app
	@$(CC) $(EXTRA_CFLAGS) $(CFLAGS) -c $< -o $@

# Library objects

$(OUTDIR)/lib/%.c.o: %.c | $(OUTDIR)/lib
	@$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/lib/%.S.o: %.S | $(OUTDIR)/lib
	@$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/libuefi.a: $(LIB_OBJS) | $(OUTDIR)
	#rm -f $@
	@$(AR) rcs $@ $^

# CRT

$(CRT_OBJ): $(CRT_SRC) | $(OUTDIR)/uefi
	@$(CC) $(CFLAGS) -c $< -o $@

# Linking

$(SO_FILE): $(APP_OBJS) $(CRT_OBJ) $(OUTDIR)/libuefi.a
	@$(LD) $(LDFLAGS) -T $(EFI_LDS) -o $@ \
		$(CRT_OBJ) \
		$(APP_OBJS) \
		$(OUTDIR)/libuefi.a

$(EFI_FILE): $(SO_FILE)
	@$(OBJCOPY) \
		-I $(ELF_FORMAT) \
		-j .text \
		-j .sdata \
		-j .data \
		-j .dynamic \
		-j .dynsym \
		-j .rel \
		-j .rela \
		-j .rel.* \
		-j .rela.* \
		-j .reloc \
		--output-target $(EFI_FORMAT) \
		--subsystem=10 \
		$< $@

# Cleanup

clean:
	#rm -rf $(OUTDIR)

distclean: clean
