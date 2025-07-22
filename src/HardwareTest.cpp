#include <Arduino.h>
#include <Wire.h>
#include <Stepper.h>
#include <Servo.h>
#include "Config.h"
#include "Sensors.h"
#include "DisplayManager.h"
#include "UserInput.h"
#include "MotorControl.h"
#include "AudioManager.h"
#include "LightingEffects.h"

// Test objects
Sensors testSensors;
DisplayManager testDisplayManager;
UserInput testUserInput;
MotorControl testMotorControl;
AudioManager testAudioManager;
LightingEffects testLightingEffects;

// Test data structure
SensorData testData;

// Test function declarations
void testDisplay();
void testRotaryEncoder();
void testAudioModule();
void testRealTimeClock();
void testTemperatureHumiditySensor();
void testLightSensor();
void testPressureSensor();
void testLEDStrip();
void testStepperMotor();
void testServoMotor();
void testWeatherSummary();
void runInteractiveMenu();
void printTestHeader(const char* testName);
void waitForUserInput();
void populateTestSensorData();

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  
  Serial.println(F("==========================================="));
  Serial.println(F("    CHRONOSPHERE HARDWARE TEST SUITE"));
  Serial.println(F("==========================================="));
  Serial.println();
  
  // Initialize I2C
  Wire.begin();
  Serial.println(F("I2C initialized"));
  
  delay(2000); // Allow hardware to settle
  
  // Run interactive test menu
  runInteractiveMenu();
}

void loop() {
  // Test suite runs in setup(), loop is empty
  delay(1000);
}

void runInteractiveMenu() {
  while (true) {
    Serial.println(F("\n==========================================="));
    Serial.println(F("HARDWARE TEST MENU"));
    Serial.println(F("==========================================="));
    Serial.println(F("1. Test Display (HT16K33 LED Displays)"));
    Serial.println(F("2. Test Rotary Encoder"));
    Serial.println(F("3. Test Real Time Clock (DS3231)"));
    Serial.println(F("4. Test Temperature/Humidity Sensor (AHT21)"));
    Serial.println(F("5. Test Light Sensor (BH1750)"));
    Serial.println(F("6. Test Pressure Sensor (DFRobot BMP280)"));
    Serial.println(F("7. Test LED Strip (NeoPixel)"));
    Serial.println(F("8. Test Audio Module (VS1053)"));
    Serial.println(F("9. Test Stepper Motor"));
    Serial.println(F("S. Test Servo Motor"));
    Serial.println(F("W. Test Weather Summary (Multi-Sensor)"));
    Serial.println(F("A. Test All Devices (Sequential)"));
    Serial.println(F("0. Exit Test Suite"));
    Serial.println(F("==========================================="));
    Serial.print(F("Enter test number (0-9, S, W, A): "));
    
    // Wait for user input
    while (!Serial.available()) {
      delay(100);
    }
    
    char choice = Serial.read();
    Serial.println(choice);
    
    // Clear any remaining input
    while (Serial.available()) {
      Serial.read();
    }
    
    switch (choice) {
      case '1':
        testDisplay();
        break;
      case '2':
        testRotaryEncoder();
        break;
      case '3':
        testRealTimeClock();
        break;
      case '4':
        testTemperatureHumiditySensor();
        break;
      case '5':
        testLightSensor();
        break;
      case '6':
        testPressureSensor();
        break;
      case '7':
        testLEDStrip();
        break;
      case '8':
        testAudioModule();
        break;
      case '9':
        testStepperMotor();
        break;
      case 'S':
      case 's':
        testServoMotor();
        break;
      case 'W':
      case 'w':
        testWeatherSummary();
        break;
      case 'A':
      case 'a':
        // Test all devices sequentially
        testDisplay();
        testRotaryEncoder();
        testRealTimeClock();
        testTemperatureHumiditySensor();
        testLightSensor();
        testPressureSensor();
        testLEDStrip();
        testAudioModule();
        testStepperMotor();
        testServoMotor();
        testWeatherSummary();
        break;
      case '0':
        Serial.println(F("Exiting test suite..."));
        return;
      default:
        Serial.println(F("Invalid choice. Please try again."));
        break;
    }
  }
}

