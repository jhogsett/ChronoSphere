# ChronoSphere Hardware Test Suite

This hardware test suite provides comprehensive testing of each individual hardware component of the Weather Clock project. It allows you to verify each sensor and device one-at-a-time before integrating them all together.

## Quick Start

### Building the Test Suite
```bash
# Build for Arduino Nano Every (recommended)
pio run -e test_nano_every

# Build for Arduino Nano ATmega328 (compatibility)
pio run -e test_nanoatmega328

# Upload to connected Arduino
pio run -e test_nano_every -t upload

# Monitor serial output
pio device monitor
```

### Running Tests
1. Connect your Arduino via USB
2. Upload the test firmware
3. Open serial monitor (115200 baud)
4. Use the interactive menu to test each component

## Complete Usage Instructions

### Step-by-Step Hardware Testing Process

#### Phase 1: Setup and Preparation
1. **Hardware Connection**
   - Connect Arduino Nano Every via USB cable
   - Wire all components according to pin definitions in `Config.h`
   - Ensure adequate power supply (5V for displays, check sensor requirements)
   - Verify I2C connections (SDA/SCL shared by multiple devices)

2. **Software Preparation**
   ```bash
   # Navigate to project directory
   cd c:\DEV\ChronoSphere
   
   # Build the hardware test suite
   pio run -e test_nano_every
   
   # Upload to Arduino (ensure correct COM port)
   pio run -e test_nano_every -t upload
   
   # Open serial monitor
   pio device monitor --baud 115200
   ```

#### Phase 2: Individual Component Testing

**🟢 Start with Display Test (Recommended First)**
- Choose option `1` from the menu
- Verify all three displays (Green, Amber, Red) show text
- Check display brightness and readability
- Confirm different display modes work correctly

**🔄 Test Rotary Encoder Next**
- Choose option `2` from the menu
- Turn encoder clockwise/counterclockwise
- Press and hold the encoder button
- Verify all movements are detected and reported

**🕒 Test Real Time Clock**
- Choose option `4` from the menu
- Verify current time reading (may be incorrect initially)
- Test setting a specific time
- Confirm time persists and increments correctly

**🌡️ Test Temperature/Humidity Sensor**
- Choose option `5` from the menu
- Verify reasonable temperature readings (~20-30°C room temp)
- Check humidity readings (~30-70% typical indoor)
- Confirm "feels like" calculations and temperature words

**💡 Test Light Sensor**
- Choose option `6` from the menu
- Cover and uncover sensor to see readings change
- Verify different light levels (Dark < 10, Dim 10-200, etc.)
- Ensure sensor responds to ambient light changes

**🌊 Test Pressure Sensor**
- Choose option `7` from the menu
- Verify pressure readings around 1013 hPa (standard sea level)
- Check for stable readings (minor variations normal)
- Note: Altitude affects pressure readings

**🌈 Test LED Strip**
- Choose option `8` from the menu
- Verify all colors display correctly (Red, Green, Blue)
- Check rainbow animation runs smoothly
- Test temperature gradient display
- Confirm all LEDs light up uniformly

**🔊 Test Audio Module (When Enabled)**
- Choose option `3` from the menu
- Currently returns placeholder (VS1053 commented out)
- Ready for future audio integration testing

#### Phase 3: Integration Testing

**Complete System Test**
- Choose option `9` to run all tests sequentially
- Monitor for any conflicts between devices
- Verify I2C bus stability under full load
- Check power consumption doesn't cause brownouts

#### Phase 4: Validation and Troubleshooting

### Common Issues and Solutions

**🚨 Display Issues**
```
Symptoms: "Display initialization failed"
Solutions:
- Check I2C wiring (SDA to A4, SCL to A5 on Nano)
- Verify I2C addresses: 0x70, 0x71, 0x72
- Ensure 5V power to displays
- Test I2C scanner code if needed
```

**🚨 Sensor Reading Errors**
```
Symptoms: "Failed to read sensor" messages
Solutions:
- Allow sensors 2-3 minutes to stabilize after power-on
- Check individual I2C addresses don't conflict
- Verify sensor power requirements (3.3V vs 5V)
- Test with simpler I2C scanner first
```

**🚨 Encoder Not Responding**
```
Symptoms: No rotation or button detection
Solutions:
- Verify pin connections (CLK=2, DT=3, SW=4)
- Check for loose wiring
- May need external 10kΩ pull-up resistors
- Test mechanical encoder by hand rotation
```

**🚨 LED Strip Not Working**
```
Symptoms: No LEDs lighting or wrong colors
Solutions:
- Check data pin connection (Pin 13)
- Verify adequate power supply for LEDs
- Test with reduced brightness first
- Ensure common ground between Arduino and LED strip
```

