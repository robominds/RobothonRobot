# Product Requirements Document: RobothonRobot

## Document Information
- **Product Name**: RobothonRobot
- **Version**: Robothon1999OriginalUpdatedtoMacOS
- **Date**: February 7, 2026
- **Status**: Legacy System - Modernization Update

---

## 1. Executive Summary

### 1.1 Product Overview
RobothonRobot is an embedded robotics control system originally developed in 1999 for the Robothon competition. The system is designed to control an autonomous mobile robot capable of multiple mission types including fire-fighting, line following, and maze navigation. The codebase has been updated to compile on modern macOS (Apple Silicon) systems using the m68k-elf-gcc cross-compiler.

### 1.2 Target Platform
- **Microcontroller**: Motorola 68332 (CPU32 architecture)
- **Clock Speed**: 16.78 MHz
- **Development Platform**: macOS (Apple Silicon)
- **Toolchain**: m68k-elf-gcc cross-compiler

### 1.3 Mission Profiles
The system supports multiple mission configurations:
1. Line following mode
2. Guided navigation mode
3. Maze navigation mode
4. Drawing/mapping mode
5. Fire detection and extinguishing

---

## 2. Product Goals and Objectives

### 2.1 Primary Goals
- Provide a reliable, real-time control system for autonomous robot navigation
- Support multiple competitive mission types with configurable modes
- Maintain precise dead-reckoning navigation with encoder-based odometry
- Enable cross-compilation on modern macOS development systems

### 2.2 Success Criteria
- System boots and initializes all hardware subsystems
- Navigation accuracy within acceptable tolerance for competition requirements
- Real-time control loop maintains 100 Hz update rate
- Sensor data acquisition and processing meets timing requirements
- Successful compilation on macOS using m68k-elf-gcc toolchain

---

## 3. System Architecture

### 3.1 Hardware Architecture
```
┌─────────────────────────────────────────┐
│     Motorola 68332 Microcontroller      │
│         (CPU32, 16.78 MHz)              │
└──────────────┬──────────────────────────┘
               │
       ┌───────┴────────┐
       │                │
   ┌───▼────┐      ┌───▼────┐
   │  TPU   │      │  QSM   │
   │ Timer  │      │Serial  │
   │ Module │      │ Module │
   └───┬────┘      └───┬────┘
       │               │
   ┌───▼───────────────▼────┐
   │   Sensor & Actuator    │
   │      Interfaces        │
   └────────────────────────┘
```

### 3.2 Software Architecture

#### 3.2.1 Real-Time Control Loop Structure
- **100 Hz Main Interrupt**: Navigation updates, sensor processing
- **10 Hz Sub-Tasks**: Distance sensing, guidance, control updates
- **Background Tasks**: Serial communication, status monitoring

#### 3.2.2 Module Hierarchy
```
main (rob.c)
├── Hardware Initialization
│   ├── Interrupt Vector Setup
│   ├── TPU (Timer Processing Unit)
│   ├── QSM (Queued Serial Module)
│   └── System Clock Configuration
├── Sensor Subsystems
│   ├── Navigation (nav.c, navutil.c)
│   ├── Distance Sensing (dist.c)
│   ├── Line Following (line.c)
│   ├── Flame Detection (flame.c)
│   └── A/D Conversion (a2d.c)
├── Control Subsystems
│   ├── PWM Motor Control (pwm.c)
│   ├── Encoder Processing (cntr.c)
│   └── FQD (Frequency/Quadrature Decoder) (fqd.c)
└── Mission Control
    ├── Guidance (guid.c)
    ├── Maze Guidance (mzguid.c)
    └── Drawing Mode (draw.c)
```

---

## 4. Functional Requirements

### 4.1 Core System Requirements