void testDisplay() {
  printTestHeader("HT16K33 LED DISPLAYS TEST");
  
  Serial.println(F("Initializing displays..."));
  if (testDisplayManager.init()) {
    Serial.println(F("✓ Display initialization successful"));
  } else {
    Serial.println(F("✗ Display initialization failed"));
    waitForUserInput();
    return;
  }
  
  // Test displays using the actual API
  Serial.println(F("\nTesting phase 1/3: Clock mode (time + date on all displays)"));
  Serial.println(F("Look for time in GREEN, month in AMBER, day in RED"));
  testDisplayManager.setMode(MODE_CLOCK);
  populateTestSensorData();
  testDisplayManager.update(testData);
  Serial.println(F("Press ENTER to continue to phase 2/3..."));
  waitForUserInput();
  
  Serial.println(F("Testing phase 2/3: Time-only display"));
  Serial.println(F("Look for time centered across all displays"));
  testDisplayManager.displayTimeOnly(testData.currentTime);
  Serial.println(F("Press ENTER to continue..."));
  waitForUserInput();
  
  Serial.println(F("Testing phase 3/3: Date-only display"));
  Serial.println(F("Look for date centered across all displays"));
  testDisplayManager.displayDateOnly(testData.currentTime);
  Serial.println(F("Press ENTER to complete display test..."));
  waitForUserInput();
  
  Serial.println(F("✓ Display test completed"));
}

void testRotaryEncoder() {
  printTestHeader("ROTARY ENCODER TEST");
  
  Serial.println(F("Initializing rotary encoder..."));
  if (testUserInput.init()) {
    Serial.println(F("✓ Rotary encoder initialization successful"));
  } else {
    Serial.println(F("✗ Rotary encoder initialization failed"));
    waitForUserInput();
    return;
  }
  
  Serial.println(F("\nRotary Encoder Test Instructions:"));
  Serial.println(F("- Turn the encoder clockwise and counterclockwise"));
  Serial.println(F("- Press the encoder button"));
  Serial.println(F("- Send any character via serial to exit test"));
  Serial.println(F("Starting test in 3 seconds..."));
  delay(3000);
  
  int lastPosition = 0;
  ButtonState lastButtonState = BUTTON_IDLE;
  
  while (!Serial.available()) {
    testUserInput.update();
    
    // Check for rotation
    int delta = testUserInput.getEncoderDelta();
    if (delta != 0) {
      Serial.print(F("Encoder delta: "));
      Serial.print(delta);
      Serial.print(F(" (Position: "));
      Serial.print(testUserInput.getEncoderPosition());
      Serial.println(F(")"));
    }
    
    // Check for button state changes
    ButtonState currentButtonState = testUserInput.getButtonState();
    if (currentButtonState != lastButtonState) {
      switch (currentButtonState) {
        case BUTTON_PRESSED:
          Serial.println(F("Button PRESSED"));
          break;
        case BUTTON_HELD:
          Serial.println(F("Button HELD"));
          break;
        case BUTTON_RELEASED:
          Serial.println(F("Button RELEASED"));
          break;
        default:
          break;
      }
      lastButtonState = currentButtonState;
    }
    
    delay(50);
  }
  
  // Clear serial input
  while (Serial.available()) {
    Serial.read();
  }
  
  Serial.println(F("✓ Rotary encoder test completed"));
  waitForUserInput();
}