### Serial Monitor Output Examples

**Successful Test Session:**
```
===========================================
    CHRONOSPHERE HARDWARE TEST SUITE
===========================================

I2C initialized

===========================================
HARDWARE TEST MENU
===========================================
1. Test Display (HT16K33 LED Displays)
...
Enter test number (0-9): 1

===========================================
TESTING: HT16K33 LED DISPLAYS TEST
===========================================
Initializing displays...
✓ Display initialization successful

Testing displays with clock mode...
Testing displays with temperature mode...
✓ Display test completed
```

**Troubleshooting Session:**
```
===========================================
TESTING: AHT21 TEMPERATURE/HUMIDITY SENSOR TEST
===========================================
Initializing AHT21 sensor...
✗ AHT21 sensor initialization failed

Check sensor wiring and power supply
Press ENTER to return to main menu...
```

### Test Results Interpretation

**🟢 Good Results:**
- All devices initialize successfully (✓ messages)
- Sensor readings within expected ranges
- Smooth encoder response and LED animations
- Stable I2C communication

**🟡 Marginal Results:**
- Occasional sensor read failures (retry usually works)
- Minor timing issues with rapid encoder movement
- LED brightness variations acceptable

**🔴 Problem Indicators:**
- Consistent initialization failures
- Completely unreasonable sensor readings
- No response from interactive devices
- I2C bus lockups requiring reset

### Advanced Usage

**Custom Test Modifications:**
```cpp
// To add custom test patterns, modify these functions:
void populateTestSensorData() {
  // Change test data values here
  testData.temperatureF = 85.0; // Test hot temperature
}

void testLEDStrip() {
  // Add custom LED patterns here
  testLightingEffects.setSolidColor(255, 255, 0); // Yellow
}
```

**Building for Different Targets:**
```bash
# For Arduino Nano ATmega328 (memory constrained)
pio run -e test_nanoatmega328
pio run -e test_nanoatmega328 -t upload

# For main application (after testing complete)
pio run -e nano_every
pio run -e nano_every -t upload
```

### Integration with Main Application

**After Successful Hardware Testing:**
1. Document which tests passed/failed
2. Note any hardware quirks or timing issues
3. Update main application if needed for discovered issues
4. Switch back to main firmware for full system testing

**Switching Between Test and Main Firmware:**
```bash
# Upload test suite
pio run -e test_nano_every -t upload

# Upload main weather clock application
pio run -e nano_every -t upload

# Monitor either firmware
pio device monitor
```

## Hardware Components Tested

### 1. Display (HT16K33 LED Displays)
- **I2C Addresses**: 0x70 (Green), 0x71 (Amber), 0x72 (Red)
- **Brightness Compensation**: Green=1, Amber=9, Red=15 (compensates for LED color variations)
- **Test Coverage**:
  - Individual display initialization with color-compensated brightness
  - Different display modes (clock, temperature, weather)
  - Startup message and error display
  - Display brightness uniformity across all three colors

### 2. Rotary Encoder
- **Pins**: CLK (Pin 2), DT (Pin 3), SW (Pin 4)
- **Test Coverage**:
  - Rotation detection (clockwise/counterclockwise)
  - Button press/release detection
  - Long press detection
  - Debouncing verification

### 3. Audio Module (VS1053)
- **Pins**: CS (Pin 10), DCS (Pin 11), DREQ (Pin 12)
- **Test Coverage**:
  - Module initialization
  - MIDI playback capability
  - Note: Currently commented out in AudioManager

### 4. Real Time Clock (DS3231)
- **I2C Address**: 0x68
- **Test Coverage**:
  - Time/date reading
  - Time/date setting
  - Backup battery functionality
  - Time accuracy verification

### 5. Temperature/Humidity Sensor (AHT21)
- **I2C Address**: 0x38
- **Test Coverage**:
  - Temperature reading (Celsius and Fahrenheit)
  - Humidity reading (%)
  - "Feels like" temperature calculation
  - Temperature word generation (FROZ, COLD, NICE, etc.)

### 6. Light Sensor (BH1750)
- **I2C Address**: 0x23
- **Test Coverage**:
  - Light level reading (lux)
  - Ambient light detection
  - Brightness range classification
  - Sensor sensitivity verification

### 7. Pressure Sensor (DFRobot BMP280)
- **I2C Communication**: SDO pulled low
- **Test Coverage**:
  - Atmospheric pressure reading (hPa)
  - Pressure trend calculation
  - Weather prediction data
  - Sensor calibration verification