#### FR-1: System Initialization
- **Priority**: Critical
- **Description**: The system shall initialize all hardware peripherals and interrupt vectors on startup
- **Acceptance Criteria**:
  - All 256 interrupt vectors initialized to default handler
  - System clock configured to 16.78 MHz
  - TPU, QSM, and sensor modules successfully initialized
  - Mission-specific modules loaded based on configuration flags

#### FR-2: Real-Time Control Loop
- **Priority**: Critical
- **Description**: The system shall maintain a 100 Hz periodic interrupt for real-time control
- **Acceptance Criteria**:
  - 100 Hz interrupt fires consistently with <1% jitter
  - Navigation updates execute within allocated time slot
  - Overrun detection mechanism tracks missed deadlines
  - 10 Hz sub-tasks execute on designated frames

#### FR-3: Start Detection
- **Priority**: High
- **Description**: The system shall detect race start condition through accelerometer monitoring
- **Acceptance Criteria**:
  - Accelerometer data sampled and filtered
  - Start threshold configurable
  - System transitions from idle to active mode on detection
  - Timing accurate to within 100ms

### 4.2 Navigation Requirements

#### FR-4: Dead Reckoning Navigation
- **Priority**: Critical
- **Description**: The system shall maintain accurate position and heading estimation using wheel encoders
- **Acceptance Criteria**:
  - Position tracked in X and Y coordinates
  - Heading maintained in angular units
  - Encoder counts processed every 100 Hz cycle
  - Position accuracy: ±2% over 10-meter course

#### FR-5: Velocity and Angular Rate Estimation
- **Priority**: High
- **Description**: The system shall compute linear velocity and angular rate from encoder data
- **Acceptance Criteria**:
  - Velocity computed from encoder deltas
  - Angular rate derived from differential wheel speeds
  - Values updated at 100 Hz
  - Filtered output with minimal lag (<50ms)

### 4.3 Sensor Processing Requirements

#### FR-6: Distance Sensing
- **Priority**: High
- **Description**: The system shall provide ultrasonic or IR distance measurements
- **Acceptance Criteria**:
  - Multiple distance sensors supported
  - Measurements triggered at 10 Hz
  - Range: 2-150 cm (configurable based on sensor)
  - Resolution: 1 cm

#### FR-7: Line Detection
- **Priority**: High (when line following mode enabled)
- **Description**: The system shall detect line position using IR sensor array
- **Acceptance Criteria**:
  - 5-channel IR sensor array supported
  - Sensor values read and processed
  - Line position computed from sensor readings
  - Update rate: 100 Hz

#### FR-8: Flame Detection
- **Priority**: Medium (when fire-fighting mode enabled)
- **Description**: The system shall detect flame presence and direction
- **Acceptance Criteria**:
  - Flame sensor magnitude readable
  - Direction estimation supported
  - Fan control interface for extinguishing

#### FR-9: Analog-to-Digital Conversion
- **Priority**: High
- **Description**: The system shall convert analog sensor signals to digital values
- **Acceptance Criteria**:
  - Multiple A/D channels supported
  - 10-bit or better resolution
  - Conversion triggered by control loop
  - Values available for sensor processing

### 4.4 Actuator Control Requirements

#### FR-10: PWM Motor Control
- **Priority**: Critical
- **Description**: The system shall control motor speed and direction via PWM signals
- **Acceptance Criteria**:
  - Independent control of left and right motors
  - PWM frequency: >20 kHz (above audible range)
  - Duty cycle range: 0-100%
  - Direction control (forward/reverse)

#### FR-11: Encoder Interface
- **Priority**: Critical
- **Description**: The system shall read quadrature encoder signals for odometry
- **Acceptance Criteria**:
  - Quadrature decoding for direction sensing
  - Counter overflow handling
  - Supports multiple encoder channels
  - Count rate: >10,000 counts/second per channel

### 4.5 Guidance and Control Requirements

#### FR-12: Waypoint Navigation
- **Priority**: High (when guided mode enabled)
- **Description**: The system shall navigate to predefined waypoints
- **Acceptance Criteria**:
  - Waypoint table definable at compile time
  - Automatic waypoint sequencing
  - Velocity and heading commands generated
  - Waypoint completion detection

