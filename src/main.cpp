#include <Arduino.h>
#include <Wire.h>
#include "Config.h"
#include "Sensors.h"
#include "DisplayManager.h"
#include "UserInput.h"
#include "MotorControl.h"
#include "AudioManager.h"
#include "DataLogger.h"
#include "LightingEffects.h"

// Global objects
Sensors sensors;
DisplayManager displayManager;
UserInput userInput;
MotorControl motorControl;
AudioManager audioManager;
DataLogger dataLogger;
LightingEffects lightingEffects;

// System state
DisplayMode currentDisplayMode = MODE_ROLLING_CURRENT;  // Start in rolling mode for debugging
bool settingsMode = false;
SettingItem currentSetting = SETTING_TIME;

// Timing variables
unsigned long lastMainLoop = 0;
const unsigned long MAIN_LOOP_INTERVAL = 50; // 20Hz main loop

// Forward declarations
void handleUserInput();
void handleSettingChange(int delta);
void checkWeatherAlerts();

void setup() {
  Serial.begin(115200);
  Serial.println(F("Weather Clock Starting..."));
  
  // Initialize I2C
  Wire.begin();
  
  // Initialize all modules
  bool initSuccess = true;
  
  if (!sensors.init()) {
    Serial.println(F("ERROR: Sensors initialization failed"));
    initSuccess = false;
  }
  
  if (!displayManager.init()) {
    Serial.println(F("ERROR: Display initialization failed"));
    initSuccess = false;
  }
  
  if (!userInput.init()) {
    Serial.println(F("ERROR: User input initialization failed"));
    initSuccess = false;
  }
  
  if (!motorControl.init()) {
    Serial.println(F("ERROR: Motor control initialization failed"));
    initSuccess = false;
  }
  
  if (!audioManager.init()) {
    Serial.println(F("ERROR: Audio initialization failed"));
    initSuccess = false;
  }
  
  if (!dataLogger.init()) {
    Serial.println(F("ERROR: Data logger initialization failed"));
    initSuccess = false;
  }
  
  if (!lightingEffects.init()) {
    Serial.println(F("ERROR: Lighting effects initialization failed"));
    initSuccess = false;
  }
  
  if (initSuccess) {
    Serial.println(F("All modules initialized successfully"));
    displayManager.showStartupMessage();
    lightingEffects.showStartupSequence();
    
    // Calibrate motors
    motorControl.calibrateMinuteHand();
    motorControl.calibratePendulum();
    
    delay(2000); // Show startup message
    
    // Read sensors once at startup to initialize data
    Serial.println(F("Reading initial sensor data..."));
    if (sensors.readSensors()) {
      Serial.println(F("Initial sensor read successful"));
      SensorData initialData = sensors.getCurrentData();
      Serial.print(F("Initial temp: "));
      Serial.print(initialData.temperatureF);
      Serial.println(F("F"));
    } else {
      Serial.println(F("WARNING: Initial sensor read failed"));
    }
    
    // Set initial display mode to ensure synchronization
    displayManager.setMode(currentDisplayMode);
  } else {
    Serial.println(F("FATAL: System initialization failed"));
    displayManager.showError("INIT");
    lightingEffects.showErrorPattern();
    while(1) delay(1000); // Halt system
  }
  
  Serial.println(F("Weather Clock Ready"));
}

