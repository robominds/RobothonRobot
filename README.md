# Robothon Robot Firmware

Vintage embedded software from the 1999 Robothon competition - a fire-fighting, line-following, and maze-navigating robot built on the M68332 microcontroller.

## Overview

This project contains firmware for a Motorola 68332-based robot that competed in the 1999 Robothon competition. The robot featured:
- Fire detection and extinguishing capabilities
- Line following navigation
- Maze solving algorithms
- Distance sensing
- PWM motor control

## Hardware

- **CPU**: Motorola 68332 (cpu32 architecture)
- **Target Board**: M68332BCC Business Card Computer
- **Memory**: 116KB RAM (0x3000-0x1d000)

## Building

### Prerequisites

Install the m68k cross-compiler toolchain:

```bash
brew install m68k-elf-gcc
```

### Build Commands

```bash
# Build the firmware
make

# Build with debug symbols removed
make release

# Clean build artifacts
make clean

# View disassembly and symbols
make dump

# View disassembly with source
make dumps

# View hex dump of ROM file
make hexdump
```

### Build Outputs

- `bmbinary` - ELF executable
- `bmbinary.rom` - Raw binary format
- `bmbinary.srec` - Motorola S-record format

## Project Structure

### Directory Layout
```
src/          - C source files
include/      - Header files
asm/          - Assembly source files
unused/       - Historical/unused files
build/        - Build artifacts (generated)
```

### Core Modules
- `rob.c` - Main robot control loop
- `nav.c`, `navutil.c` - Navigation system
- `guid.c`, `mzguid.c` - Guidance algorithms
- `line.c` - Line following
- `flame.c` - Fire detection

### Hardware Drivers
- `tpu.c` - Timer Processing Unit
- `pwm.c` - Pulse Width Modulation (motor control)
- `fqd.c` - Frequency/Quadrature Decoder (encoders)
- `a2d.c` - Analog-to-Digital converter
- `dist.c` - Distance sensors
- `scibuff.c` - Serial communication

### Utilities
- `stdio.c` - Basic I/O functions
- `draw.c` - Display/drawing functions
- `cntr.c` - Counter utilities

### Startup & System
- `asm/crt0x.s` - C runtime startup code
- `asm/scibuff.s` - Serial interrupt handlers
- `platform.ld` - Linker script
- `332.ld` - Memory layout

## Memory Map

```
0x0000 - 0x0400   Vector Base Register (VBR)
0x3000 - 0x1d000  RAM (116KB)
```

## Legacy Files

Historical files not part of the current build are preserved in the `unused/` directory.

## License

Historical educational project - 1999