#### FR-13: Maze Navigation
- **Priority**: Medium (when maze mode enabled)
- **Description**: The system shall navigate through maze using wall-following or mapping algorithm
- **Acceptance Criteria**:
  - Distance sensor data used for wall detection
  - Navigation decisions made at intersections
  - Goal-seeking behavior implemented

#### FR-14: Drawing/Mapping Mode
- **Priority**: Low (when drawing mode enabled)
- **Description**: The system shall create a map or drawing based on sensor data
- **Acceptance Criteria**:
  - Position tracking for path recording
  - Configurable drawing patterns
  - Integration with navigation system

### 4.6 Communication Requirements

#### FR-15: Serial Communication
- **Priority**: Medium
- **Description**: The system shall provide serial communication for debugging and monitoring
- **Acceptance Criteria**:
  - Buffered serial I/O
  - Configurable baud rate
  - Transmit and receive functional
  - Echo capability for terminal interaction

#### FR-16: Status Reporting
- **Priority**: Medium
- **Description**: The system shall output status information for monitoring
- **Acceptance Criteria**:
  - Sensor values displayed
  - Navigation state reported
  - Format: hexadecimal or human-readable
  - Update rate: Limited by serial bandwidth

---

## 5. Non-Functional Requirements

### 5.1 Performance Requirements

#### NFR-1: Real-Time Responsiveness
- Interrupt latency: <100 microseconds
- Control loop execution time: <8 milliseconds (80% of 10ms period)
- Sensor processing: <2 milliseconds per 10 Hz cycle

#### NFR-2: Timing Determinism
- Interrupt service routine execution time shall be bounded
- Overrun detection for all time-critical tasks
- Graceful degradation when timing constraints violated

#### NFR-3: Memory Footprint
- ROM/Flash usage: <64 KB
- RAM usage: <16 KB
- Stack depth: <2 KB

### 5.2 Reliability Requirements

#### NFR-4: Fault Tolerance
- Illegal instruction trap handler prevents runaway code
- Watchdog timer (if available) prevents system lockup
- Sensor validation prevents erroneous readings

#### NFR-5: Robustness
- System shall recover from transient sensor failures
- Navigation shall degrade gracefully with encoder errors
- Control outputs shall default to safe state on error

### 5.3 Maintainability Requirements

#### NFR-6: Code Organization
- Modular design with clear separation of concerns
- Header files define module interfaces
- Source files contain implementation
- Configuration via compile-time flags (mssndef.h)

#### NFR-7: Build System
- Makefile-based build process
- Dependency tracking for incremental builds
- Multiple output formats (ELF, ROM binary, S-record)
- Support for debug and release configurations

### 5.4 Portability Requirements

#### NFR-8: Cross-Platform Development
- Code shall compile on macOS (Apple Silicon)
- Use of m68k-elf-gcc toolchain
- Platform-independent hardware abstraction where feasible
- Linker script defines memory layout

---

## 6. Configuration Management

### 6.1 Mission Configuration
Mission modes configured via `mssndef.h`:
```c
#define mssnline   0  // Line following mode
#define mssnguid   1  // Guided navigation mode
#define mssnmzguid 0  // Maze navigation mode
#define mssndraw   0  // Drawing mode
```

### 6.2 Hardware Configuration
- Target CPU: cpu32 (Motorola 68332)
- Compiler: m68k-elf-gcc
- Optimization level: -O (configurable)
- Memory map defined in `platform.ld` and `332.ld`

### 6.3 Build Targets
- `all`: Build complete binary
- `bmbinary`: Link executable
- `rom`: Generate ROM binary and S-record
- `clean`: Remove build artifacts
- `dump`/`dumps`: Disassemble sections
- `hexdump`: Display ROM in hex

---

## 7. System Modules

