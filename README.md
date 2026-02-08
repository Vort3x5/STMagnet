# STMagnet

A digital magnetometer system using STM32F446RE microcontroller, QMC5883P 3-axis magnetic sensor, and SSD1306 OLED display for real-time magnetic field measurements.

## Features

- **3-axis magnetic field measurement** (X, Y, Z components)
- **Real-time OLED display** (128×64 SSD1306)
- **Hard-iron and soft-iron calibration** for accurate readings
- **Digital low-pass filtering** (Exponential Moving Average)
- **UART output** for debugging and data logging
- **Magnitude calculation** of total magnetic field strength
- Detects Earth's magnetic field (~50 µT) and nearby magnets

## Hardware

| Component | Description |
|-----------|-------------|
| **STM32 Nucleo-F446RE** | ARM Cortex-M4 @ 84 MHz |
| **QMC5883P (GY-271)** | 3-axis digital magnetometer |
| **SSD1306 OLED** | 128×64 monochrome display |

### Pinout

```
STM32 Nucleo-F446RE
├─ PB8  → I2C1_SCL (to QMC5883P & OLED)
├─ PB9  → I2C1_SDA (to QMC5883P & OLED)
├─ PA2  → USART2_TX (USB serial)
└─ PA3  → USART2_RX (USB serial)

QMC5883P: I2C Address 0x2C (7-bit)
OLED:     I2C Address 0x3C (7-bit)
```

## Physics Behind the Project

### Hall Effect
The QMC5883P sensor uses the **Hall effect** to detect magnetic fields. When current flows through a conductor in a magnetic field, the Lorentz force creates a voltage perpendicular to both:

```
V_H = (I × B) / (n × q × t)
```

Where:
- `V_H` = Hall voltage
- `B` = Magnetic field strength
- `I` = Current through the sensor
- `n` = Charge carrier density
- `t` = Conductor thickness

The system calculates total field strength:
```
|B| = √(Bx² + By² + Bz²)
```

## Building and Flashing

### Prerequisites
```bash
# Install toolchain
sudo apt install gcc-arm-none-eabi openocd make

# Or using Nix
nix-shell
```

### Build
```bash
make
```

### Flash to STM32
```bash
make flash
```

### Monitor Serial Output
```bash
picocom /dev/ttyACM0 -b 115200
```

## Project Structure

```
STMagnet/
├── Core/
│   ├── Src/
│   │   ├── main.c           # Main program loop
│   │   ├── qmc5883p.c       # Magnetometer driver
│   │   ├── ssd1306.c        # OLED display driver
│   │   ├── calibration.c    # Hard/soft-iron compensation
│   │   └── filters.c        # Low-pass filter (EMA)
│   └── Inc/
│       └── *.h              # Header files
├── Makefile
├── STMagnet.ioc             # STM32CubeMX configuration
└── tex/                     # documentation (Polish)
```

## Calibration

The system compensates for magnetic interference using:

### Hard-Iron Calibration
Removes constant offset from ferromagnetic materials:
```
offset = (max + min) / 2
```

### Soft-Iron Calibration
Corrects distortion in different axes:
```
scale = 2 / (max - min)
```

### Applying Calibration
```c
B_calibrated = scale × (B_raw - offset)
```

Rotate the sensor in all orientations during startup to collect min/max values for each axis.

## Signal Filtering

Exponential Moving Average (EMA) reduces measurement noise:
```
y[n] = α × x[n] + (1-α) × y[n-1]
```

Where `α ∈ (0,1)` is the smoothing factor (typically 0.1-0.3).

## QMC5883P Configuration

| Parameter | Value |
|-----------|-------|
| Measurement Range | ±8 Gauss (±800 µT) |
| Resolution | 16-bit (±32768) |
| Sensitivity | ~3000 LSB/Gauss |
| Output Data Rate | 100 Hz |
| Oversampling Ratio | 512 |
| I²C Address | 0x2C (7-bit) |

## Usage Example

```c
// Initialize
QMC5883P_Init(&magnetometer, &hi2c1);

// Read data
QMC5883P_ReadRaw(&magnetometer);

// Apply calibration
Calibration_Apply(&mag_cal, 
    magnetometer.x, magnetometer.y, magnetometer.z,
    &cal_x, &cal_y, &cal_z);

// Filter noise
LowPassFilter_Update(&filter, cal_x, cal_y, cal_z,
    &filt_x, &filt_y, &filt_z);

// Calculate magnitude
float magnitude = sqrtf(filt_x*filt_x + filt_y*filt_y + filt_z*filt_z);