void testAudioModule() {
  printTestHeader("VS1053 AUDIO MODULE TEST");
  
  Serial.println(F("Initializing audio module..."));
  if (testAudioManager.init()) {
    Serial.println(F("✓ Audio module initialization successful"));
    
    Serial.println(F("\nTesting audio playback with Columbo doorbell chime..."));
    Serial.println(F("You should hear a 3-note descending chime: G-E-C"));
    Serial.println(F("Playing in 2 seconds..."));
    delay(2000);
    
    // Play the Columbo doorbell chime (G-E-C descending)
    const uint8_t MIDI_CHANNEL = 0;
    const uint8_t NOTE_VELOCITY = 100;
    const uint8_t COLUMBO_NOTE_1 = 67; // G4
    const uint8_t COLUMBO_NOTE_2 = 64; // E4  
    const uint8_t COLUMBO_NOTE_3 = 60; // C4
    
    // Set to Tubular Bells for authentic chime sound
    testAudioManager.setChimeInstrument(INSTRUMENT_TUBULAR_BELLS);
    delay(100);
    
    Serial.println(F("Playing Note 1: G4 (67)"));
    testAudioManager.playNote(COLUMBO_NOTE_1, NOTE_VELOCITY, 1); // 1 quarter note = 250ms
    delay(200); // Short pause between notes
    
    Serial.println(F("Playing Note 2: E4 (64)"));
    testAudioManager.playNote(COLUMBO_NOTE_2, NOTE_VELOCITY, 1); // 1 quarter note = 250ms
    delay(200); // Short pause between notes
    
    Serial.println(F("Playing Note 3: C4 (60)"));
    testAudioManager.playNote(COLUMBO_NOTE_3, NOTE_VELOCITY, 2); // 2 quarter notes = 500ms
    delay(2500); // Let final note ring out
    
    Serial.println(F("✓ Audio playback test completed"));
    Serial.println(F("Did you hear the 3-note chime? (G-E-C descending)"));
  } else {
    Serial.println(F("✗ Audio module initialization failed"));
    Serial.println(F("Check VS1053 connections and power"));
  }
  
  Serial.println(F("✓ Audio module test completed"));
  waitForUserInput();
}

void testRealTimeClock() {
  printTestHeader("DS3231 REAL TIME CLOCK TEST");
  
  Serial.println(F("Initializing DS3231 RTC directly..."));
  
  // Initialize Wire for I2C communication
  Wire.begin();
  
  // Create a temporary DS3231 object for testing
  DS3231 rtc;
  
  // Test RTC by trying to read the year
  bool century = false;
  bool h12Flag;
  bool pm;
  
  uint8_t year = rtc.getYear();
  if (year > 99) { // Invalid year suggests RTC not working
    Serial.println(F("✗ DS3231 RTC initialization failed - cannot read valid year"));
    waitForUserInput();
    return;
  }
  
  Serial.println(F("✓ DS3231 RTC initialized successfully"));
  
  // Read current time directly from DS3231
  Serial.println(F("\nReading current time from DS3231..."));
  
  uint8_t rtcYear = rtc.getYear();
  uint8_t rtcMonth = rtc.getMonth(century);
  uint8_t rtcDay = rtc.getDate();
  uint8_t rtcHour = rtc.getHour(h12Flag, pm);
  uint8_t rtcMinute = rtc.getMinute();
  uint8_t rtcSecond = rtc.getSecond();
  
  // Convert to full year
  int fullYear = 2000 + rtcYear;
  
  Serial.print(F("Current Date/Time: "));
  Serial.print(fullYear, DEC);
  Serial.print('/');
  Serial.print(rtcMonth, DEC);
  Serial.print('/');
  Serial.print(rtcDay, DEC);
  Serial.print(" ");
  Serial.print(rtcHour, DEC);
  Serial.print(':');
  if (rtcMinute < 10) Serial.print('0');
  Serial.print(rtcMinute, DEC);
  Serial.print(':');
  if (rtcSecond < 10) Serial.print('0');
  Serial.println(rtcSecond, DEC);
  
  // Test setting a fixed time (July 21, 2025, 12:30:45)
  Serial.println(F("\nTesting time setting..."));
  Serial.println(F("Setting test time: 2025-07-21 12:30:45"));
  
  rtc.setYear(25); // DS3231 uses 2-digit year (25 = 2025)
  rtc.setMonth(7);
  rtc.setDate(21);
  rtc.setHour(12);
  rtc.setMinute(30);
  rtc.setSecond(45);
  
  Serial.println(F("✓ Time set successfully"));
  
  // Wait a moment then read back the set time
  delay(1000);
  
  Serial.println(F("Reading back set time..."));
  
  uint8_t readYear = rtc.getYear();
  uint8_t readMonth = rtc.getMonth(century);
  uint8_t readDay = rtc.getDate();
  uint8_t readHour = rtc.getHour(h12Flag, pm);
  uint8_t readMinute = rtc.getMinute();
  uint8_t readSecond = rtc.getSecond();
  
  int readFullYear = 2000 + readYear;
  
  Serial.print(F("Read Back Time: "));
  Serial.print(readFullYear, DEC);
  Serial.print('/');
  Serial.print(readMonth, DEC);
  Serial.print('/');
  Serial.print(readDay, DEC);
  Serial.print(" ");
  Serial.print(readHour, DEC);
  Serial.print(':');
  if (readMinute < 10) Serial.print('0');
  Serial.print(readMinute, DEC);
  Serial.print(':');
  if (readSecond < 10) Serial.print('0');
  Serial.println(readSecond, DEC);
  
  // Verify the time was set correctly (allowing for a few seconds difference)
  if (readFullYear == 2025 && readMonth == 7 && readDay == 21 && 
      readHour == 12 && readMinute == 30) {
    Serial.println(F("✓ RTC time setting and reading test PASSED"));
  } else {
    Serial.println(F("✗ RTC time setting verification FAILED"));
  }
  
  Serial.println(F("✓ RTC test completed"));
  waitForUserInput();
}