### 7.1 Hardware Interface Modules

| Module | File | Purpose |
|--------|------|---------|
| TPU | tpu.c/h | Timer Processing Unit control |
| QSM | qsm_reg.h | Queued Serial Module registers |
| PWM | pwm.c/h | Pulse Width Modulation for motors |
| FQD | fqd.c/h | Frequency/Quadrature Decoder |
| A/D | a2d.c/h | Analog-to-Digital conversion |

### 7.2 Sensor Modules

| Module | File | Purpose |
|--------|------|---------|
| Distance | dist.c/h | Ultrasonic/IR distance sensing |
| Line | line.c/h | Line following sensor array |
| Flame | flame.c/h | Fire detection sensors |
| Counter | cntr.c/h | Encoder counter processing |

### 7.3 Navigation and Control Modules

| Module | File | Purpose |
|--------|------|---------|
| Navigation | nav.c/h, navutil.c/h | Dead-reckoning navigation |
| Guidance | guid.c/h | Waypoint-based guidance |
| Maze Guidance | mzguid.c/h | Maze navigation logic |
| Drawing | draw.c/h | Mapping/drawing mode |

### 7.4 Utility Modules

| Module | File | Purpose |
|--------|------|---------|
| Serial I/O | scibuff.c/h, scibuff.s | Buffered serial communication |
| Standard I/O | stdio.c/h | Printf and basic I/O |
| Math | math.c/h | Mathematical utilities |
| Binary | binary.c/h | Binary data handling |
| Message | message.c/h | Message formatting |

### 7.5 Application Modules

| Module | File | Purpose |
|--------|------|---------|
| Main | rob.c | Main control loop and initialization |
| Command Parser | cmdparse.c/h | Command line interface |
| Exit | exit.c | Program termination |
| Profile | profile.c/h | Performance profiling |

---

## 8. Data Structures and Interfaces

### 8.1 Navigation State
```
Position: (X, Y) coordinates
Heading: Angular orientation
Velocity: Linear speed
Angular Rate: Rotational speed
Encoder Counts: Left and right wheel counts
```

### 8.2 Sensor Data
```
Distance Sensors: Array of range measurements
Line Sensors: 5-channel intensity values
Flame Sensor: Magnitude and direction
A/D Channels: Multi-channel analog readings
```

### 8.3 Control Outputs
```
PWM Commands: Left and right motor duty cycles
Fan Control: On/Off for flame extinguishing
```

### 8.4 Guidance Commands
```
Velocity Command: Target speed
Heading Command: Target direction
Waypoint Index: Current navigation waypoint
```

---

## 9. Timing and Scheduling

### 9.1 Interrupt Schedule

**100 Hz Main Interrupt (every 10 ms)**
- Frame 0: Distance sensor trigger, navigation update
- Frame 1: Counter update, navigation update
- Frame 2: Navigation update
- Frame 3: Counter update, navigation update
- Frame 4: Guidance update (guid/mzguid/draw), navigation update
- Frame 5: Counter update, navigation update
- Frame 6: Distance read (maze mode), navigation update
- Frame 7: Counter update, navigation update
- Frame 8: Navigation update
- Frame 9: Counter update, navigation update

**A/D Conversion**: Triggered at 10 Hz (on frame transition)

### 9.2 Background Tasks
- Serial communication (polling)
- Status output
- Command processing (if implemented)

---

## 10. Development and Build Environment

### 10.1 Required Tools
- **Compiler**: m68k-elf-gcc (installable via Homebrew on macOS)
- **Assembler**: m68k-elf-as (included with gcc)
- **Linker**: m68k-elf-ld (included with gcc)
- **Utilities**: m68k-elf-objcopy, m68k-elf-objdump
- **Build System**: GNU Make

### 10.2 Installation (macOS)
```bash
brew install m68k-elf-gcc
```

### 10.3 Build Process
```bash
# Full build
make all

# Clean build
make clean
make all

# Generate ROM images
make rom

# Disassemble sections
make dump
```