### 8. LED Strip (NeoPixel)
- **Pin**: Pin 13
- **LED Count**: 8 LEDs (optimized for memory)
- **Test Coverage**:
  - Individual color testing (Red, Green, Blue)
  - Rainbow animation
  - Temperature gradient display
  - Brightness control

## Test Menu Options

```
===========================================
HARDWARE TEST MENU
===========================================
1. Test Display (HT16K33 LED Displays)
2. Test Rotary Encoder
3. Test Audio Module (VS1053)
4. Test Real Time Clock (DS3231)
5. Test Temperature/Humidity Sensor (AHT21)
6. Test Light Sensor (BH1750)
7. Test Pressure Sensor (DFRobot BMP280)
8. Test LED Strip (NeoPixel)
9. Test All Devices (Sequential)
0. Exit Test Suite
===========================================
```

## Expected Test Outputs

### Successful Display Test
```
✓ Display initialization successful
Testing phase 1/5: Display brightness and color compensation
Press ENTER to continue to phase 2/5...

Testing phase 2/5: Clock mode (time display on all three displays)  
Press ENTER to continue to phase 3/5...

Testing phase 3/5: Temperature mode (temp, humidity, feels-like)
Press ENTER to continue to phase 4/5...

Testing phase 4/5: Weather summary mode (pressure, light, comfort index)
Press ENTER to continue to phase 5/5...

Testing phase 5/5: Final verification - all displays working together
Press ENTER to complete display test...
✓ Display test completed
```

### Successful Sensor Test
```
Sample 1: Temperature: 23.50°C (74.30°F), Humidity: 45.00%, Feels Like: 75.00°F, Word: NICE
Sample 2: Temperature: 23.52°C (74.34°F), Humidity: 45.10%, Feels Like: 75.02°F, Word: NICE
```

### Successful LED Test
```
Test 1: Solid Red
Test 2: Solid Green  
Test 3: Solid Blue
Test 4: Temperature Gradient (75°F)
Test 5: Rainbow Mode
✓ LED strip test completed
```

## Troubleshooting

### Common Issues

**Display Not Working**
- Check I2C connections (SDA/SCL)
- Verify I2C addresses (0x70, 0x71, 0x72)
- Ensure 5V power supply
- **Brightness Matching**: Different colored LEDs have different brightness levels at the same setting. The code automatically compensates (Green=1, Amber=9, Red=15) to achieve uniform apparent brightness

**Sensor Reading Errors**
- Check I2C wiring
- Verify sensor power (3.3V or 5V as required)
- Allow sensors to stabilize (temperature sensors need ~2-3 minutes)

**Encoder Not Responding**
- Verify pin connections (CLK=2, DT=3, SW=4)
- Check pull-up resistors (may need external 10kΩ)
- Test mechanical encoder rotation

**LED Strip Not Lighting**
- Check data pin connection (Pin 13)
- Verify power supply (LEDs need adequate current)
- Test with lower brightness first

### Memory Usage
- **Arduino Nano Every**: 17.6% RAM, 69.2% Flash - ✅ Good headroom
- **Arduino Nano ATmega328**: May require reduced feature set

## Hardware Validation Checklist

Use this checklist when testing your hardware:

### Before Starting
- [ ] Arduino connected via USB
- [ ] All I2C devices connected to SDA/SCL
- [ ] Power supply adequate for all components
- [ ] Serial monitor open at 115200 baud

### Individual Component Tests
- [ ] **Display**: All three displays show text clearly
- [ ] **Encoder**: Rotation and button press detected
- [ ] **RTC**: Time can be set and read accurately
- [ ] **AHT21**: Temperature and humidity readings reasonable
- [ ] **BH1750**: Light readings change when covered/uncovered
- [ ] **BMP280**: Pressure readings around 1013 hPa (standard)
- [ ] **NeoPixels**: All LEDs light in different colors
- [ ] **Audio**: Module initializes (when enabled)

### Integration Readiness
- [ ] All individual tests pass
- [ ] No I2C address conflicts
- [ ] Stable power supply under load
- [ ] No intermittent connection issues

## Next Steps

After successful hardware testing:
1. Use main firmware: `pio run -e nano_every && pio upload -e nano_every`
2. Test integrated functionality
3. Calibrate sensors for local conditions
4. Implement time setting UI
5. Add audio system integration

## File Structure

```
src/HardwareTest.cpp     - Test suite main file
platformio.ini          - Includes test_nano_every and test_nanoatmega328 environments
include/                 - All header files for component APIs
src/                     - Implementation files for each component
```

This test suite provides a solid foundation for validating your Weather Clock hardware before moving to full system integration.
