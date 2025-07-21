# Weather Clock

A sophisticated Arduino-based weather clock that combines timekeeping, environmental monitoring, and interactive displays.

## Features

### Hardware Components
- **Arduino Nano/Nano Every** - Main controller
- **DS3231 RTC** - Real-time clock with battery backup
- **AHT21** - Temperature and humidity sensor
- **BMP280** - Barometric pressure sensor  
- **BH1750** - Light sensor for ambient light detection
- **HT16K33 Displays** - Three 4-character alphanumeric displays (Green, Amber, Red)
- **Rotary Encoder** - User input for mode selection and settings
- **VS1053 MIDI** - Audio module for chimes and alerts
- **WS2812 LEDs** - Addressable RGB lighting effects
- **28BYJ48 Stepper** - Animated minute hand (2048 steps/hour)
- **Servo Motor** - Animated pendulum

### Software Features
- **Clock Functions**: Time/date display, animated minute hand, pendulum motion
- **Weather Monitoring**: Real-time sensor readings with historical data logging
- **Smart Temperature Display**: 
  - Raw temperature in Fahrenheit
  - "Feels like" temperature using NOAA heat index calculations
  - Four-letter descriptive words (FROZ, COLD, CHLY, COOL, NICE, WARM, COZY, TOSY, HOT, SCOR)
  - Color-coded display (Green=comfortable, Amber=moderate, Red=uncomfortable)
- **Multiple Display Modes**:
  - Clock display
  - Temperature focus
  - Weather summary
  - Rolling current data
  - Historical data trends
- **Audio Features**: 
  - Hourly chimes (Westminster, Whittington, St. Michael's)
  - Configurable MIDI instruments
  - Weather alerts
- **User Interface**: Rotary encoder for mode selection and configuration
- **Data Logging**: Historical data with trend analysis and weather prediction

## Project Structure

```
Weather Clock/
├── platformio.ini          # PlatformIO configuration
├── include/                 # Header files
│   ├── Config.h            # Pin definitions and constants
│   ├── Sensors.h           # Sensor management
│   ├── DisplayManager.h    # Display control
│   ├── UserInput.h         # Rotary encoder and button handling
│   ├── MotorControl.h      # Stepper motor and servo control
│   ├── AudioManager.h      # MIDI chimes and alerts
│   ├── DataLogger.h        # Historical data and trends
│   └── LightingEffects.h   # RGB LED effects
├── src/                    # Source files
│   ├── main.cpp            # Main application
│   ├── Sensors.cpp         # Sensor implementations
│   ├── DisplayManager.cpp  # Display implementations
│   ├── UserInput.cpp       # Input handling
│   ├── MotorControl.cpp    # Motor control implementations
│   └── ... (other .cpp files)
└── README.md               # This file
```

## Pin Assignments

| Component | Pin | Notes |
|-----------|-----|-------|
| Rotary Encoder CLK | 2 | Interrupt capable |
| Rotary Encoder DT | 3 | Interrupt capable |
| Rotary Button | 4 | |
| Stepper Motor | 5,6,7,8 | 28BYJ48 control pins |
| Servo | 9 | PWM pin |
| VS1053 CS | 10 | SPI |
| VS1053 DCS | 11 | SPI |
| VS1053 DREQ | 12 | |
| WS2812 LEDs | 13 | |
| I2C SDA | A4 | All sensors and displays |
| I2C SCL | A5 | All sensors and displays |

## I2C Addresses

| Device | Address |
|--------|---------|
| DS3231 RTC | 0x68 |
| AHT21 Sensor | 0x38 |
| BMP280 Sensor | 0x76 |
| BH1750 Light | 0x23 |
| Green Display | 0x70 |
| Amber Display | 0x71 |
| Red Display | 0x72 |

## Temperature Display Logic

The system displays temperature in three ways:

1. **Raw Temperature**: Actual temperature in °F with one decimal place
2. **Feels Like**: NOAA heat index calculation for apparent temperature
3. **Four-Letter Words**: Descriptive terms for 5-degree ranges:
   - FROZ (≤19°F), COLD (20-34°F), CHLY (35-49°F), COOL (50-64°F)
   - NICE (65-79°F), WARM (80-84°F), COZY (85-89°F), TOSY (90-94°F)
   - HOT (95-104°F), SCOR (≥105°F)

Display colors indicate comfort level:
- **Green**: Comfortable temperatures (NICE range)
- **Amber**: Moderate temperatures  
- **Red**: Uncomfortable temperatures (very cold or hot)

## Building and Installation

### Prerequisites
- PlatformIO Core or PlatformIO IDE
- Arduino framework support
- All required libraries (automatically installed via platformio.ini)

### Build Instructions
1. Clone or download this project
2. Open in PlatformIO
3. Select your target environment (`nanoatmega328` or `nano_every`)
4. Build and upload to your Arduino

### Hardware Setup
1. Connect all components according to pin assignments
2. Ensure proper power supply for all components
3. Calibrate motors on first startup

## Usage

### Operating Modes
- Turn rotary encoder to cycle through display modes
- Press rotary button to enter settings mode
- Hold button for extended settings access

### Settings Menu
- Time/Date adjustment
- Chime type selection
- MIDI instrument selection
- Chime frequency (hourly, half-hourly, quarter-hourly)
- Sensor sensitivity adjustments
- Weather alert thresholds

## Development Notes

- The stepper motor timing accounts for the non-even division of 2048 steps per hour
- All operations are non-blocking to maintain smooth pendulum motion
- Data logging uses EEPROM for persistence across power cycles
- Weather prediction uses simple trend analysis of pressure and temperature changes

## Future Enhancements

- WiFi connectivity for NTP time sync and weather data
- Sunrise/sunset calculations and chimes
- Additional chime melodies
- Web interface for remote monitoring
- Data export capabilities
