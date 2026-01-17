# ==============================================================================
# Makefile for M68332 Robothon Robot Firmware
# Target: 68332 CPU (cpu32 architecture)
# ==============================================================================

# Directory structure
SRCDIR   = src
INCDIR   = include
ASMDIR   = asm

# Source files to compile into the binary
SRCS = rob.c scibuff.c fqd.c pwm.c tpu.c nav.c navutil.c cntr.c guid.c \
       mzguid.c dist.c line.c draw.c a2d.c flame.c stdio.c exit.c

# Target CPU architecture (68332 uses cpu32)
CPU = cpu32

# Toolchain prefix for m68k cross-compiler
# Install with: brew install m68k-elf-gcc
PREFIX = m68k-elf

# ==============================================================================
# Build Configuration
# ==============================================================================
BUILDDIR = build

# Toolchain
CC      = $(PREFIX)-gcc
LD      = $(PREFIX)-ld
OBJCOPY = $(PREFIX)-objcopy
OBJDUMP = $(PREFIX)-objdump

# Compiler flags
CFLAGS = -m$(CPU) -Wall -Wextra -g -static -I$(INCDIR) -I../include -msoft-float -MMD -MP -O

# Linker flags
LFLAGS = --script=platform.ld

# Object files
OBJS  = $(patsubst %.c,$(BUILDDIR)/%.c.o,$(SRCS))
OBJS := $(patsubst %.S,$(BUILDDIR)/%.S.o,$(OBJS))
OBJS := $(patsubst %.s,$(BUILDDIR)/%.s.o,$(OBJS))
DEPS  = $(OBJS:.o=.d)

# Add source directory to VPATH for automatic source file lookup
vpath %.c $(SRCDIR)
vpath %.s $(ASMDIR)
vpath %.S $(ASMDIR)

# ==============================================================================
# Build Targets
# ==============================================================================
.PHONY: all release clean rom dump dumps hexdump

all: bmbinary rom

# Release build with optimizations
release: CFLAGS += -DNDEBUG
release: all

# ==============================================================================
# Compilation Rules
# ==============================================================================

# Create build directory
$(BUILDDIR):
	mkdir -p $@

# Compile C files
$(BUILDDIR)/%.c.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile assembly files (.S)
$(BUILDDIR)/%.S.o: %.S
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile assembly files (.s)
$(BUILDDIR)/%.s.o: %.s
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c -o $@ $<

# Include dependency files
-include $(DEPS)

# Build startup code in build directory
$(BUILDDIR)/crt0x.s.o: $(ASMDIR)/crt0x.s
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c -o $@ $<

# Main binary depends on startup code
bmbinary: $(OBJS) $(BUILDDIR)/crt0x.s.o
	$(LD) -o $@ $(OBJS) $(LFLAGS)

# ==============================================================================
# Utility Targets
# ==============================================================================

# Clean build ar
clean:
	rm -rf $(BUILDDIR)/*
	rm -f bmbinary* crt0x.o

# Generate ROM files
rom: bmbinary
	$(OBJCOPY) -O binary bmbinary bmbinary.rom
	$(OBJCOPY) -O srec bmbinary bmbinary.srec

# Dump binary information
dump: bmbinary
	$(OBJDUMP) -mm68k:$(CPU) -belf32-m68k -st -j.evt bmbinary
	$(OBJDUMP) -mm68k:$(CPU) -belf32-m68k -dt -j.text bmbinary
	$(OBJDUMP) -mm68k:$(CPU) -belf32-m68k -st -j.rodata -j.data -j.bss -j.heap -j.stack bmbinary

# Dump with source
dumps: bmbinary
	$(OBJDUMP) -mm68k:$(CPU) -belf32-m68k -st -j.evt bmbinary
	$(OBJDUMP) -mm68k:$(CPU) -belf32-m68k -St -j.text bmbinary
	$(OBJDUMP) -mm68k:$(CPU) -belf32-m68k -st -j.rodata -j.data -j.bss -j.heap -j.stack bmbinary

# Hex dump of ROM file
hexdump: rom
	hexdump -C bmbinary.rom
