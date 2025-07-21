#include <Arduino.h>
#include <Wire.h>
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
    Serial.println(F("3. Test Audio Module (VS1053)"));
    Serial.println(F("4. Test Real Time Clock (DS3231)"));
    Serial.println(F("5. Test Temperature/Humidity Sensor (AHT21)"));
    Serial.println(F("6. Test Light Sensor (BH1750)"));
    Serial.println(F("7. Test Pressure Sensor (DFRobot BMP280)"));
    Serial.println(F("8. Test LED Strip (NeoPixel)"));
    Serial.println(F("9. Test All Devices (Sequential)"));
    Serial.println(F("0. Exit Test Suite"));
    Serial.println(F("==========================================="));
    Serial.print(F("Enter test number (0-9): "));
    
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
        testAudioModule();
        break;
      case '4':
        testRealTimeClock();
        break;
      case '5':
        testTemperatureHumiditySensor();
        break;
      case '6':
        testLightSensor();
        break;
      case '7':
        testPressureSensor();
        break;
      case '8':
        testLEDStrip();
        break;
      case '9':
        // Test all devices sequentially
        testDisplay();
        testRotaryEncoder();
        testAudioModule();
        testRealTimeClock();
        testTemperatureHumiditySensor();
        testLightSensor();
        testPressureSensor();
        testLEDStrip();
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
  Serial.println(F("\nTesting phase 2/5: Clock mode (time + date on all displays)"));
  Serial.println(F("Look for time in GREEN, month in AMBER, day in RED"));
  testDisplayManager.setMode(MODE_CLOCK);
  populateTestSensorData();
  testDisplayManager.update(testData);
  Serial.println(F("Press ENTER to continue to phase 3/5..."));
  waitForUserInput();
  
  Serial.println(F("Testing phase 3/5: Time-only display"));
  Serial.println(F("Look for time centered across all displays"));
  testDisplayManager.displayTimeOnly(testData.currentTime);
  Serial.println(F("Press ENTER to continue..."));
  waitForUserInput();
  
  Serial.println(F("Testing date-only display"));
  Serial.println(F("Look for date centered across all displays"));
  testDisplayManager.displayDateOnly(testData.currentTime);
  Serial.println(F("Press ENTER to continue..."));
  waitForUserInput();
  
  Serial.println(F("Testing phase 4/5: Temperature mode"));
  Serial.println(F("Look for temperature data across the displays"));
  testDisplayManager.setMode(MODE_TEMPERATURE);
  testDisplayManager.update(testData);
  Serial.println(F("Press ENTER to continue..."));
  waitForUserInput();
  
  Serial.println(F("Testing phase 5/5: Weather summary mode"));
  Serial.println(F("Look for weather data (temp, humidity, pressure) displayed"));
  testDisplayManager.setMode(MODE_WEATHER_SUMMARY);
  testDisplayManager.update(testData);
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
    
    Serial.println(F("\nTesting audio playback..."));
    Serial.println(F("Note: Audio module functionality is currently commented out"));
    Serial.println(F("Check AudioManager.cpp for implementation details"));
  } else {
    Serial.println(F("✗ Audio module initialization failed"));
    Serial.println(F("Note: Audio module is currently commented out in AudioManager"));
  }
  
  Serial.println(F("✓ Audio module test completed"));
  Serial.println(F("Note: Listen for audio output to verify functionality when enabled"));
  waitForUserInput();
}

void testRealTimeClock() {
  printTestHeader("DS3231 REAL TIME CLOCK TEST");
  
  Serial.println(F("Initializing RTC..."));
  if (testSensors.init()) {
    Serial.println(F("✓ RTC initialization successful"));
  } else {
    Serial.println(F("✗ RTC initialization failed"));
    waitForUserInput();
    return;
  }
  
  // Read sensor data to get current time
  Serial.println(F("\nReading current time..."));
  if (testSensors.readSensors()) {
    SensorData data = testSensors.getCurrentData();
    DateTime currentTime = data.currentTime;
    
    Serial.print(F("Current Date/Time: "));
    Serial.print(currentTime.year(), DEC);
    Serial.print('/');
    Serial.print(currentTime.month(), DEC);
    Serial.print('/');
    Serial.print(currentTime.day(), DEC);
    Serial.print(" ");
    Serial.print(currentTime.hour(), DEC);
    Serial.print(':');
    if (currentTime.minute() < 10) Serial.print('0');
    Serial.print(currentTime.minute(), DEC);
    Serial.print(':');
    if (currentTime.second() < 10) Serial.print('0');
    Serial.println(currentTime.second(), DEC);
  } else {
    Serial.println(F("✗ Failed to read sensor data"));
  }
  
  // Test setting a fixed time (July 21, 2025, 12:30:45)
  Serial.println(F("\nSetting test time: 2025-07-21 12:30:45"));
  DateTime testTime(2025, 7, 21, 12, 30, 45);
  if (testSensors.setDateTime(testTime)) {
    Serial.println(F("✓ Time set successfully"));
  } else {
    Serial.println(F("✗ Failed to set time"));
  }
  
  delay(2000);
  
  // Read back the set time
  Serial.println(F("Reading back set time..."));
  if (testSensors.readSensors()) {
    SensorData data = testSensors.getCurrentData();
    DateTime readBackTime = data.currentTime;
    
    Serial.print(F("Read Back Time: "));
    Serial.print(readBackTime.year(), DEC);
    Serial.print('/');
    Serial.print(readBackTime.month(), DEC);
    Serial.print('/');
    Serial.print(readBackTime.day(), DEC);
    Serial.print(" ");
    Serial.print(readBackTime.hour(), DEC);
    Serial.print(':');
    if (readBackTime.minute() < 10) Serial.print('0');
    Serial.print(readBackTime.minute(), DEC);
    Serial.print(':');
    if (readBackTime.second() < 10) Serial.print('0');
    Serial.println(readBackTime.second(), DEC);
  }
  
  Serial.println(F("✓ RTC test completed"));
  waitForUserInput();
}

