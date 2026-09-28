# :set noexpandtab

BIN = BOOTx64.EFI

DSOURCES = sources
DBUILD   = build
DESP     = $(DBUILD)/ESP
DBOOT    = $(DESP)/EFI/BOOT

CSOURCES = $(wildcard $(DSOURCES)/*.c)
COBJETS  = $(CSOURCES:$(DSOURCES)/%.c=$(DBUILD)/%.o)
CDEPS    = $(COBJETS:.o=.d)

RM    = /usr/bin/rm -f
MKDIR = /usr/bin/mkdir -p
QEMU  = /usr/bin/qemu-system-x86_64
OVMF  = /usr/share/ovmf/OVMF.fd

CC = /usr/bin/clang-19 \
	-target x86_64-unknown-windows

CFLAGS =         \
	-std=c17       \
	-Wall          \
	-Wextra        \
	-Wconversion   \
	-Werror        \
	-pedantic      \
	-mno-red-zone  \
	-ffreestanding \
	-fshort-wchar  \
	-nostdlib

# Temporary
CFLAGS += -O3 -Wno-unused -Wno-unused-parameter

LDFLAGS =                        \
	-Wl,-subsystem:efi_application \
	-Wl,-entry:EfiMain             \
	-fuse-ld=lld                   \
	-nostdlib

CDFLAGS = -MMD -MP -MT $@ -MF $(@:.o=.d)

QEMUFLAGS := -name TREFI

# The recommended way to use OVMF is with a pflash parameter.
# TODO: OVMF image can be splitted into CODE and VARS section.
QEMUFLAGS += -drive if=pflash,format=raw,unit=0,file=$(OVMF),readonly=on
# QEMUFLAGS += -bios /usr/share/ovmf/OVMF.fd

# QEMU can emulate a virtual drive with a FAT filesystem.
# https://en.wikibooks.org/wiki/QEMU/Devices/Storage#Virtual_FAT_filesystem_(VVFAT)
QEMUFLAGS += -drive file=fat:rw:$(DESP),media=disk,format=raw

# Ensure that the mouse cursor is visible when
# interacting with the virtual machine.
QEMUFLAGS += -display default,show-cursor=on

# Prevent recent versions of QEMU from attempting
# a PXE (network) boot when no boot disk is found.
QEMUFLAGS += -net none

.PHONY : all dump_macros check clean cleanall mrproper run
.DEFAULT_GOAL := all

all: $(DBOOT)/$(BIN)
	@echo "-->" ./$(notdir $<)

dump_macros:
	@$(CC) -dM -E -x c /dev/null

check: $(CSOURCES)
	@$(CC) $(CFLAGS) -fsyntax-only $^

$(DBOOT)/$(BIN): $(COBJETS)
	@$(MKDIR) $(DBOOT)
	@echo Generating Code...
	@$(CC) $^ -o $@ $(LDFLAGS)

$(COBJETS): $(DBUILD)/%.o: $(DSOURCES)/%.c
	@echo $(notdir $<)
	@$(CC) -c $< -o $@ $(CFLAGS) $(CDFLAGS)

-include $(CDEPS)

clean:
	@$(RM) $(COBJETS)

cleanall: clean
	@$(RM) $(CDEPS)

mrproper: cleanall
	@$(RM) $(DBOOT)/$(BIN)

run: $(DBOOT)/$(BIN)
	@echo Run $(BIN)
	@$(QEMU) $(QEMUFLAGS)