void loop() {
  unsigned long currentTime = millis();
  
  // Main loop timing control
  if (currentTime - lastMainLoop < MAIN_LOOP_INTERVAL) {
    return;
  }
  lastMainLoop = currentTime;
  
  // Update all input sources
  userInput.update();
  
  // Handle user input
  handleUserInput();
  
  // Always get current sensor data (or use last good reading)
  SensorData currentData = sensors.getCurrentData();
  
  // Read sensors when needed and save real data
  SensorData realData = currentData;  // Keep a copy of real sensor data
  if (sensors.isTimeToRead()) {
    if (sensors.readSensors()) {
      Serial.println(F("Sensors updated"));
      realData = sensors.getCurrentData();  // Get fresh real data
      
      // Debug: Print real sensor data
      Serial.print(F("REAL Temperature: "));
      Serial.print(realData.temperature);
      Serial.print(F("C, "));
      Serial.print(realData.temperatureF);
      Serial.println(F("F"));
      Serial.print(F("REAL Feels like: "));
      Serial.print(realData.feelsLikeF);
      Serial.println(F("F"));
      Serial.print(F("REAL Humidity: "));
      Serial.print(realData.humidity);
      Serial.println(F("%"));
      Serial.print(F("REAL Pressure: "));
      Serial.print(realData.pressure);
      Serial.println(F(" hPa"));
      Serial.print(F("REAL Light: "));
      Serial.print(realData.lightLevel);
      Serial.println(F(" lux"));
      Serial.print(F("REAL Temp word: '"));
      Serial.print(realData.tempWord);
      Serial.println(F("'"));
      
      // Update data logger
      dataLogger.update(realData);
      
      // Update lighting effects
      lightingEffects.update(realData);
      
      // Adjust lighting based on ambient light
      lightingEffects.adjustBrightnessForAmbientLight(realData.lightLevel);
      displayManager.adjustBrightnessForAmbientLight(realData.lightLevel);
      
      // Check for alerts
      checkWeatherAlerts();
      
    } else {
      Serial.println(F("WARNING: Sensor read failed"));
    }
  }
  
  // DEBUGGING: Start with dummy data, then replace ONE real value at a time
  currentData.temperatureF = realData.temperatureF;  // STEP 1: REAL TEMPERATURE ✅ WORKS
  currentData.feelsLikeF = realData.feelsLikeF;      // STEP 3: REAL FEELS LIKE ✅ WORKS
  strcpy(currentData.tempWord, realData.tempWord);   // STEP 2: REAL TEMP WORD ✅ WORKS
  currentData.humidity = 45.0;                       // DUMMY - REAL HUMIDITY BREAKS DISPLAY!
  currentData.pressure = 1013.0;                     // DUMMY
  currentData.lightLevel = 150.0;                    // DUMMY
  
  Serial.print(F("DISPLAY DATA - Temp: "));
  Serial.print(currentData.temperatureF);
  Serial.print(F("F, FeelsLike: "));
  Serial.print(currentData.feelsLikeF);
  Serial.print(F("F, Word: "));
  Serial.println(currentData.tempWord);
  
  // Update display every cycle (like hardware test does)
  if (displayManager.isTimeToUpdate()) {
    Serial.print(F("Updating display, mode: "));
    Serial.println((int)currentDisplayMode);
    displayManager.update(currentData);
  }
  
  // Update motors
  DateTime now = currentData.currentTime;
  motorControl.updateMinuteHand(now.getMinute(), now.getSecond());
  
  // Update continuous systems
  motorControl.update(); // Update pendulum and stepper motor
  audioManager.update(); // Handle chime timing and playbook
  
  // Check for chimes
  audioManager.checkAndPlayChime(sensors.getCurrentTime());
}

void handleUserInput() {
  int encoderDelta = userInput.getEncoderDelta();
  ButtonState buttonState = userInput.getButtonState();
  
  // Handle button presses
  if (buttonState == BUTTON_PRESSED) {
    if (!settingsMode) {
      // Enter settings mode
      settingsMode = true;
      currentSetting = SETTING_TIME;
      Serial.println(F("Entering settings mode"));
    } else {
      // Move to next setting or exit settings
      int nextSetting = (int)currentSetting + 1;
      if (nextSetting >= SETTING_MOTOR_SETTINGS + 1) {
        settingsMode = false;
        Serial.println(F("Exiting settings mode"));
      } else {
        currentSetting = (SettingItem)nextSetting;
      }
    }
  }
  
  // Handle encoder rotation
  if (encoderDelta != 0) {
    if (settingsMode) {
      handleSettingChange(encoderDelta);
    } else {
      // Change display mode
      DisplayMode newMode = userInput.handleModeChange(currentDisplayMode, encoderDelta);
      if (newMode != currentDisplayMode) {
        currentDisplayMode = newMode;
        displayManager.setMode(currentDisplayMode);
        Serial.print(F("Display mode changed to: "));
        Serial.println((int)currentDisplayMode);
      }
    }
  }
}

void handleSettingChange(int delta) {
  // This is a placeholder - you would implement specific setting changes
  // based on the currentSetting and delta values
  
  switch (currentSetting) {
    case SETTING_TIME:
      // Adjust time
      Serial.println(F("Adjusting time setting"));
      break;
      
    case SETTING_DATE:
      // Adjust date
      Serial.println(F("Adjusting date setting"));
      break;
      
    case SETTING_CHIME_TYPE:
      // Change chime type
      Serial.println(F("Adjusting chime type"));
      break;
      
    case SETTING_CHIME_INSTRUMENT:
      // Change MIDI instrument
      Serial.println(F("Adjusting chime instrument"));
      break;
      
    case SETTING_CHIME_FREQUENCY:
      // Change chime frequency
      Serial.println(F("Adjusting chime frequency"));
      break;
      
    default:
      Serial.println(F("Setting not implemented"));
      break;
  }
}

void checkWeatherAlerts() {
  // Check for rapid weather changes and trigger alerts
  if (dataLogger.checkPressureAlert()) {
    audioManager.playPressureAlert();
    lightingEffects.flashAlert({255, 255, 0}, 3); // Yellow flash
  }
  
  if (dataLogger.checkTemperatureAlert()) {
    audioManager.playTemperatureAlert();
    lightingEffects.flashAlert({255, 0, 0}, 3); // Red flash
  }
  
  if (dataLogger.checkRapidChange()) {
    audioManager.playWeatherAlert();
    lightingEffects.flashAlert({0, 255, 255}, 3); // Cyan flash
  }
}
