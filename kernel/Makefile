# Supertos Industries (2012 - 2026)
#   Kernel Makefile
ARCH = x86_64

LIB_FLAGS = -L$(OUTDIR) -lshared -I$(SHRDIR) -I.

.PHONY: $(OUTDIR)/kernel.elf 

KRN_SRCS := $(shell find . -name "*.c")
KRN_OBJS := $(patsubst ./%.c, $(OUTDIR)/%.o, $(KRN_SRCS))


$(OUTDIR)/%.o: ./%.c
	@-mkdir -p $(dir $@)
	@$(CC) $(DBG_FLAGS) $(WFLAGS) $(CFLAGS) -c $< -o $@ $(LIB_FLAGS)
	@echo "    CC $@"

$(OUTDIR)/kernel.elf: $(KRN_OBJS) | $(OUTDIR)
	@-mkdir -p $(dir $@)
	@$(CC) $(DBG_FLAGS) $(LDFLAGS) -I. -o $@ $(KRN_OBJS) $(LIB_FLAGS)
	@echo "    LD $@"