void testTemperatureHumiditySensor() {
  printTestHeader("AHT21 TEMPERATURE/HUMIDITY SENSOR TEST");
  
  Serial.println(F("Initializing AHT21 sensor directly..."));
  
  // Initialize I2C
  Wire.begin();
  
  // Create standalone AHT21 sensor object
  Adafruit_AHTX0 aht;
  
  if (!aht.begin()) {
    Serial.println(F("✗ AHT21 sensor initialization failed"));
    waitForUserInput();
    return;
  }
  
  Serial.println(F("✓ AHT21 sensor initialization successful"));
  
  Serial.println(F("\nReading temperature and humidity (5 samples)..."));
  for (int i = 0; i < 5; i++) {
    Serial.print(F("Sample "));
    Serial.print(i + 1);
    Serial.print(F(": "));
    
    sensors_event_t humidity, temp;
    if (aht.getEvent(&humidity, &temp)) {
      float tempC = temp.temperature;
      float tempF = tempC * 9.0 / 5.0 + 32.0;
      float hum = humidity.relative_humidity;
      
      Serial.print(F("Temperature: "));
      Serial.print(tempC, 2);
      Serial.print(F("°C ("));
      Serial.print(tempF, 2);
      Serial.print(F("°F), Humidity: "));
      Serial.print(hum, 2);
      Serial.println(F("%"));
      
      // Validate reasonable ranges
      if (tempC >= -40 && tempC <= 85 && hum >= 0 && hum <= 100) {
        Serial.println(F("  ✓ Values within expected range"));
      } else {
        Serial.println(F("  ⚠ Values outside expected range"));
      }
    } else {
      Serial.println(F("Failed to read sensor"));
    }
    
    delay(2000);
  }
  
  Serial.println(F("\nNote: Display test skipped (requires all sensors initialized)"));
  Serial.println(F("✓ Temperature/Humidity sensor test completed"));
  waitForUserInput();
}

void testLightSensor() {
  printTestHeader("BH1750 LIGHT SENSOR TEST");
  
  Serial.println(F("Initializing BH1750 light sensor directly..."));
  
  // Initialize I2C
  Wire.begin();
  
  // Create standalone BH1750 sensor object
  BH1750 lightMeter;
  
  if (!lightMeter.begin()) {
    Serial.println(F("✗ BH1750 sensor initialization failed"));
    waitForUserInput();
    return;
  }
  
  Serial.println(F("✓ BH1750 sensor initialization successful"));
  
  Serial.println(F("\nReading light levels (10 samples)..."));
  Serial.println(F("Try covering and uncovering the sensor to see changes"));
  delay(2000);
  
  for (int i = 0; i < 10; i++) {
    float lux = lightMeter.readLightLevel();
    
    Serial.print(F("Sample "));
    Serial.print(i + 1);
    Serial.print(F(": Light Level: "));
    Serial.print(lux, 2);
    Serial.print(F(" lux"));
    
    // Provide context for light levels
    if (lux < 10) {
      Serial.println(F(" (Dark)"));
    } else if (lux < 200) {
      Serial.println(F(" (Dim)"));
    } else if (lux < 1000) {
      Serial.println(F(" (Indoor lighting)"));
    } else {
      Serial.println(F(" (Bright)"));
    }
    
    // Validate reasonable range
    if (lux >= 0 && lux <= 65535) {
      Serial.println(F("  ✓ Value within expected range"));
    } else {
      Serial.println(F("  ⚠ Value outside expected range"));
    }
    
    delay(1000);
  }
  
  Serial.println(F("✓ Light sensor test completed"));
  waitForUserInput();
}