void testTemperatureHumiditySensor() {
  printTestHeader("AHT21 TEMPERATURE/HUMIDITY SENSOR TEST");
  
  Serial.println(F("Initializing AHT21 sensor..."));
  if (testSensors.init()) {
    Serial.println(F("✓ AHT21 sensor initialization successful"));
  } else {
    Serial.println(F("✗ AHT21 sensor initialization failed"));
    waitForUserInput();
    return;
  }
  
  Serial.println(F("\nReading temperature and humidity (5 samples)..."));
  for (int i = 0; i < 5; i++) {
    Serial.print(F("Sample "));
    Serial.print(i + 1);
    Serial.print(F(": "));
    
    if (testSensors.readSensors()) {
      SensorData data = testSensors.getCurrentData();
      
      Serial.print(F("Temperature: "));
      Serial.print(data.temperature, 2);
      Serial.print(F("°C ("));
      Serial.print(data.temperatureF, 2);
      Serial.print(F("°F), Humidity: "));
      Serial.print(data.humidity, 2);
      Serial.print(F("%, Feels Like: "));
      Serial.print(data.feelsLikeF, 2);
      Serial.print(F("°F, Word: "));
      Serial.println(data.tempWord);
    } else {
      Serial.println(F("Failed to read sensor"));
    }
    
    delay(2000);
  }
  
  Serial.println(F("✓ Temperature/Humidity sensor test completed"));
  waitForUserInput();
}

void testLightSensor() {
  printTestHeader("BH1750 LIGHT SENSOR TEST");
  
  Serial.println(F("Initializing BH1750 light sensor..."));
  if (testSensors.init()) {
    Serial.println(F("✓ BH1750 sensor initialization successful"));
  } else {
    Serial.println(F("✗ BH1750 sensor initialization failed"));
    waitForUserInput();
    return;
  }
  
  Serial.println(F("\nReading light levels (10 samples)..."));
  Serial.println(F("Try covering and uncovering the sensor to see changes"));
  delay(2000);
  
  for (int i = 0; i < 10; i++) {
    if (testSensors.readSensors()) {
      SensorData data = testSensors.getCurrentData();
      
      Serial.print(F("Sample "));
      Serial.print(i + 1);
      Serial.print(F(": Light Level: "));
      Serial.print(data.lightLevel, 2);
      Serial.print(F(" lux"));
      
      // Provide context for light levels
      if (data.lightLevel < 10) {
        Serial.println(F(" (Dark)"));
      } else if (data.lightLevel < 200) {
        Serial.println(F(" (Dim)"));
      } else if (data.lightLevel < 1000) {
        Serial.println(F(" (Indoor lighting)"));
      } else {
        Serial.println(F(" (Bright)"));
      }
    } else {
      Serial.println(F("Failed to read sensor"));
    }
    
    delay(1000);
  }
  
  Serial.println(F("✓ Light sensor test completed"));
  waitForUserInput();
}

void testPressureSensor() {
  printTestHeader("DFROBOT BMP280 PRESSURE SENSOR TEST");
  
  Serial.println(F("Initializing DFRobot BMP280 pressure sensor..."));
  if (testSensors.init()) {
    Serial.println(F("✓ BMP280 sensor initialization successful"));
  } else {
    Serial.println(F("✗ BMP280 sensor initialization failed"));
    waitForUserInput();
    return;
  }
  
  Serial.println(F("\nReading pressure data (5 samples)..."));
  for (int i = 0; i < 5; i++) {
    if (testSensors.readSensors()) {
      SensorData data = testSensors.getCurrentData();
      
      Serial.print(F("Sample "));
      Serial.print(i + 1);
      Serial.print(F(": Pressure: "));
      Serial.print(data.pressure, 2);
      Serial.println(F(" hPa"));
    } else {
      Serial.println(F("Failed to read sensor"));
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

void populateTestSensorData() {
  // Fill test data with reasonable values
  testData.currentTime = DateTime(2025, 7, 21, 12, 30, 45);
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
