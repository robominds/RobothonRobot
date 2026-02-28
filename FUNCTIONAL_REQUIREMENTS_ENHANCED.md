# RobothonRobot - Enhanced Functional Requirements

## Document Information
This document provides detailed technical specifications for each functional requirement in the Product Requirements Document, including implementation details, algorithms, data structures, acceptance criteria, error handling, and test procedures.

**Related Document**: PRD.md  
**Version**: 1.0  
**Date**: February 7, 2026

---

## Table of Contents
1. [Core System Requirements](#41-core-system-requirements)
2. [Navigation Requirements](#42-navigation-requirements)
3. [Sensor Processing Requirements](#43-sensor-processing-requirements)
4. [Actuator Control Requirements](#44-actuator-control-requirements)
5. [Guidance and Control Requirements](#45-guidance-and-control-requirements)
6. [Communication Requirements](#46-communication-requirements)

---

## 4.1 Core System Requirements

### FR-1: System Initialization

**Priority**: Critical  
**Module**: `rob.c::main()`  
**Dependencies**: All hardware header files

**Description**: Initialize all hardware peripherals, interrupt vectors, and software modules on system startup.

**Implementation Details**:
```c
// Interrupt vector table setup (0x00000000 - 0x000003FF)
for(j=2; j<256; j++) *(long*)(j*4) = (long)__vector_default;
*(long*)(4*4) = (long)illegal_instruction_int;  // Vector 4
*(long*)(64*4) = (long)hz100_int;                // Vector 64

// System clock configuration (SYNCR register)
*(word*)(0xfffa04) = (word)0x7d08;  // 16.78 MHz

// Module initialization sequence
tpu_init();         // Timer Processing Unit
qinit();            // Queued Serial Module  
sciinit();          // Serial communications
dist_init();        // Distance sensors
nav_init();         // Navigation system
guid_init();        // Guidance (if enabled)
mzguid_init();      // Maze guidance (if enabled)
cntr_init();        // Motor control
line_init();        // Line following (if enabled)
draw_init();        // Drawing mode (if enabled)

// Periodic timer setup (100 Hz, level 7)
*(word*)0xfffa24 = 82;    // Prescaler
*(word*)0xfffa24 = 1;     // Start
*(word*)0xfffa22 = 0x700+64;  // Priority + vector

// Enable interrupts (level 6 & 7)
asm("move.w #0x2500,%sr");
```

**Hardware Configuration**:
- **SYNCR (0xFFFA04)**: System clock synthesis register
  - Value 0x7D08 configures PLL for 16.78 MHz
  - Formula: FCLK = EXTAL × (Y+1) × 4 / (2^(X+1))
- **Periodic Timer (0xFFFA22-24)**: Level 7, autovector 64
  - Prescaler = 82 for 100 Hz from 16.78 MHz clock
  - Calculation: 16,780,000 / (82 × 2048) = 100.0 Hz

**Acceptance Criteria**:
1. All 256 vectors point to valid handlers (no null pointers)
2. Illegal instruction trap configured and testable
3. System clock stable at 16.78 MHz (±0.5%)
4. Each module initialization returns success (0)
5. TPU channels programmed correctly (verify via register read-back)
6. QSM baud rate configured (verify with loopback test)
7. FQD channels 11 & 13 reading encoder counts
8. 100 Hz interrupt firing (measure with oscilloscope or timer)
9. SR register = 0x2500 (interrupts enabled, supervisor mode)

**Error Conditions**:
- Hardware not responding: Timeout after 1 second
- Register mismatch: Read-back value ≠ written value
- Module init failure: Non-zero return code

**Test Procedure**:
1. Power on, verify LED blink or serial output
2. Check vector table with debugger
3. Trigger illegal instruction, verify trap
4. Measure clock frequency with scope
5. Verify each peripheral responds to writes
6. Check interrupt rate with timer/counter

---

### FR-2: Real-Time Control Loop

**Priority**: Critical  
**Module**: `rob.c::hz100_handler()`, `rob.c::hz100_int()`  
**Dependencies**: Navigation, sensors, actuators

**Description**: Maintain deterministic 100 Hz control loop with scheduled 10 Hz sub-tasks.

**Interrupt Service Routine**:
```assembly
// Assembly wrapper (rob.c)
hz100_int:
    link    %a6,#0
    moveml  %a6-%d0,-(%a7)    // Save all registers
    jsr     hz100_handler      // Call C handler
    moveml  (%a7)+,%d0-%a6    // Restore registers
    unlk    %a6
    rte                        // Return from exception
```

**Handler Implementation**:
```c
void hz100_handler(void) {
    // Overrun detection
    if(hz100_flag) {
        hz100_overrun++;
        return;
    }
    hz100_flag = 1;

    // Pre-start accelerometer processing
    if(!start) {
        // [Start detection code - see FR-3]
        hz100_flag = 0;
        return;
    }

    // Frame management
    frame = (frame+1) % 100;  // 0-99 counter
    if(frame % 10 == 0) hz10_inhibit = 0;

    // Critical 100 Hz task
    nav_update();  // MUST complete in <5 ms

    hz100_flag = 0;

    // Check for 10 Hz task availability
    if(hz10_inhibit) return;

    // A/D conversion (every frame during 10 Hz window)
    a2d_read();

    // 10 Hz task scheduling
    switch(frame % 10) {
        case 0:
            if(hz10_step) {
                hz10_overrun++;
                hz10_inhibit = 1;
                return;
            }
            hz10_step = 0x3ff;  // Set all 10 bits
            dist_start();
            break;
        case 1: case 3: case 5: case 7: case 9:
            cntr_update();
            break;
        case 4:
            if(mssnguid) guid_update();
            if(mssnmzguid) mzguid_update();
            if(mssndraw) draw_update();
            break;
        case 6:
            if(mssnmzguid) dist_read();
            break;
    }
    hz10_step &= ~(1 << (frame % 10));  // Clear bit
}
```

**Timing Analysis**:
| Task | Frequency | Frame(s) | Est. Time | Critical? |
|------|-----------|----------|-----------|-----------|
| nav_update | 100 Hz | All | 2-4 ms | Yes |
| a2d_read | 10 Hz | 0-9 | <500 µs | No |
| dist_start | 10 Hz | 0 | <100 µs | No |
| cntr_update | 50 Hz | 1,3,5,7,9 | 1-2 ms | Yes |
| guid_update | 10 Hz | 4 | <500 µs | No |
| dist_read | 10 Hz | 6 | <1 ms | No |

**Worst-case execution**: Frame 5 with nav_update + cntr_update ≈ 6 ms < 10 ms period (60% utilization)

**Acceptance Criteria**:
1. **Timing Precision**: Interrupt period = 10.00 ms ± 0.05 ms (measured over 1000 samples)
2. **Determinism**: Jitter < 50 µs between consecutive interrupts
3. **Overrun Handling**: `hz100_overrun` increments if ISR re-entered
4. **10 Hz Synchronization**: Sub-tasks execute every 10th frame (frames 0-9 repeating)
5. **Task Completion**: `hz10_step` bits clear indicating task finished
6. **Inhibit Logic**: New 10 Hz tasks blocked if previous overran
7. **Frame Counter**: Rolls over 0→99→0 correctly
8. **Navigation Updates**: Execute every single frame without exception

**Performance Metrics**:
- Average ISR time: <4 ms
- Maximum ISR time: <8 ms
- CPU utilization: 40-60%
- Overrun rate: <0.01% (1 per 10,000 interrupts)

**Error Handling**:
```c
// Overrun counters (global variables)
static int hz100_overrun = 0;  // Count of 100 Hz overruns
static int hz10_overrun = 0;   // Count of 10 Hz overruns
static int hz10_inhibit = 0;   // Flag to skip 10 Hz tasks

// Check in background loop
if(hz100_overrun > 0) {
    printf("100Hz overruns: %d\n", hz100_overrun);
}
```

**Test Procedure**:
1. Oscilloscope on interrupt pin: Verify 100 Hz square wave
2. Toggle GPIO in ISR: Measure pulse width (<8 ms)
3. Stress test: Add delays, verify overrun detection
4. Frame counter: Log values, confirm 0-99 pattern
5. Task execution: Set breakpoints, verify frame scheduling
6. Long-term: Run for 1 hour, check overrun counters

---

### FR-3: Start Detection

**Priority**: High  
**Module**: `rob.c::hz100_handler()` (pre-start section)  
**Dependencies**: A/D converter, periodic timer

**Description**: Detect robot movement via accelerometer with digital bandpass filter to reject noise and vibration.

**Algorithm**:
```c
// Digital biquad filter (2nd order IIR bandpass)
// Coefficients (tuned for 10-20 Hz signal at 100 Hz sample rate)
#define pa0 1314
#define pa1 1539  
#define pa2 680
#define pb0 1000
#define pb1 1539
#define pb2 994

// State variables
static int as=0, s=0, s1=0, s2=0;
static int start=0;

// Executed every 10 ms (100 Hz) before start
int ss = (int)*(unsigned char*)0x100000 - 0x80;  // Read sensor, remove bias

// Biquad filter (Direct Form I)
s  = s1 + ss * pa0;
s1 = s2 + ss * pa1 - (s * pb1) / pb0;
s2 = ss * pa2 - (s * pb2) / pb0;

// Accumulator with decay (100-sample moving average of |s|)
as += abs(s) - as/100;

// Threshold detection
if(as > 0x01800000) {  // ~25 million (calibrated threshold)
    start = 1;
    *(word*)0xfffa24 = 82;  // Reset timer
    // Exit pre-start loop, begin main control
}
```

**Filter Design**:
- **Type**: 2nd-order infinite impulse response (IIR) bandpass
- **Passband**: 10-20 Hz (robot acceleration frequency)
- **Stopband**: DC and >50 Hz (gravity offset and vibration noise)
- **Q-factor**: ~2 (moderate sharpness)
- **Gain**: Unity at center frequency

**Signal Flow**:
```
Accelerometer → Offset Removal → Biquad Filter → Absolute Value → Moving Average → Threshold
(0x00-0xFF)     (-128 to +127)   (±10,000)       (0 to 10,000)   (0 to 30M)      (>25M = start)
```

**Calibration Process**:
1. Place robot stationary: `as` should stabilize < 0x00500000
2. Apply 0.5g acceleration: `as` should exceed 0x01800000 within 200 ms
3. Apply vibration (engine, bumps): `as` should stay < 0x01000000
4. Adjust threshold if needed based on testing

**Acceptance Criteria**:
1. **Stationary Noise**: `as` < 0x00500000 (5 million) when robot at rest
2. **Response Time**: Start detected within 150-250 ms of motion onset
3. **Sensitivity**: Detects acceleration > 0.3g sustained for >100 ms
4. **False Rejection**: No false starts from typical vibration (<0.2g, >30 Hz)
5. **Bias Compensation**: Works with sensor offset ±50 counts
6. **Threshold Margin**: 2:1 ratio between start signal and noise floor
7. **State Transition**: `start` flag 0→1, timer resets, control loop starts

**Sensor Specifications**:
- **Address**: 0x100000 (memory-mapped 8-bit register)
- **Range**: 0-255 (0x00-0xFF)
- **Offset**: 128 (0x80) at 0g
- **Scale**: ~50 counts/g (typical for 8-bit ±2.5g sensor)
- **Noise**: ±2-3 counts RMS

**Test Procedure**:
1. **Static Test**: Robot on table, verify `as` stays low for 10 seconds
2. **Push Test**: Push robot gently, measure time to start detection
3. **Vibration Test**: Tap table, shake robot, verify no false starts
4. **Acceleration Sweep**: Apply 0.1g, 0.3g, 0.5g, 1.0g; measure response time
5. **Long-term**: 100 start cycles, verify consistency and no drift
6. **Edge Cases**: 
   - Very slow acceleration (0.1g over 1 second)
   - Sharp impulse (hammer tap)
   - Continuous vibration (motor running)

---

## 4.2 Navigation Requirements

### FR-4: Dead Reckoning Navigation

**Priority**: Critical  
**Module**: `nav.c::nav_update()`, `nav.c::nav_init()`  
**Dependencies**: FQD (Frequency/Quadrature Decoder), navtbl.h

**Description**: Estimate robot position and heading using differential drive kinematics with quadrature wheel encoders.

**Kinematic Model**:
```
Robot: Two-wheel differential drive
Encoders: Left wheel (FQD channel 13), Right wheel (FQD channel 11)
Position: (x, y) in millimeters
Heading: θ in milli-radians (0 to 2π×ipi, wraps at ±π)
```

**Update Algorithm**:
```c
// Constants from navtbl.h
#define lres 28824  // Left wheel: mm/count
#define rres 28772  // Right wheel: mm/count  
#define ipi 14655870  // π in milli-radians
#define angres 8142  // Angle resolution (mmrad/table entry)

void nav_update(void) {
    short int count[2];
    int delta[2];
    int angtemp;

    // 1. Read encoder counts (16-bit counters)
    count[0] = fqd_get_count(13);  // Left
    count[1] = fqd_get_count(11);  // Right

    // 2. Compute deltas with overflow handling
    delta[0] = count[0] - count_old[0];
    if(delta[0] >  32767) delta[0] -= 65536;  // Wrapped up
    if(delta[0] < -32768) delta[0] += 65536;  // Wrapped down
    
    delta[1] = count_old[1] - count[1];  // Note: Right inverted
    if(delta[1] >  32767) delta[1] -= 65536;
    if(delta[1] < -32768) delta[1] += 65536;

    // 3. Compute intermediate heading (for position update)
    angtemp = ang + (delta[0]*lres - delta[1]*rres) / 2;
    if(angtemp < 0) angtemp += 2*ipi;
    if(angtemp >= 2*ipi) angtemp -= 2*ipi;

    // 4. Update position (trapezoidal integration)
    pos[0] += (delta[0]*navcos(angtemp/angres, 0) + 
               delta[1]*navcos(angtemp/angres, 1)) / 2;
    pos[1] += (delta[0]*navsin(angtemp/angres, 0) + 
               delta[1]*navsin(angtemp/angres, 1)) / 2;

    // 5. Update heading with wrap-around
    ang += (delta[0]*lres - delta[1]*rres);
    if(ang < 0) {
        ang += 2*ipi;
        rotcnt -= 2;  // Track full rotations
    }
    if(ang >= 2*ipi) {
        ang -= 2*ipi;
        rotcnt += 2;
    }

    // 6. Save encoder values for next update
    count_old[0] = count[0];
    count_old[1] = count[1];

    // 7. Update velocity (low-pass filtered)
    vel += ((delta[0]+delta[1])*lres/2/1000 - vel - sign(vel)*9) / 10;
    
    // 8. Update angular rate (low-pass filtered)
    ang_rate += ((delta[0]*lres - delta[1]*rres) - ang_rate) / 3;

    // 9. Store raw counts for diagnostics
    n[0] = delta[0];
    n[1] = delta[1];
}
```

**Trigonometric Tables**:
- **Size**: 3600 entries (0.1° resolution)
- **Range**: 0 to 359.9°
- **Storage**: Two tables (left/right wheels with slight calibration offset)
- **Access**: `navcos(angle_index, wheel)` and `navsin(angle_index, wheel)`
- **Precision**: 16-bit integers (scale factor ~28,000 for unity)

**Coordinate System**:
```
Y (forward)
^
|
+---> X (right)

θ = 0: Robot facing +Y (forward)
θ = π/2: Robot facing +X (right)
θ = -π/2: Robot facing -X (left)
```

**Acceptance Criteria**:
1. **Position Accuracy**: ±2% over 10 meter straight line
2. **Heading Accuracy**: ±2° after 360° rotation
3. **Velocity Estimation**: Within 5% of true speed (50-500 mm/s range)
4. **Angular Rate**: Within 10% of true rotation rate
5. **Encoder Overflow**: Handles ±32K count changes per update
6. **Wrap-around**: Angle wraps correctly at ±π boundary
7. **Update Rate**: 100 Hz (every 10 ms)
8. **Dead Reckoning Drift**: <5% position error after 30 seconds

**Error Sources and Mitigation**:
| Error Source | Magnitude | Mitigation |
|--------------|-----------|------------|
| Wheel slippage | ±5% | None (inherent to DR) |
| Encoder quantization | ±0.5 mm/count | High-resolution encoders |
| Wheel diameter mismatch | ±1% | Calibrate lres/rres |
| Wheelbase error | ±2 mm | Calibrate geometry |
| Trig table quantization | ±0.01° | 3600-entry tables |

**Calibration Procedure**:
1. **Straight Line**: Drive 10m, measure actual distance, adjust `lres` and `rres`
2. **Rotation**: Rotate 360°, measure actual angle, adjust wheelbase
3. **Cross-calibration**: Drive square pattern, check closure error

**Test Procedure**:
1. **Straight Line Test**: 5m forward, verify position and heading
2. **Rotation Test**: 10 full rotations, check `rotcnt` and final heading
3. **Figure-8 Pattern**: Complex path, return to start, measure error
4. **Velocity Test**: Constant speed, compare encoder-based vs. measured
5. **Slalom Test**: Rapid direction changes, verify stability
6. **Long Duration**: 5 minutes continuous operation, track drift

---

### FR-5: Velocity and Angular Rate Estimation

**Priority**: High  
**Module**: `nav.c::nav_update()`, `nav.c::nav_get_vel()`, `nav.c::nav_get_ang_rate()`  
**Dependencies**: FR-4 (Navigation Update)

**Description**: Compute linear velocity and angular rate from encoder deltas using low-pass filters for noise rejection.

**Linear Velocity Estimation**:
```c
// Filter equation (1st-order IIR low-pass)
// Time constant ≈ 100 ms (10 samples at 100 Hz)
int raw_vel = (delta[0] + delta[1]) * lres / 2 / 1000;  // mm/s
int deadband = sign(vel) * 9;  // 9 mm/s deadband
vel += (raw_vel - vel - deadband) / 10;

// Retrieve
int nav_get_vel(void) {
    return vel;  // Units: mm/s
}
```

**Analysis**:
- **Raw Velocity**: Average of left and right wheel speeds
- **Units**: Millimeters per second (mm/s)
- **Calculation**: `(ΔL + ΔR) / 2 × resolution / dt`
  - `ΔL, ΔR`: Encoder counts per sample
  - `resolution`: 28,800 mm per count
  - `dt`: 10 ms (0.01 s) → factor of 1000 in denominator
- **Filter**: Exponential moving average with α = 0.1
  - Transfer function: H(z) = 0.1 / (1 - 0.9z⁻¹)
  - 3 dB cutoff: ~1.5 Hz
  - Settling time: ~230 ms (10 time constants)
- **Deadband**: ±9 mm/s to prevent drift at standstill
- **Range**: -1000 to +1000 mm/s typical (±1 m/s)

**Angular Rate Estimation**:
```c
// Filter equation (1st-order IIR low-pass)
// Time constant ≈ 30 ms (3 samples at 100 Hz)
int raw_ang_rate = (delta[0]*lres - delta[1]*rres);  // mmrad/s
ang_rate += (raw_ang_rate - ang_rate) / 3;

// Retrieve (with scaling)
int nav_get_ang_rate(void) {
    return 5 * ang_rate / angres;  // Units: degrees/s × 10
}
```

**Analysis**:
- **Raw Angular Rate**: Difference between wheel speeds
- **Units**: Milli-radians per second (internal), deci-degrees per second (output)
- **Calculation**: `(ΔL - ΔR) × resolution / dt`
- **Filter**: Exponential moving average with α = 0.33
  - 3 dB cutoff: ~5 Hz
  - Settling time: ~70 ms
- **Output Scaling**: Convert mmrad → deg/s × 10
  - Factor: 5 / angres = 5 / 8142 ≈ 0.000614
  - Example: 1000 mmrad/s → 6.14 deg/s → output 61
- **Range**: ±180°/s typical

**Acceptance Criteria**:
1. **Velocity Accuracy**: ±5% of true speed (50-500 mm/s)
2. **Velocity Settling**: Reaches 90% of step change in <250 ms
3. **Velocity Noise**: <10 mm/s RMS in steady state
4. **Deadband Function**: Output = 0 when input < ±9 mm/s
5. **Angular Rate Accuracy**: ±10% of true rate
6. **Angular Rate Settling**: Reaches 90% in <80 ms
7. **Angular Rate Noise**: <5 deg/s RMS
8. **Update Rate**: Both updated at 100 Hz

**Test Procedure**:
1. **Step Response**: Accelerate from 0 to 300 mm/s, measure settling time
2. **Steady State**: Constant 200 mm/s, measure noise and accuracy
3. **Deceleration**: Stop from 300 mm/s, verify deadband prevents drift
4. **Rotation**: Constant rotation rate, measure angular rate vs. gyro
5. **Slew Rate**: Maximum acceleration/deceleration, verify no overflow
6. **Zero Crossing**: Forward/reverse transitions, check sign handling

---

## 4.3 Sensor Processing Requirements

### FR-6: Distance Sensing

**Priority**: High  
**Module**: `dist.c::dist_start()`, `dist.c::dist_read()`, `dist.c::dist_get()`  
**Dependencies**: Port I/O (0xFFFA13, 0xFFFA15)

**Description**: Interface with ultrasonic or IR rangefinder sensors via parallel I/O with serial readout protocol.

**Hardware Interface**:
```
Port E (0xFFFA13): Data direction + read/write
Port F (0xFFFA15): Data direction config
Sensors: 3 channels (expandable to 8)
Protocol: Custom serial shift-out
```

**Initialization**:
```c
int dist_init(void) {
    *(char*)(0xfffa15) = 0x55;  // Port F: configure pins
    *(char*)(0xfffa13) = 0x55;  // Port E: set direction
    return 0;
}
```

**Measurement Trigger**:
```c
int dist_start(void) {
    // Check if previous measurement complete
    if((*(char*)(0xfffa13) & 0x2a)) {
        *(char*)(0xfffa13) = 0x00;  // Start new measurement
        return 1;
    }
    return 0;  // Busy
}
```

**Data Readout**:
```c
int dist_read(void) {
    int i, j;
    int range[8] = {0,0,0,0,0,0,0,0};

    // Check ready flag
    if((*(char*)(0xfffa13) & 0x2a)) {
        *(char*)(0xfffa13) = 0x15;  // Clock enable
        
        // Shift out 8 bits per channel
        for(i=0; i<8; i++) {
            *(char*)(0xfffa13) = 0x00;  // Clock low
            *(char*)(0xfffa13) = 0x15;  // Clock high
            
            // Read 3 channels in parallel
            range[0] = range[0]<<1 | ((*(char*)(0xfffa13) & 0x20) >> 5);
            range[1] = range[1]<<1 | ((*(char*)(0xfffa13) & 0x08) >> 3);
            range[2] = range[2]<<1 | ((*(char*)(0xfffa13) & 0x02) >> 1);
        }
        
        // Store results
        distance[0] = range[0];
        distance[1] = range[1];
        distance[2] = range[2];
    } else {
        // Not ready - mark invalid
        distance[0] = distance[1] = distance[2] = -1;
    }
    return 0;
}
```

**Timing**:
- **Trigger**: Frame 0 (every 100 ms at 10 Hz)
- **Measurement Time**: ~50-70 ms typical
- **Readout**: Frame 6 (60 ms after trigger)
- **Update Rate**: 10 Hz

**Data Format**:
- **Range**: 8-bit value (0-255)
- **Units**: Centimeters (typical for ultrasonic sensors)
- **Resolution**: 1 cm
- **Range**: 2-255 cm (sensor dependent)
- **Invalid**: -1 indicates sensor not ready or error

**Acceptance Criteria**:
1. **Measurement Cycle**: Trigger → Wait 50ms → Read → Valid data
2. **Range Accuracy**: ±2 cm from 10-200 cm
3. **Update Rate**: 10 Hz (stable)
4. **Multi-channel**: All 3 sensors update simultaneously
5. **Error Detection**: Returns -1 when sensor not ready
6. **Interface Timing**: Clock pulse width >1 µs
7. **Data Integrity**: No bit errors in serial readout

**Test Procedure**:
1. **Single Target**: Place object at 10, 50, 100, 200 cm; verify readings
2. **Multi-target**: Verify all 3 channels read independently
3. **Timing**: Oscilloscope on clock line, verify pulse widths
4. **Error Handling**: Block sensor, verify -1 return
5. **Repeatability**: 100 measurements at fixed distance, measure σ
6. **Dynamic**: Moving target, verify tracking

---

### FR-7: Line Detection

**Priority**: High  
**Module**: `line.c::line_init()`, `line.c::line_ang_err()`, `line.c::line_chan()`  
**Dependencies**: A/D converter (a2d.c), navigation

**Description**: Detect and track line position using 5-channel IR reflectance sensor array with differential measurement and error signal generation.

**Sensor Array Configuration**:
```
[0] [1] [2] [3] [4]
 L  ML   C  MR   R     (Left, Mid-Left, Center, Mid-Right, Right)
<----  50 mm  ---->    (typical spacing)
```

**Initialization**:
```c
int line_init(void) {
    int i;

    // Capture baseline (white background) over 20 samples
    for(i=0; i<20; i++) a2d_read();
    
    // Store reference for each channel
    for(i=0; i<5; i++) 
        lineref[i] = a2d_chan(i+2);  // Channels 2-6

    // Initialize state
    angerr = 0;
    velcmd = 0;
    linedone = 0;
    countdown = 25;  // Frames after line end
    headingcmd = -1;  // Not set

    return 0;
}
```

**Channel Reading**:
```c
int line_chan(int chan) {
    // Differential measurement (removes ambient light)
    return a2d_chan(chan+2) - lineref[chan];
}
```

**Line Position Algorithm**:
```c
int line_ang_err(void) {
    int linechan[5];
    int linemax = 0x08;  // Minimum detection threshold
    int index = -1;      // Peak sensor index
    int ldone = 1;

    // 1. Read all channels (clamp negative to 0)
    for(i=0; i<5; i++) linechan[i] = max(0, line_chan(i));

    // 2. Find peak sensor
    for(i=0; i<5; i++) {
        if(linechan[i] < 0x08) ldone = 0;  // Still on line
        if(linechan[i] > linemax) {
            linemax = linechan[i];
            index = i;
        }
    }
    
    if(ldone) linedone = 1;  // All sensors off line

    // 3. Compute angular error based on peak position
    switch(index) {
        case 0:  // Far left
            angerr = -200 + (100*linechan[1]) / (linechan[0]+linechan[1]);
            break;
        case 1:  // Mid-left
            angerr = -100 + (50*(linechan[2]-linechan[0])) / linechan[1];
            break;
        case 2:  // Center
            angerr = (50*(linechan[3]-linechan[1])) / linechan[2];
            break;
        case 3:  // Mid-right
            angerr = 100 + (50*(linechan[4]-linechan[2])) / linechan[3];
            break;
        case 4:  // Far right
            angerr = 200 - (100*linechan[3]) / (linechan[3]+linechan[4]);
            break;
        default:  // No line detected
            angerr = sign(angerr) * 200;  // Max error, maintain direction
            break;
    }

    // 4. Set velocity command
    velcmd = 400;  // mm/s

    // 5. Handle end-of-line condition
    if(linedone) {
        if(headingcmd < 0) 
            headingcmd = nav_get_ang();  // Capture heading
        return nav_ang_err(headingcmd, nav_get_ang());
    }

    return angerr / 2;  // Scale down for control
}
```

**Error Signal Mapping**:
```
Line Position    Sensor[peak]    Error (deg × 10)
Far Left         [0]             -200 to -100
Mid-Left         [1]             -100 to -50
Center           [2]             -50 to +50
Mid-Right        [3]             +50 to +100
Far Right        [4]             +100 to +200
Off Line         none            ±200 (max)
```

**Interpolation Method**:
- Uses adjacent sensor ratios for sub-sensor resolution
- Example (index=1): Error interpolates between -100 and -50 based on relative brightness of sensors 0, 1, 2
- Provides ~5mm position resolution with 10mm sensor spacing

**Acceptance Criteria**:
1. **Baseline Calibration**: Reference values stored correctly
2. **Line Detection**: Triggers when any channel > threshold (0x08)
3. **Position Accuracy**: ±5 mm from true line position
4. **Error Range**: -200 to +200 (±20°)
5. **End Detection**: `linedone` flag set when all channels < threshold
6. **Velocity Command**: 400 mm/s during tracking
7. **Post-line Behavior**: Maintains last heading for 25 frames (250 ms)
8. **Update Rate**: 100 Hz (error computed every frame)

**Test Procedure**:
1. **Calibration**: White surface, verify references in range 100-200
2. **Center Line**: Place line under center sensor, verify error ≈ 0
3. **Offset Test**: Shift line ±10mm, verify error changes correctly
4. **Dynamic**: Follow curved line, measure tracking accuracy
5. **End Condition**: Drive off line, verify heading hold
6. **Noise**: Add lighting variation, verify stable tracking

---

### FR-8: Flame Detection

**Priority**: Medium  
**Module**: `flame.c::flame_mag()`, `flame.c::fan_on()`, `flame.c::fan_off()`  
**Dependencies**: A/D converter, digital I/O

**Description**: Detect fire source using IR flame sensor and provide flame magnitude reading.

**Implementation**: 
[Note: Full implementation not provided in available source files, but interface defined in flame.h]

```c
int flame_init(void);        // Initialize flame sensor
int flame_mag(void);         // Read flame intensity (0-255)
int fan_on(void);           // Activate extinguisher fan
int fan_off(void);          // Deactivate extinguisher fan
```

**Expected Behavior** (based on guid.c usage):
- **Magnitude**: 8-bit value (0x00-0xFF)
  - 0x00-0x03: No flame detected
  - 0x04-0x4B: Flame detected, approaching
  - 0x4C+: Close proximity, start extinguishing
- **Detection Range**: 30-100 cm typical for IR flame sensors
- **Update Rate**: Read via A/D at 10 Hz
- **Fan Control**: Digital output, PWM possible

**Fire-fighting Sequence** (from guid.c):
```c
if(abs(flame_mag()) > 0x04) {
    // Flame detected - enter scan mode
    scanmode = 10;
    flamemag = abs(flame_mag());
    flameang = nav_get_ang();  // Record direction
}

if(abs(flame_mag()) > 0x4C) {
    // Close enough - start extinguishing
    scanmode = 11;
}

if(scanmode == 11) {
    if(tmr < 100) dio_update(0,1);  // Fan on for 1 second
    if(tmr > 100) dio_update(0,0);  // Fan off
    // Verify extinguished
}
```

**Acceptance Criteria**:
1. **Detection Threshold**: Reliably detects flame at >30 cm
2. **Magnitude Accuracy**: Within 10% of true intensity
3. **Directionality**: Used with heading to locate flame
4. **Fan Control**: Turns on/off on command
5. **Extinguish Verification**: Flame magnitude drops after fan activation
6. **Update Rate**: 10 Hz minimum

---

### FR-9: Analog-to-Digital Conversion

**Priority**: High  
**Module**: `a2d.c::a2d_read()`, `a2d.c::a2d_chan()`, `a2d.c::a2d_chan_f()`  
**Dependencies**: Memory-mapped A/D hardware

**Description**: Convert analog sensor signals (0-5V) to digital values (0-255) with software filtering.

**Hardware Interface**:
```c
// A/D registers (memory-mapped)
Base address: 0x100000
Channels: 8 (addresses 0x100000-0x100007)
Resolution: 8-bit (0-255)
Reference: 5.0V → 1 LSB ≈ 20 mV
```

**Read Function**:
```c
static int a2d_chans[8];    // Raw readings
static int a2d_chans_f[8];  // Filtered readings

int a2d_read(void) {
    int i;

    // Read all 8 channels (addresses wrap at 8)
    for(i=1; i<9; i++) {
        a2d_chans[i-1] = (int)*(unsigned char*)(0x100000 + (i%8));
        
        // 1st-order IIR low-pass filter with deadband
        int error = a2d_chans[i-1] - a2d_chans_f[i-1];
        int deadband = sign(error) * 3;
        a2d_chans_f[i-1] += (error + deadband) / 4;
    }
    return 0;
}

// Retrieve raw reading
int a2d_chan(int chan) {
    return a2d_chans[chan];
}

// Retrieve filtered reading
int a2d_chan_f(int chan) {
    return a2d_chans_f[chan];
}
```

**Channel Assignments**:
| Channel | Sensor | Usage |
|---------|--------|-------|
| 0 | Accelerometer | Start detection |
| 1 | Battery voltage | Power monitoring |
| 2-6 | Line sensors | Line following (5 channels) |
| 7 | Flame sensor | Fire detection |

**Filter Characteristics**:
- **Type**: 1st-order IIR low-pass
- **Equation**: y[n] = y[n-1] + (x[n] - y[n-1] + deadband) / 4
- **Alpha**: 0.25 (averaging factor)
- **Cutoff**: ~4 Hz at 10 Hz sample rate
- **Deadband**: ±3 counts (prevents oscillation)
- **Settling**: ~40 ms (4 time constants)

**Acceptance Criteria**:
1. **Conversion Accuracy**: ±1 LSB (±20 mV)
2. **Channel Independence**: All 8 channels read correctly
3. **Update Rate**: 10 Hz (called from 100 Hz ISR on frame transitions)
4. **Filter Stability**: No oscillation in steady state
5. **Noise Rejection**: <2 counts RMS after filtering
6. **Deadband Function**: Prevents ±3 count jitter
7. **Settling Time**: <50 ms to 90% of step change

**Calibration**:
1. **Zero**: Ground input, verify reading 0-2
2. **Full Scale**: 5V input, verify reading 253-255
3. **Linearity**: Apply 1V, 2V, 3V, 4V; verify 51, 102, 153, 204 ±2
4. **Noise**: Measure with scope, verify <50 mV pk-pk

**Test Procedure**:
1. **Static Test**: Apply known voltages, verify readings
2. **Dynamic Test**: Sweep 0-5V at 1 Hz, plot response
3. **Filter Test**: Step input, measure settling time
4. **Noise Test**: Short to ground, measure variance
5. **Multi-channel**: Vary all inputs, verify no crosstalk

---

## 4.4 Actuator Control Requirements

### FR-10: PWM Motor Control

**Priority**: Critical  
**Module**: `pwm.c::pwm_init()`, `pwm.c::pwm_update()`, `cntr.c::cntr_update()`  
**Dependencies**: TPU (Timer Processing Unit)

**Description**: Generate PWM signals for motor speed and direction control using TPU hardware PWM channels.

**TPU Channel Configuration**:
```c
// TPU channels
Channel 3: Left motor PWM
Channel 5: Right motor PWM
Channels 4,6: Direction control (DIO)

// PWM frequency
#define PWMFRQ 240  // Base count (determines frequency)
Actual frequency = TPU_CLOCK / (2 * PWMFRQ * prescaler)
```

**Initialization**:
```c
int pwm_init(int chan, int on, int off) {
    tpu_set_cpr(chan, 0);      // Clear priority
    tpu_set_cfsr(chan, 14);    // Function 14: PWM

    // Configure PWM parameters in TPU RAM
    (*(short int*)(TPU_RAM + chan*16 + 0)) = chan*16 + 3*2 + 0;  // Link
    (*(short int*)(TPU_RAM + chan*16 + 2)) = 0;                  // Reserved
    (*(short int*)(TPU_RAM + chan*16 + 4)) = off*2 + 0;          // Low time
    (*(short int*)(TPU_RAM + chan*16 + 6)) = on*2 + 1;           // High time

    tpu_set_hsqr(chan, 2);     // Host sequence: initialize
    tpu_set_hsrr(chan, 2);     // Host service request
    tpu_set_cpr(chan, 3);      // Set priority (high)

    return 0;
}
```

**Update Function**:
```c
int pwm_update(int chan, int on, int off) {
    on  = max(0, on);
    off = max(0, off);

    // Write to TPU parameter RAM
    (*(short int*)(TPU_RAM + chan*16 + 4)) = off*2 + 0;
    (*(short int*)(TPU_RAM + chan*16 + 6)) = on*2 + 1;

    return 0;
}
```

**Motor Control Logic** (cntr.c):
```c
int cntr_update(void) {
    int pwmpos;   // Forward/backward component
    int pwmang;   // Left/right steering component

    // [Compute pwmpos and pwmang from guidance/navigation]
    // ...

    // Differential steering
    pwmleft  = pwmpos - (pwmang + sign(pwmang)*60);
    pwmright = pwmpos + pwmang;

    // Limit based on current motion
    pwmleftlim  = 120 - sign(pwmleft)*15*nav_get_n(0);
    pwmrightlim = 120 - sign(pwmright)*15*nav_get_n(1);
    
    if((abs(pwmleft) > pwmleftlim) && (pwmleft*nav_get_n(0) > 0))
        pwmleft = sign(pwmleft) * pwmleftlim;
    
    if((abs(pwmright) > pwmrightlim) && (pwmright*nav_get_n(1) > 0))
        pwmright = sign(pwmright) * pwmrightlim;

    // Update PWM (duty cycle = on / (on + off))
    pwm_update(3, PWMFRQ-pwmleft,  PWMFRQ+pwmleft);   // Left
    pwm_update(5, PWMFRQ+pwmright, PWMFRQ-pwmright);  // Right

    return 0;
}
```

**PWM Signal Details**:
- **Frequency**: ~17-35 kHz (above audible, depends on TPU clock)
- **Duty Cycle Range**: 0-100%
  - 0: Motor off
  - 50%: Medium speed
  - 100%: Full speed
- **Resolution**: ~0.4% (240 discrete levels)
- **Update Rate**: 50 Hz (every other 100 Hz frame)

**Direction Control**:
```c
// Digital I/O for H-bridge direction
dio_init(4, 1);  // Left motor direction
dio_init(6, 1);  // Right motor direction

// Update direction based on PWM sign
// (Implicit in pwm_update with signed on/off values)
```

**Acceptance Criteria**:
1. **Frequency**: 17-35 kHz (verify with oscilloscope)
2. **Duty Cycle**: Linear from 0-100% across full command range
3. **Resolution**: <1% duty cycle steps
4. **Update Rate**: Commands take effect within 20 ms
5. **Differential Drive**: Left/right independent control
6. **Direction Switching**: <10 ms transition time
7. **Dead Zone**: Motors off when |command| < threshold
8. **Anti-saturation**: Limits prevent >100% duty cycle

**Test Procedure**:
1. **Frequency Measurement**: Scope on PWM output, verify >17 kHz
2. **Duty Cycle Sweep**: Command 0, 25, 50, 75, 100%; measure actual
3. **Direction Test**: Forward, reverse, verify H-bridge states
4. **Dynamic Response**: Step command, measure motor response time
5. **Differential**: Command pure rotation, verify opposite wheel speeds
6. **Stability**: Run at 50% for 10 seconds, verify no glitches

---

### FR-11: Encoder Interface

**Priority**: Critical  
**Module**: `fqd.c`, `nav.c`  
**Dependencies**: TPU FQD (Frequency/Quadrature Decoder) function

**Description**: Read quadrature encoder signals for wheel position measurement using TPU hardware decoder.

**Hardware Configuration**:
```
Left Wheel: FQD Channel 13 (TPU channel 13)
Right Wheel: FQD Channel 11 (TPU channel 11)

Encoder Signals:
- Channel A: Quadrature phase A
- Channel B: Quadrature phase B (90° offset)
- Optional Index: Once per revolution (not used)

Count Mode: X4 (all edges) for maximum resolution
```

**Initialization**:
```c
int fqd_init(int chan) {
    // Configure TPU channel for FQD function
    tpu_set_cpr(chan, 0);
    tpu_set_cfsr(chan, 6);  // Function 6: FQD

    // Initialize counter to 0
    // [TPU parameter RAM configuration]
    
    tpu_set_hsqr(chan, 2);
    tpu_set_hsrr(chan, 2);
    tpu_set_cpr(chan, 2);  // Medium priority

    return 0;
}
```

**Reading Counts**:
```c
short int fqd_get_count(int chan) {
    // Read 16-bit counter from TPU RAM
    return *(short int*)(TPU_RAM + chan*16 + offset);
}
```

**Quadrature Decoding**:
```
State Machine (X4 mode):
AB: 00 → 01 → 11 → 10 → 00  (forward, count up)
AB: 00 → 10 → 11 → 01 → 00  (reverse, count down)

Each edge of A or B increments/decrements counter:
- 4 counts per encoder cycle
- Typical encoder: 512 cycles/rev
- Total: 2048 counts/rev
```

**Encoder Specifications**:
- **Type**: Incremental quadrature
- **Resolution**: 512 CPR (cycles per revolution) typical
- **X4 Decoding**: 2048 counts/rev
- **Wheel Circumference**: ~360 mm (114 mm diameter)
- **Linear Resolution**: 360mm / 2048 = 0.176 mm/count
- **Maximum Speed**: 10,000 counts/sec (>3 m/s)

**Overflow Handling**:
```c
// nav.c handles 16-bit overflow
delta[0] = count[0] - count_old[0];
if(delta[0] >  32767) delta[0] -= 65536;
if(delta[0] < -32768) delta[0] += 65536;
```

**Acceptance Criteria**:
1. **Direction Sensing**: Correctly identifies forward/reverse
2. **Edge Detection**: All 4 edges per cycle detected
3. **Count Accuracy**: ±1 count over 10 revolutions
4. **Maximum Rate**: Handles 10,000 counts/sec without loss
5. **Overflow**: 16-bit counter wraps correctly
6. **Noise Immunity**: Rejects <100 ns glitches
7. **Startup**: Initializes to 0 or known value
8. **Both Channels**: Left and right operate independently

**Calibration**:
```c
// From navtbl.h
#define lres 28824  // Left: mm × 1000 / count
#define rres 28772  // Right: mm × 1000 / count

// Calibration process:
// 1. Measure wheel circumference: C mm
// 2. Count encoder pulses per revolution: N
// 3. Resolution = C × 1000 / N
```

**Test Procedure**:
1. **Static**: Rotate wheel slowly by hand, verify counts
2. **Direction**: Forward/reverse, verify sign
3. **Speed**: Drive at maximum speed, verify no missed counts
4. **Overflow**: Drive >32K counts, verify delta calculation
5. **Long-term**: 10 minutes of operation, check drift
6. **Calibration**: Drive 10 meters, measure actual vs. computed

---

## 4.5 Guidance and Control Requirements

### FR-12: Waypoint Navigation

**Priority**: High  
**Module**: `guid.c::guid_init()`, `guid.c::guid_update()`, `guid.c::guid_get_headingcmd()`, `guid.c::guid_get_velcmd()`  
**Dependencies**: Navigation (nav.c), waypoint tables (waypoint.h, fireplan.h)

**Description**: Navigate to predefined waypoints using path planning and control law generation.

**Data Structures**:
```c
// Waypoint definition
struct way_points_str {
    int entry;      // Index
    int cord[2];    // (x, y) in mm
};

// Path plan step
struct way_plan_str {
    int entry;      // Index
    int type;       // 1=transit, 2=scan, 3=extended scan
    int way_point;  // Target waypoint index
    int next;       // Next step in forward path
    int next_ret;   // Next step in return path
    int tol;        // Arrival tolerance (mm)
    int wall;       // Wall following flag
};
```

**Example Mission** (fireplan.h):
```c
// Waypoints (positions in mm)
way_points[] = {
    {0, {0, 0}},          // Start
    {1, {0, 41500}},      // North
    {2, {28000, 41500}},  // NE corner
    // ...
};

// Path plan
way_plan[] = {
    {0, 1, 0, 1, -1, 6000, 0},  // Start→WP1, tol=60mm
    {1, 1, 1, 2, 24, 6000, 0},  // WP1→WP2, return to step 24
    {2, 1, 2, 3,  1, 6000, 0},  // WP2→WP3, return to step 1
    {3, 2, 3, 4,  2, 3000, 0},  // WP3: Scan mode
    // ...
};
```

**Guidance Update**:
```c
int guid_update(void) {
    // Check arrival condition
    if(abs(target_cord[0] - nav_get_pos(0)) < way_plan[next_step].tol &&
       abs(target_cord[1] - nav_get_pos(1)) < way_plan[next_step].tol) {
        
        // Advance to next waypoint
        step = next_step;
        next_step = way_plan[step].next;
        
        // Update target coordinates
        target_way_point = way_plan[next_step].way_point;
        target_cord[0] = way_points[target_way_point].cord[0];
        target_cord[1] = way_points[target_way_point].cord[1];
    }
    
    return 0;
}
```

**Velocity Command**:
```c
int guid_get_velcmd(void) {
    int dx = target_cord[0] - nav_get_pos(0);
    int dy = target_cord[1] - nav_get_pos(1);
    int dist = max(abs(dx), abs(dy));
    
    // Proportional control with min/max
    int velcmd = dist / 40;              // P-gain = 1/40
    velcmd = min(300, velcmd) + 50;     // 50-350 mm/s
    
    return velcmd;
}
```

**Heading Command**:
```c
int guid_get_headingcmd(void) {
    int dx, dy;
    int heading0, heading, headingcmd;

    // Desired heading (origin → target)
    dx = target_cord[0] - origin_cord[0];
    dy = target_cord[1] - origin_cord[1];
    heading0 = (900*dy) / (abs(dx) + abs(dy) + 1);
    if(dx < 0) heading0 = 1800*sign(dy) - heading0;

    // Current heading (position → target)
    dx = target_cord[0] - nav_get_pos(0);
    dy = target_cord[1] - nav_get_pos(1);
    heading = (900*dy) / (abs(dx) + abs(dy) + 1);
    if(dx < 0) heading = 1800*sign(dy) - heading;

    // Blend with proportional correction
    headingcmd = heading0 - 3*nav_ang_err(heading0, heading) / 2;

    // Wrap to ±180°
    if(headingcmd >  1800) headingcmd -= 3600;
    if(headingcmd < -1800) headingcmd += 3600;

    return headingcmd;
}
```

**Angle Calculation** (arctangent approximation):
```c
// Fast atan2 using division and conditionals
// heading = (900 * dy) / (|dx| + |dy| + 1)
// Range: -900 to +900 (±45° in tenths of degrees)
// Corrected based on quadrant: if dx<0, heading = 1800*sign(dy) - heading
```

**Acceptance Criteria**:
1. **Waypoint Arrival**: Detects within tolerance (typ. 30-60 mm)
2. **Path Following**: Follows straight-line segments between waypoints
3. **Heading Accuracy**: ±5° during transit
4. **Velocity Profile**: Slows as approaching target (proportional)
5. **Sequencing**: Correctly advances through waypoint list
6. **Return Paths**: Follows alternate path when specified
7. **Completion**: Sets `way_point_done` flag when list exhausted
8. **Scan Mode**: Executes type=2,3 behaviors (fire search)

**Test Procedure**:
1. **Single Waypoint**: Navigate to (1000, 1000), verify arrival
2. **Multi-waypoint**: Follow 5-point path, measure deviations
3. **Square Pattern**: 4 corners, return to start, check closure error
4. **Return Path**: Execute mission, verify return follows different route
5. **Tolerance**: Vary tolerance, verify earlier/later arrival detection
6. **Scan Modes**: Trigger type=2,3 steps, verify special behaviors

---

### FR-13: Maze Navigation

**Priority**: Medium  
**Module**: `mzguid.c` (referenced but not provided in full)  
**Dependencies**: Distance sensors (dist.c), navigation

**Description**: Navigate through maze using wall-following or mapping algorithm with distance sensor feedback.

**Expected Functionality** (inferred from references):
- Wall detection using distance sensors
- Decision-making at intersections
- Path planning or reactive behavior
- Goal-seeking orientation

**Control Interface**:
```c
int mzguid_init(void);
int mzguid_update(void);  
int mzguid_get_headingcmd(void);
int mzguid_get_velcmd(void);
```

**Typical Maze Algorithm** (wall-following):
```
1. Maintain constant distance from right wall (or left wall)
2. At intersection:
   - Right wall ends: Turn right
   - Straight wall: Continue straight
   - T-junction: Turn right/left based on rule
   - Dead end: 180° turn
3. Velocity: Slow near walls, faster in open areas
```

**Acceptance Criteria**:
1. **Wall Following**: Maintains 100-200 mm from wall
2. **Intersection Detection**: Identifies T, L, + junctions
3. **Decision Making**: Consistent rule application
4. **Goal Finding**: Reaches exit/target within time limit
5. **Collision Avoidance**: No wall impacts
6. **Velocity Adaptation**: Slows for turns, speeds on straight

**Test Procedure**:
1. **Simple Corridor**: Follow straight wall
2. **90° Turn**: Navigate L-junction
3. **T-Junction**: Verify turn decision
4. **Dead End**: Execute 180° reversal
5. **Full Maze**: Solve standard maze configuration
6. **Repeatability**: Same path on repeated runs

---

### FR-14: Drawing/Mapping Mode

**Priority**: Low  
**Module**: `draw.c` (referenced but not fully detailed)  
**Dependencies**: Navigation, guidance

**Description**: Create a map or execute a pre-programmed drawing pattern based on position tracking.

**Control Interface**:
```c
int draw_init(void);
int draw_update(void);
int draw_get_headingcmd(void);
int draw_get_velcmd(void);
```

**Possible Implementations**:
1. **Pattern Drawing**: Follow predefined path (circle, square, etc.)
2. **Sensor Mapping**: Record sensor readings at positions
3. **Trajectory Recording**: Log path taken for playback

**Acceptance Criteria**:
1. **Pattern Accuracy**: Drawn shape within 5% of ideal
2. **Position Recording**: Logs coordinates at specified intervals
3. **Velocity Control**: Smooth, constant speed execution
4. **Heading Control**: Follows calculated path

**Test Procedure**:
1. **Circle**: Draw 1m diameter circle, measure roundness
2. **Square**: Draw 1m square, check corner angles
3. **Figure-8**: Complex pattern, verify crossover point
4. **Map Recording**: Drive random path, verify log completeness

---

## 4.6 Communication Requirements

### FR-15: Serial Communication

**Priority**: Medium  
**Module**: `scibuff.c`, `scibuff.s`, `stdio.c`  
**Dependencies**: QSM (Queued Serial Module)

**Description**: Provide buffered, interrupt-driven serial I/O for debugging and status reporting.

**Buffering**:
```c
// Circular buffers (assembly implementation)
#define BUFFER_SIZE 256  // Typical
uint8_t tx_buffer[BUFFER_SIZE];
uint8_t rx_buffer[BUFFER_SIZE];
uint16_t tx_head, tx_tail;
uint16_t rx_head, rx_tail;
```

**Transmit**:
```c
int tx_byte(char c) {
    // Check if buffer full
    if((tx_head + 1) % BUFFER_SIZE == tx_tail)
        return 0;  // Buffer full
    
    // Add to buffer
    tx_buffer[tx_head] = c;
    tx_head = (tx_head + 1) % BUFFER_SIZE;
    
    // Enable TX interrupt
    // [QSM register configuration]
    
    return 1;  // Success
}
```

**Receive**:
```c
int rx_byte(char *c) {
    // Check if buffer empty
    if(rx_head == rx_tail)
        return 0;  // No data
    
    // Read from buffer
    *c = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % BUFFER_SIZE;
    
    return 1;  // Success
}
```

**Printf Support**:
```c
int printf(const char *format, ...) {
    char buffer[128];
    va_list args;
    
    va_start(args, format);
    vsprintf(buffer, format, args);  // Format string
    va_end(args);
    
    // Transmit buffer
    for(int i=0; buffer[i]; i++) {
        while(!tx_byte(buffer[i]));  // Block if full
    }
    
    return 0;
}
```

**Acceptance Criteria**:
1. **Baud Rate**: 9600, 19200, or 115200 bps
2. **Format**: 8-N-1 (8 data, no parity, 1 stop)
3. **Buffering**: 256 bytes TX, 256 bytes RX
4. **Interrupt-driven**: Does not block control loop
5. **Printf**: Supports %d, %x, %s, %c format specifiers
6. **Echo**: Can echo received characters
7. **Flow Control**: Software (none) or hardware (RTS/CTS)

**Test Procedure**:
1. **Loopback**: Connect TX to RX, verify echo
2. **Baud Rate**: Test with serial terminal at various rates
3. **Throughput**: Measure maximum sustained data rate
4. **Overflow**: Fill buffer, verify graceful handling
5. **Printf**: Test all format specifiers
6. **Long Message**: Send >256 byte string, verify buffering

---

### FR-16: Status Reporting

**Priority**: Medium  
**Module**: `rob.c::main()` background loop  
**Dependencies**: Serial I/O, sensors, navigation

**Description**: Output real-time status information for monitoring and debugging.

**Status Message Format**:
```c
// Background loop (rob.c)
while(1) {
    // Line sensor array
    printf("%x %x %x %x %x ",
        line_chan(0), line_chan(1), line_chan(2),
        line_chan(3), line_chan(4));
    
    // Navigation state
    printf("%x %x %x ",
        nav_get_ang(),      // Heading
        nav_get_pos(0),     // X position
        nav_get_pos(1));    // Y position
    
    printf("\n\r");
    
    // Echo any received characters
    if(rx_byte(&rxchar)) {
        while(!tx_byte(rxchar));
        if(rxchar == '\r') while(!tx_byte('\n'));
    }
}
```

**Output Rate**:
- Limited by printf execution time (~10-50 ms)
- Effective rate: ~10-20 Hz
- Does not interfere with 100 Hz control loop

**Acceptance Criteria**:
1. **Format**: Hexadecimal values, space-separated
2. **Rate**: 10-20 Hz update (non-critical)
3. **Completeness**: All key state variables included
4. **Readability**: Terminal-compatible (CR+LF line endings)
5. **Echo**: User input echoed back
6. **Non-blocking**: Does not delay control loop

**Example Output**:
```
12 34 56 78 9A 0 1234 5678
15 38 5A 7C 9E 5 1240 5680
...
```

**Test Procedure**:
1. **Connection**: Verify output visible in terminal
2. **Update Rate**: Measure lines per second
3. **Accuracy**: Compare printed values to actual state
4. **Echo**: Type characters, verify echo
5. **Long-term**: Run for 10 minutes, verify no corruption

---

## Appendix A: Control System Block Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                      100 Hz INTERRUPT                       │
│  ┌────────────┐    ┌──────────────┐    ┌────────────────┐  │
│  │ Encoders   │───>│ Navigation   │───>│ State          │  │
│  │ (FQD)      │    │ (nav_update) │    │ Estimator      │  │
│  └────────────┘    └──────────────┘    └────────────────┘  │
│                            │                                │
│                            v                                │
│  ┌────────────┐    ┌──────────────┐    ┌────────────────┐  │
│  │ Line/Dist  │───>│ Guidance     │───>│ Commands       │  │
│  │ Sensors    │    │ (guid_update)│    │ (vel, heading) │  │
│  └────────────┘    └──────────────┘    └────────────────┘  │
│                            │                                │
│                            v                                │
│  ┌────────────┐    ┌──────────────┐    ┌────────────────┐  │
│  │ Control    │───>│ Motor        │───>│ PWM            │  │
│  │ Law        │    │ Commands     │    │ Output         │  │
│  └────────────┘    └──────────────┘    └────────────────┘  │
└─────────────────────────────────────────────────────────────┘
           │                                         │
           v                                         v
     [Navigation                               [Physical
      Feedback]                                 Robot]
```

## Appendix B: Memory Map

```
0x00000000 - 0x000003FF: Interrupt Vector Table (256 × 4 bytes)
0x00000400 - 0x0000FFFF: ROM/Flash (code + const data)
0x00100000 - 0x00100007: A/D Converter (8 channels)
0x00800000 - 0x00803FFF: RAM (16 KB typical)
0xFFFA00   - 0xFFFA23:   System Registers
0xFFFA24   - 0xFFFA25:   Periodic Timer Control
0xFFFC00   - 0xFFFCFF:   TPU Registers
0xFFFD00   - 0xFFFDFF:   TPU Parameter RAM
0xFFFE00   - 0xFFFFFF:   QSM Registers
```

## Appendix C: Timing Budget

| Task | Frequency | Execution Time | % CPU |
|------|-----------|----------------|-------|
| Interrupt Overhead | 100 Hz | 100 µs | 1% |
| nav_update | 100 Hz | 3 ms | 30% |
| cntr_update | 50 Hz | 1.5 ms | 7.5% |
| guid_update | 10 Hz | 500 µs | 0.5% |
| a2d_read | 10 Hz | 200 µs | 0.2% |
| dist_start/read | 10 Hz | 1 ms | 1% |
| Background (printf) | ~20 Hz | 10 ms | ~20% |
| **Total** | | | **~60%** |
| **Margin** | | | **40%** |

---

**End of Enhanced Functional Requirements Document**