void testPressureSensor() {
  printTestHeader("DFROBOT BMP280 PRESSURE SENSOR TEST");
  
  Serial.println(F("Initializing DFRobot BMP280 pressure sensor directly..."));
  
  // Initialize I2C
  Wire.begin();
  
  // Create BMP280 sensor object exactly like the working example
  typedef DFRobot_BMP280_IIC BMP;
  BMP bmp(&Wire, BMP::eSdoLow);
  
  // Reset and initialize exactly like the working example
  bmp.reset();
  Serial.println("BMP280 initialization test");
  
  while(bmp.begin() != BMP::eStatusOK) {
    Serial.println("BMP280 begin failed");
    switch(bmp.lastOperateStatus) {
      case BMP::eStatusOK: Serial.println("everything ok"); break;
      case BMP::eStatusErr: Serial.println("unknown error"); break;
      case BMP::eStatusErrDeviceNotDetected: Serial.println("device not detected"); break;
      case BMP::eStatusErrParameter: Serial.println("parameter error"); break;
      default: Serial.println("unknown status"); break;
    }
    Serial.println(F("✗ BMP280 sensor initialization failed"));
    Serial.println(F("  Check I2C connections and sensor power"));
    waitForUserInput();
    return;
  }
  
  Serial.println("BMP280 begin success");
  Serial.println(F("✓ BMP280 sensor initialization successful"));
  
  // Configure sensor exactly like working example
  bmp.setConfigFilter(BMP::eConfigFilter_off);
  bmp.setConfigTStandby(BMP::eConfigTStandby_125);
  bmp.setCtrlMeasSamplingTemp(BMP::eSampling_X8);
  bmp.setCtrlMeasSamplingPress(BMP::eSampling_X8);
  bmp.setCtrlMeasMode(BMP::eCtrlMeasModeNormal);
  
  delay(100);
  
  Serial.println(F("\nReading pressure data (5 samples)..."));
  for (int i = 0; i < 5; i++) {
    float temp = bmp.getTemperature();
    uint32_t press = bmp.getPressure();
    float pressureHPa = press / 100.0; // Convert Pa to hPa
    
    Serial.print(F("Sample "));
    Serial.print(i + 1);
    Serial.print(F(": Temperature: "));
    Serial.print(temp);
    Serial.print(F("°C, Pressure: "));
    Serial.print(press);
    Serial.print(F(" Pa ("));
    Serial.print(pressureHPa, 2);
    Serial.print(F(" hPa)"));
    
    // Provide context for pressure levels
    if (pressureHPa < 980) {
      Serial.println(F(" (Low - Storm)"));
    } else if (pressureHPa < 1013) {
      Serial.println(F(" (Below Average)"));
    } else if (pressureHPa < 1030) {
      Serial.println(F(" (Normal)"));
    } else {
      Serial.println(F(" (High)"));
    }
    
    // Validate reasonable range
    if (pressureHPa >= 800 && pressureHPa <= 1200) {
      Serial.println(F("  ✓ Values within expected range"));
    } else {
      Serial.println(F("  ⚠ Values outside expected range"));
    }
    
    delay(2000);
  }
  
  Serial.println(F("✓ Pressure sensor test completed"));
  waitForUserInput();
}

void testLEDStrip() {
  printTestHeader("NEOPIXEL LED STRIP TEST");
  
  Serial.println(F("Initializing NeoPixel LED strip..."));
  if (testLightingEffects.init()) {
    Serial.println(F("✓ LED strip initialization successful"));
  } else {
    Serial.println(F("✗ LED strip initialization failed"));
    waitForUserInput();
    return;
  }
  
  Serial.println(F("\nTesting LED strip effects..."));
  
  // Test different lighting modes
  Serial.println(F("Test 1: Solid Red"));
  testLightingEffects.setMode(LIGHTING_SOLID_COLOR);
  testLightingEffects.setSolidColor(255, 0, 0); // Red
  delay(2000);
  
  Serial.println(F("Test 2: Solid Green"));
  testLightingEffects.setSolidColor(0, 255, 0); // Green
  delay(2000);
  
  Serial.println(F("Test 3: Solid Blue"));
  testLightingEffects.setSolidColor(0, 0, 255); // Blue
  delay(2000);
  
  Serial.println(F("Test 4: Temperature Gradient (75°F)"));
  testLightingEffects.setMode(LIGHTING_TEMPERATURE_GRADIENT);
  populateTestSensorData();
  testData.temperatureF = 75.0;
  testLightingEffects.update(testData);
  delay(3000);
  
  Serial.println(F("Test 5: Rainbow Mode"));
  testLightingEffects.setMode(LIGHTING_RAINBOW);
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {
    testLightingEffects.update(testData);
    delay(50);
  }
  
  Serial.println(F("Turning off LEDs..."));
  testLightingEffects.setMode(LIGHTING_OFF);
  testLightingEffects.update(testData);
  
  Serial.println(F("✓ LED strip test completed"));
  waitForUserInput();
}