### 10.4 Output Files
- `bmbinary`: ELF executable
- `bmbinary.rom`: Raw binary for ROM programming
- `bmbinary.srec`: Motorola S-record format
- `build/*.o`: Object files
- `build/*.d`: Dependency files

---

## 11. Testing and Validation

### 11.1 Unit Testing
- Individual module testing (sensors, actuators)
- Timing validation (interrupt latency, execution time)
- Communication loopback tests

### 11.2 Integration Testing
- Full system initialization sequence
- Sensor-to-actuator pipeline
- Navigation accuracy over test course
- Mission-specific scenarios

### 11.3 Performance Testing
- CPU utilization measurement
- Memory usage verification
- Timing margin analysis
- Overrun detection validation

### 11.4 Field Testing
- Competition course simulation
- Multiple mission profiles
- Edge case scenarios (sensor failures, obstacles)
- Long-duration reliability tests

---

## 12. Known Limitations and Constraints

### 12.1 Hardware Constraints
- Limited processing power (16.78 MHz)
- Memory constraints (typical 68332: 2-4 KB RAM, 32-128 KB ROM)
- Fixed interrupt priorities
- TPU channel availability

### 12.2 Software Constraints
- No operating system (bare metal)
- Limited floating-point support (software emulation)
- Fixed-point arithmetic for performance
- Static memory allocation

### 12.3 Development Constraints
- Cross-compilation required
- No on-chip debugging (typical for this era)
- Limited profiling capabilities
- Manual memory management

---

## 13. Future Enhancements

### 13.1 Potential Improvements
- [ ] Implement PID control for more precise navigation
- [ ] Add sensor fusion (accelerometer + encoders)
- [ ] Implement SLAM for maze navigation
- [ ] Add wireless telemetry for real-time monitoring
- [ ] Optimize critical paths for better performance
- [ ] Add watchdog timer support
- [ ] Implement graceful error recovery

### 13.2 Code Modernization
- [ ] Refactor to C++ for better abstraction
- [ ] Add unit test framework
- [ ] Improve configuration management (runtime vs. compile-time)
- [ ] Document API with Doxygen
- [ ] Add simulation support for testing without hardware

### 13.3 Toolchain Updates
- [ ] Migrate to newer compiler versions
- [ ] Add static analysis tools (linting)
- [ ] Implement continuous integration
- [ ] Add automated testing infrastructure

---

## 14. Appendices

### 14.1 Register Maps
See hardware header files:
- `qsm_reg.h`: QSM registers
- `tpu_reg.h`: TPU registers
- `mem_332.h`: Memory map definitions

### 14.2 Compilation Flags
```
CFLAGS: -mcpu32 -Wall -Wextra -g -static -msoft-float -O
LFLAGS: --script=platform.ld
```

### 14.3 Memory Layout
Defined in `platform.ld` and `332.ld`:
- Interrupt vectors: 0x00000000
- ROM/Flash: Starting at configured base
- RAM: Configured per hardware
- Stack and heap: Defined in linker script

### 14.4 Glossary
- **CPU32**: Motorola's 32-bit embedded CPU architecture based on 68000
- **TPU**: Time Processing Unit - peripheral for timing and PWM
- **QSM**: Queued Serial Module - UART peripheral
- **FQD**: Frequency/Quadrature Decoder
- **Dead Reckoning**: Navigation using encoder-based position estimation
- **Robothon**: University-level robotics competition (1999 era)

### 14.5 References
- Motorola 68332 User Manual
- CPU32 Reference Manual
- Robothon Competition Rules (1999)
- m68k-elf-gcc Documentation

---

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | Feb 7, 2026 | Auto-generated | Initial PRD creation from Robothon1999OriginalUpdatedtoMacOS tag |

---

**Document Status**: Draft  
**Next Review Date**: TBD  
**Owner**: Mark Castelluccio
