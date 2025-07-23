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
bool editingSettingValue = false;  // New: track if we're in a setting or editing its value
SettingItem currentSetting = SETTING_TIME;

// Time/Date setting state
int settingTimeComponent = 0;  // 0=hour, 1=minute, 2=second
int settingDateComponent = 0;  // 0=month, 1=day, 2=year
DateTime pendingDateTime;      // Working copy for time/date changes
bool hasDateTimeChanges = false;

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
  
  // DEBUGGING: Using ALL dummy data to test display stability
  currentData.temperatureF = 79.5;                   // DUMMY
  currentData.feelsLikeF = 77.8;                     // DUMMY
  strcpy(currentData.tempWord, "WARM");              // DUMMY
  currentData.humidity = 45.0;                       // DUMMY
  currentData.pressure = 1013.0;                     // DUMMY
  currentData.lightLevel = 150.0;                    // DUMMY

  currentData.temperatureF = realData.temperatureF;      
  currentData.feelsLikeF = realData.feelsLikeF;        
  strcpy(currentData.tempWord, realData.tempWord); 
  currentData.humidity = realData.humidity;          
  currentData.pressure = realData.pressure;        
  currentData.lightLevel = realData.lightLevel;
  currentData.currentTime = realData.currentTime;  // IMPORTANT: Restore the real time!       

  // Serial.print(F("DISPLAY DATA - ALL DUMMY: Temp: "));
  // Serial.print(currentData.temperatureF);
  // Serial.print(F("F, FeelsLike: "));
  // Serial.print(currentData.feelsLikeF);
  // Serial.print(F("F, Word: "));
  // Serial.print(currentData.tempWord);
  // Serial.print(F(", Humidity: "));
  // Serial.print(currentData.humidity);
  // Serial.print(F("%, Pressure: "));
  // Serial.print(currentData.pressure);
  // Serial.print(F(" hPa, Light: "));
  // Serial.print(currentData.lightLevel);
  // Serial.println(F(" lux"));
  
  // Update display every cycle (like hardware test does)
  if (displayManager.isTimeToUpdate()) {
    Serial.print(F("Updating display, mode: "));
    Serial.print((int)currentDisplayMode);
    Serial.print(F(", settingsMode: "));
    Serial.print(settingsMode);
    Serial.print(F(", editingSettingValue: "));
    Serial.println(editingSettingValue);
    
    if (settingsMode) {
      displayManager.updateSettings(currentData, settingsMode, currentSetting, 
                                    settingTimeComponent, settingDateComponent, pendingDateTime, editingSettingValue);
    } else {
      displayManager.update(currentData);
    }
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
  
  // Debug output for button state
  if (buttonState != BUTTON_IDLE) {
    Serial.print(F("Button state: "));
    Serial.print(buttonState);
    Serial.print(F(" (IDLE=0, PRESSED=1, HELD=2, RELEASED=3)"));
    Serial.print(F(", Settings mode: "));
    Serial.print(F(", editingSettingValue: "));
    Serial.println(editingSettingValue);
  }
  
  // Handle button presses - only process BUTTON_PRESSED once per physical press
  static ButtonState lastButtonState = BUTTON_IDLE;
  bool buttonJustPressed = (buttonState == BUTTON_PRESSED && lastButtonState != BUTTON_PRESSED);
  bool buttonJustHeld = (buttonState == BUTTON_HELD && lastButtonState != BUTTON_HELD);
  bool buttonJustReleased = (buttonState == BUTTON_RELEASED && lastButtonState != BUTTON_RELEASED);
  lastButtonState = buttonState;
  
  // Reset edge detection after button release to prepare for next press
  if (buttonJustReleased) {
    Serial.println(F("Button released - resetting for next press"));
  }
  
  if (buttonJustPressed) {
    Serial.println(F("BUTTON_PRESSED detected (edge)"));
    
    if (!settingsMode) {
      // Enter settings mode (settings menu)
      Serial.println(F("Entering settings menu..."));
      settingsMode = true;
      editingSettingValue = false;  // Start in menu, not editing
      currentSetting = SETTING_TIME;
      settingTimeComponent = 0;
      settingDateComponent = 0;
      hasDateTimeChanges = false;
      Serial.println(F("Settings menu entered - use encoder to select setting"));
    } else if (!editingSettingValue) {
      // In settings menu - enter the selected setting to edit it OR exit
      if (currentSetting == SETTING_EXIT) {
        // EXIT option selected - leave settings mode
        Serial.println(F("EXIT selected - leaving settings mode"));
        
        // Save any pending time/date changes before exiting
        if (hasDateTimeChanges) {
          sensors.setDateTime(pendingDateTime);
          Serial.println(F("Final time/date saved to RTC on exit"));
          
          // Force immediate sensor read to update display with new time
          Serial.println(F("Forcing sensor read to update display time..."));
          sensors.readSensors();
          
          hasDateTimeChanges = false;
        }
        
        settingsMode = false;
        editingSettingValue = false;
      } else {
        // Regular setting selected - enter editing mode
        Serial.print(F("Entering setting for editing: "));
        Serial.println((int)currentSetting);
        editingSettingValue = true;
        
        // Initialize components for the selected setting
        if (currentSetting == SETTING_TIME) {
          settingTimeComponent = 0;  // Start with hour
          Serial.println(F("Now editing TIME - HOUR"));
        } else if (currentSetting == SETTING_DATE) {
          settingDateComponent = 0;  // Start with month
          Serial.println(F("Now editing DATE - MONTH"));
        }
      }
    } else {
      // In setting edit mode - cycle through components
      if (currentSetting == SETTING_TIME) {
        Serial.print(F("TIME: settingTimeComponent before: "));
        Serial.println(settingTimeComponent);
        settingTimeComponent = (settingTimeComponent + 1) % 3;
        Serial.print(F("TIME: settingTimeComponent after: "));
        Serial.println(settingTimeComponent);
        Serial.print(F("TIME: Now editing "));
        Serial.println(settingTimeComponent == 0 ? "HOUR" : settingTimeComponent == 1 ? "MINUTE" : "SECOND");
      } else if (currentSetting == SETTING_DATE) {
        settingDateComponent = (settingDateComponent + 1) % 3;
        Serial.print(F("DATE: Now editing "));
        Serial.println(settingDateComponent == 0 ? "MONTH" : settingDateComponent == 1 ? "DAY" : "YEAR");
      } else {
        Serial.println(F("Cycling through setting options"));
      }
    }
  }
  
  // Handle long button press to go back/exit from editing mode only
  static bool wasHeld = false;
  
  if (buttonState == BUTTON_HELD) {
    wasHeld = true;
  }
  
  if (buttonJustReleased && wasHeld) {
    Serial.println(F("LONG PRESS RELEASED - Processing exit"));
    wasHeld = false; // Reset the flag
    
    if (settingsMode && editingSettingValue) {
      // Exit from editing back to settings menu (long press only works when editing)
      Serial.println(F("LONG PRESS: Exiting to settings menu"));
      if (hasDateTimeChanges) {
        sensors.setDateTime(pendingDateTime);
        Serial.println(F("Time/date saved to RTC"));
        
        // Force immediate sensor read to update display with new time
        Serial.println(F("Forcing sensor read to update display time..."));
        sensors.readSensors();
        
        hasDateTimeChanges = false;
      }
      editingSettingValue = false;
    }
  }
  
  // Handle encoder rotation
  if (encoderDelta != 0) {
    Serial.print(F("Encoder delta: "));
    Serial.print(encoderDelta);
    Serial.print(F(", Settings mode: "));
    Serial.print(settingsMode);
    Serial.print(F(", Editing: "));
    Serial.println(editingSettingValue);
    
    if (settingsMode && !editingSettingValue) {
      // In settings menu - navigate through settings list
      int newSetting = (int)currentSetting + encoderDelta;
      if (newSetting < 0) newSetting = SETTING_EXIT;  // Wrap to EXIT (last setting)
      if (newSetting > SETTING_EXIT) newSetting = 0;   // Wrap to first setting
      currentSetting = (SettingItem)newSetting;
      
      Serial.print(F("Settings menu: Selected setting "));
      Serial.println((int)currentSetting);
    } else if (settingsMode && editingSettingValue) {
      // In setting edit mode - adjust the setting value
      handleSettingChange(encoderDelta);
    } else {
      // Normal display mode - change display mode
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
  switch (currentSetting) {
    case SETTING_TIME:
      {
        // Initialize pending time if first edit
        if (!hasDateTimeChanges) {
          pendingDateTime = sensors.getCurrentTime();
          hasDateTimeChanges = true;
        }
        
        // Adjust current time component
        switch (settingTimeComponent) {
          case 0: // Hour
            {
              int hour = pendingDateTime.getHour() + delta;
              if (hour < 0) hour = 23;
              if (hour > 23) hour = 0;
              pendingDateTime = DateTime(pendingDateTime.getYear(), pendingDateTime.getMonth(), 
                                       pendingDateTime.getDay(), hour, pendingDateTime.getMinute(), 
                                       pendingDateTime.getSecond());
              Serial.print(F("Hour set to: "));
              Serial.println(hour);
            }
            break;
          case 1: // Minute
            {
              int minute = pendingDateTime.getMinute() + delta;
              if (minute < 0) minute = 59;
              if (minute > 59) minute = 0;
              pendingDateTime = DateTime(pendingDateTime.getYear(), pendingDateTime.getMonth(), 
                                       pendingDateTime.getDay(), pendingDateTime.getHour(), minute, 
                                       pendingDateTime.getSecond());
              Serial.print(F("Minute set to: "));
              Serial.println(minute);
            }
            break;
          case 2: // Second
            {
              int second = pendingDateTime.getSecond() + delta;
              if (second < 0) second = 59;
              if (second > 59) second = 0;
              pendingDateTime = DateTime(pendingDateTime.getYear(), pendingDateTime.getMonth(), 
                                       pendingDateTime.getDay(), pendingDateTime.getHour(), 
                                       pendingDateTime.getMinute(), second);
              Serial.print(F("Second set to: "));
              Serial.println(second);
            }
            break;
        }
        
        // Show current time setting
        Serial.print(F("Setting time: "));
        Serial.print(pendingDateTime.getHour());
        Serial.print(F(":"));
        if (pendingDateTime.getMinute() < 10) Serial.print(F("0"));
        Serial.print(pendingDateTime.getMinute());
        Serial.print(F(":"));
        if (pendingDateTime.getSecond() < 10) Serial.print(F("0"));
        Serial.print(pendingDateTime.getSecond());
        Serial.print(F(" (editing "));
        Serial.print(settingTimeComponent == 0 ? "HOUR" : settingTimeComponent == 1 ? "MINUTE" : "SECOND");
        Serial.println(F(")"));
      }
      break;
      
    case SETTING_DATE:
      {
        // Initialize pending date if first edit
        if (!hasDateTimeChanges) {
          pendingDateTime = sensors.getCurrentTime();
          hasDateTimeChanges = true;
        }
        
        // Adjust current date component
        switch (settingDateComponent) {
          case 0: // Month
            {
              int month = pendingDateTime.getMonth() + delta;
              if (month < 1) month = 12;
              if (month > 12) month = 1;
              pendingDateTime = DateTime(pendingDateTime.getYear(), month, pendingDateTime.getDay(), 
                                       pendingDateTime.getHour(), pendingDateTime.getMinute(), 
                                       pendingDateTime.getSecond());
              Serial.print(F("Month set to: "));
              Serial.println(month);
            }
            break;
          case 1: // Day
            {
              int day = pendingDateTime.getDay() + delta;
              if (day < 1) day = 31;  // Simplified - doesn't account for different month lengths
              if (day > 31) day = 1;
              pendingDateTime = DateTime(pendingDateTime.getYear(), pendingDateTime.getMonth(), day, 
                                       pendingDateTime.getHour(), pendingDateTime.getMinute(), 
                                       pendingDateTime.getSecond());
              Serial.print(F("Day set to: "));
              Serial.println(day);
            }
            break;
          case 2: // Year
            {
              int year = pendingDateTime.getYear() + delta;
              if (year < 2020) year = 2020;
              if (year > 2099) year = 2099;
              pendingDateTime = DateTime(year, pendingDateTime.getMonth(), pendingDateTime.getDay(), 
                                       pendingDateTime.getHour(), pendingDateTime.getMinute(), 
                                       pendingDateTime.getSecond());
              Serial.print(F("Year set to: "));
              Serial.println(year);
            }
            break;
        }
        
        // Show current date setting
        Serial.print(F("Setting date: "));
        Serial.print(pendingDateTime.getMonth());
        Serial.print(F("/"));
        Serial.print(pendingDateTime.getDay());
        Serial.print(F("/"));
        Serial.print(pendingDateTime.getYear());
        Serial.print(F(" (editing "));
        Serial.print(settingDateComponent == 0 ? "MONTH" : settingDateComponent == 1 ? "DAY" : "YEAR");
        Serial.println(F(")"));
      }
      break;
      
    case SETTING_CHIME_TYPE:
      Serial.println(F("Adjusting chime type"));
      break;
      
    case SETTING_CHIME_INSTRUMENT:
      Serial.println(F("Adjusting chime instrument"));
      break;
      
    case SETTING_CHIME_FREQUENCY:
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