void testWeatherSummary() {
  printTestHeader("WEATHER SUMMARY MULTI-SENSOR TEST");
  
  Serial.println(F("This test combines all environmental sensors:"));
  Serial.println(F("- Temperature/Humidity Sensor (AHT21)"));
  Serial.println(F("- Light Sensor (BH1750)"));  
  Serial.println(F("- Pressure Sensor (DFRobot BMP280)"));
  Serial.println(F("- Real Time Clock (DS3231)"));
  Serial.println();
  
  Serial.println(F("Initializing sensors..."));
  if (testSensors.init()) {
    Serial.println(F("✓ All sensors initialization successful"));
  } else {
    Serial.println(F("✗ One or more sensors failed to initialize"));
    Serial.println(F("Individual sensor tests should be run first"));
    waitForUserInput();
    return;
  }
  
  Serial.println(F("\nReading all sensors and displaying weather summary..."));
  if (testSensors.readSensors()) {
    SensorData data = testSensors.getCurrentData();
    
    Serial.println(F("Current sensor readings:"));
    Serial.print(F("  Temperature: "));
    Serial.print(data.temperature, 2);
    Serial.print(F("°C ("));
    Serial.print(data.temperatureF, 2);
    Serial.print(F("°F), Word: "));
    Serial.println(data.tempWord);
    Serial.print(F("  Humidity: "));
    Serial.print(data.humidity, 2);
    Serial.println(F("%"));
    Serial.print(F("  Pressure: "));
    Serial.print(data.pressure, 2);
    Serial.println(F(" hPa"));
    Serial.print(F("  Light Level: "));
    Serial.print(data.lightLevel, 1);
    Serial.println(F(" lux"));
    
    Serial.println(F("\nDisplaying weather summary on LED displays..."));
    Serial.println(F("Look for weather data (temp, humidity, pressure) displayed"));
    testDisplayManager.setMode(MODE_WEATHER_SUMMARY);
    testDisplayManager.update(data);
    Serial.println(F("Press ENTER to continue..."));
    waitForUserInput();
  } else {
    Serial.println(F("Failed to read sensor data"));
    Serial.println(F("Check individual sensor connections and run individual tests"));
  }
  
  Serial.println(F("✓ Weather summary test completed"));
  waitForUserInput();
}

void populateTestSensorData() {
  // Fill test data with reasonable values
  testData.currentTime = DateTime(2025, 7, 21, 12, 30, 45); // DS3231-RTC DateTime uses 4-digit year
  testData.temperature = 23.5;      // 23.5°C
  testData.humidity = 45.0;         // 45%
  testData.pressure = 1013.25;      // 1013.25 hPa (standard pressure)
  testData.lightLevel = 300.0;      // 300 lux (indoor lighting)
  testData.temperatureF = 74.3;     // ~74°F
  testData.feelsLikeF = 75.0;       // Feels like 75°F
  strcpy(testData.tempWord, "NICE");
  testData.displayColor = 0;        // Green display
}

void printTestHeader(const char* testName) {
  Serial.println();
  Serial.println(F("==========================================="));
  Serial.print(F("TESTING: "));
  Serial.println(testName);
  Serial.println(F("==========================================="));
}

void waitForUserInput() {
  Serial.println(F("\nPress ENTER to return to main menu..."));
  while (!Serial.available()) {
    delay(100);
  }
  while (Serial.available()) {
    Serial.read();
  }
}

void testStepperMotor() {
  printTestHeader("STEPPER MOTOR TEST");
  
  Serial.println(F("Initializing stepper motor..."));
  Serial.println(F("Using 28BYJ-48 stepper motor with ULN2003 driver"));
  Serial.println(F("Pins: 14, 15, 16, 17"));
  
  // Create stepper object (steps per revolution, pin1, pin2, pin3, pin4)
  // 28BYJ-48 has 2048 steps per full revolution in half-step mode
  Stepper stepper(2048, STEPPER_PIN1, STEPPER_PIN3, STEPPER_PIN2, STEPPER_PIN4);
  
  // Set speed in RPM (revolutions per minute)
  stepper.setSpeed(10); // 10 RPM for smooth operation
  
  Serial.println(F("✓ Stepper motor initialized"));
  
  Serial.println(F("\nTesting stepper motor movement..."));
  Serial.println(F("You should see/hear the motor turning"));
  
  // Test 1: Full rotation clockwise
  Serial.println(F("Test 1: Full rotation clockwise (2048 steps)"));
  stepper.step(2048);
  delay(1000);
  
  // Test 2: Full rotation counterclockwise  
  Serial.println(F("Test 2: Full rotation counterclockwise (-2048 steps)"));
  stepper.step(-2048);
  delay(1000);
  
  // Test 3: Quarter rotations
  Serial.println(F("Test 3: Four quarter rotations clockwise"));
  for (int i = 0; i < 4; i++) {
    Serial.print(F("Quarter rotation "));
    Serial.print(i + 1);
    Serial.println(F("/4"));
    stepper.step(512); // 1/4 of 2048 steps
    delay(500);
  }
  
  // Test 4: Small incremental steps
  Serial.println(F("Test 4: Small incremental steps (64 steps x 8)"));
  for (int i = 0; i < 8; i++) {
    Serial.print(F("Step "));
    Serial.print(i + 1);
    Serial.println(F("/8"));
    stepper.step(64);
    delay(300);
  }
  
  Serial.println(F("✓ Stepper motor test completed"));
  Serial.println(F("Note: Motor should have rotated smoothly in both directions"));
  waitForUserInput();
}

void testServoMotor() {
  printTestHeader("SERVO MOTOR TEST");
  
  Serial.println(F("Initializing servo motor..."));
  Serial.println(F("Using standard 180° servo on pin 5"));
  
  Servo testServo;
  testServo.attach(SERVO_PIN);
  
  Serial.println(F("✓ Servo motor initialized"));
  
  Serial.println(F("\nTesting servo motor movement..."));
  Serial.println(F("You should see the servo horn moving to different positions"));
  
  // Test 1: Move to center position
  Serial.println(F("Test 1: Moving to center position (90°)"));
  testServo.write(90);
  delay(1000);
  
  // Test 2: Sweep from 0 to 180 degrees
  Serial.println(F("Test 2: Sweeping from 0° to 180°"));
  for (int pos = 0; pos <= 180; pos += 10) {
    testServo.write(pos);
    Serial.print(F("Position: "));
    Serial.print(pos);
    Serial.println(F("°"));
    delay(200);
  }
  
  delay(500);
  
  // Test 3: Sweep from 180 to 0 degrees
  Serial.println(F("Test 3: Sweeping from 180° to 0°"));
  for (int pos = 180; pos >= 0; pos -= 10) {
    testServo.write(pos);
    Serial.print(F("Position: "));
    Serial.print(pos);
    Serial.println(F("°"));
    delay(200);
  }
  
  delay(500);
  
  // Test 4: Specific positions
  Serial.println(F("Test 4: Moving to specific positions"));
  int positions[] = {0, 45, 90, 135, 180, 90};
  int numPositions = sizeof(positions) / sizeof(positions[0]);
  
  for (int i = 0; i < numPositions; i++) {
    Serial.print(F("Moving to "));
    Serial.print(positions[i]);
    Serial.println(F("°"));
    testServo.write(positions[i]);
    delay(1000);
  }
  
  // Test 5: Rapid movement test
  Serial.println(F("Test 5: Rapid movement test"));
  for (int i = 0; i < 5; i++) {
    testServo.write(0);
    delay(300);
    testServo.write(180);
    delay(300);
  }
  
  // Return to center
  Serial.println(F("Returning to center position"));
  testServo.write(90);
  delay(500);
  
  testServo.detach();
  
  Serial.println(F("✓ Servo motor test completed"));
  Serial.println(F("Note: Servo should have moved smoothly to all positions"));
  waitForUserInput();
}
