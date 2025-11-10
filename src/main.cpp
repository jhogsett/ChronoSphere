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

// Alert cooldown tracking (replaces lightingEffects.isAlertActive() check)
unsigned long lastAlertTime = 0;
const unsigned long ALERT_COOLDOWN_MS = 600000; // 10 minutes (same as old NeoPixel alert duration)

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
  
  // Motor control removed - servo and stepper motor features deprecated
  // if (!motorControl.init()) {
  //   Serial.println(F("ERROR: Motor control initialization failed"));
  //   initSuccess = false;
  // }
  
  if (!audioManager.init()) {
    Serial.println(F("ERROR: Audio initialization failed"));
    initSuccess = false;
  }
  
  if (!dataLogger.init()) {
    Serial.println(F("ERROR: Data logger initialization failed"));
    initSuccess = false;
  }
  
  // Lighting effects removed - NeoPixel control deprecated (future clock display will handle LEDs)
  // if (!lightingEffects.init()) {
  //   Serial.println(F("ERROR: Lighting effects initialization failed"));
  //   initSuccess = false;
  // }
  
  if (initSuccess) {
    Serial.println(F("All modules initialized successfully"));
    displayManager.showStartupMessage();
    // lightingEffects.showStartupSequence();  // DEPRECATED - NeoPixel removed
    
    delay(2000); // Show startup message
    
    // Read sensors once at startup to initialize data
    if (sensors.readSensors()) {
      // Sensor initialization successful - now play startup chime with current hour
      DateTime currentTime = sensors.getCurrentTime();
      uint8_t currentHour = currentTime.getHour();
      
      // Play startup chime with current hour chimes for testing
      audioManager.playStartupChime(currentHour);
    } else {
      Serial.println(F("WARNING: Initial sensor read failed"));
      // Play basic startup chime if sensor read fails
      audioManager.playStartupChime();
    }
    
    // Set initial display mode to ensure synchronization
    displayManager.setMode(currentDisplayMode);
  } else {
    Serial.println(F("FATAL: System initialization failed"));
    displayManager.showError("INIT");
    // lightingEffects.showErrorPattern();  // DEPRECATED - NeoPixel removed
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
  
  // DEBUG: Check initial currentData lightLevel
  // Serial.print(F("DEBUG: Initial currentData.lightLevel: "));
  // Serial.println(currentData.lightLevel);
  
  // Read sensors when needed and save real data
  SensorData realData = currentData;  // Keep a copy of real sensor data
  if (sensors.isTimeToRead()) {
    if (sensors.readSensors()) {
      realData = sensors.getCurrentData();  // Get fresh real data
      
      // Update data logger
      dataLogger.update(realData);
      
      // NeoPixel updates removed - LED control deprecated
      // lightingEffects.update(realData);
      
      // Adjust lighting based on ambient light
      // lightingEffects.adjustBrightnessForAmbientLight(realData.lightLevel);
      displayManager.adjustBrightnessForAmbientLight(realData.lightLevel);
      
      // Check for alerts
      checkWeatherAlerts();
      
    } else {
      Serial.println(F("WARNING: Sensor read failed"));
    }
  }
  
  // // DEBUGGING: Using ALL dummy data to test display stability
  // currentData.temperatureF = 79.5;                   // DUMMY
  // currentData.feelsLikeF = 77.8;                     // DUMMY
  // strcpy(currentData.tempWord, "WARM");              // DUMMY
  // currentData.humidity = 45.0;                       // DUMMY
  // currentData.pressure = 1013.0;                     // DUMMY
  // currentData.lightLevel = 150.0;                    // DUMMY

  // currentData.temperatureF = realData.temperatureF;      
  // currentData.feelsLikeF = realData.feelsLikeF;        
  // strcpy(currentData.tempWord, realData.tempWord); 
  // currentData.humidity = realData.humidity;          
  // currentData.pressure = realData.pressure;        
  // currentData.lightLevel = realData.lightLevel;
  // currentData.currentTime = realData.currentTime;  // IMPORTANT: Restore the real time!

  // DEBUG: Check final currentData lightLevel after restoration
  // Serial.print(F("DEBUG: Final currentData.lightLevel: "));
  // Serial.println(currentData.lightLevel);       

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
    if (settingsMode) {
      displayManager.updateSettings(currentData, settingsMode, currentSetting, 
                                    settingTimeComponent, settingDateComponent, pendingDateTime, editingSettingValue);
    } else {
      displayManager.update(currentData);
    }
  }
  
  // Update continuous systems
  audioManager.update(); // Handle chime timing and playback
  
  // Check for chimes
  audioManager.checkAndPlayChime(sensors.getCurrentTime());
}

void handleUserInput() {
  int encoderDelta = userInput.getEncoderDelta();
  ButtonState buttonState = userInput.getButtonState();
  
  // Handle button presses - only process BUTTON_PRESSED once per physical press
  static ButtonState lastButtonState = BUTTON_IDLE;
  bool buttonJustPressed = (buttonState == BUTTON_PRESSED && lastButtonState != BUTTON_PRESSED);
  bool buttonJustHeld = (buttonState == BUTTON_HELD && lastButtonState != BUTTON_HELD);
  bool buttonJustReleased = (buttonState == BUTTON_RELEASED && lastButtonState != BUTTON_RELEASED);
  lastButtonState = buttonState;
  
  // Reset edge detection after button release to prepare for next press
  if (buttonJustReleased) {
    // Button released
  }
  
  if (buttonJustPressed) {
    if (!settingsMode) {
      // Enter settings mode (settings menu)
      settingsMode = true;
      editingSettingValue = false;  // Start in menu, not editing
      currentSetting = SETTING_TIME;
      settingTimeComponent = 0;
      settingDateComponent = 0;
      hasDateTimeChanges = false;
    } else if (!editingSettingValue) {
      // In settings menu - enter the selected setting to edit it OR exit
      if (currentSetting == SETTING_EXIT) {
        // EXIT option selected - leave settings mode
        
        // Save any pending time/date changes before exiting
        if (hasDateTimeChanges) {
          sensors.setDateTime(pendingDateTime);
          
          // Force immediate sensor read to update display with new time
          sensors.readSensors();
          
          hasDateTimeChanges = false;
        }
        
        settingsMode = false;
        editingSettingValue = false;
      } else {
        // Regular setting selected - enter editing mode
        editingSettingValue = true;
        
        // Initialize components for the selected setting
        if (currentSetting == SETTING_TIME) {
          settingTimeComponent = 0;  // Start with hour
        } else if (currentSetting == SETTING_DATE) {
          settingDateComponent = 0;  // Start with month
        }
      }
    } else {
      // In setting edit mode - cycle through components
      if (currentSetting == SETTING_TIME) {
        settingTimeComponent = (settingTimeComponent + 1) % 3;
      } else if (currentSetting == SETTING_DATE) {
        settingDateComponent = (settingDateComponent + 1) % 3;
      }
    }
  }
  
  // Handle long button press to go back/exit from editing mode only
  static bool wasHeld = false;
  
  if (buttonState == BUTTON_HELD) {
    wasHeld = true;
  }
  
  if (buttonJustReleased && wasHeld) {
    wasHeld = false; // Reset the flag
    
    if (settingsMode && editingSettingValue) {
      // Exit from editing back to settings menu (long press only works when editing)
      if (hasDateTimeChanges) {
        sensors.setDateTime(pendingDateTime);
        
        // Force immediate sensor read to update display with new time
        sensors.readSensors();
        
        hasDateTimeChanges = false;
      }
      editingSettingValue = false;
    }
  }
  
  // Handle encoder rotation
  if (encoderDelta != 0) {
    if (settingsMode && !editingSettingValue) {
      // In settings menu - navigate through settings list
      int newSetting = (int)currentSetting + encoderDelta;
      if (newSetting < 0) newSetting = SETTING_EXIT;  // Wrap to EXIT (last setting)
      if (newSetting > SETTING_EXIT) newSetting = 0;   // Wrap to first setting
      currentSetting = (SettingItem)newSetting;
    } else if (settingsMode && editingSettingValue) {
      // In setting edit mode - adjust the setting value
      handleSettingChange(encoderDelta);
    } else {
      // Normal display mode - change display mode
      DisplayMode newMode = userInput.handleModeChange(currentDisplayMode, encoderDelta);
      if (newMode != currentDisplayMode) {
        currentDisplayMode = newMode;
        displayManager.setMode(currentDisplayMode);
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
            }
            break;
        }
        
        // // Show current time setting (minimal output)
        // Serial.print(F("Time: "));
        // Serial.print(pendingDateTime.getHour());
        // Serial.print(F(":"));
        // if (pendingDateTime.getMinute() < 10) Serial.print(F("0"));
        // Serial.print(pendingDateTime.getMinute());
        // Serial.print(F(":"));
        // if (pendingDateTime.getSecond() < 10) Serial.print(F("0"));
        // Serial.println(pendingDateTime.getSecond());
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
            }
            break;
        }
        
        // // Show current date setting
        // Serial.print(F("Date: "));
        // Serial.print(pendingDateTime.getMonth());
        // Serial.print(F("/"));
        // Serial.print(pendingDateTime.getDay());
        // Serial.print(F("/"));
        // Serial.println(pendingDateTime.getYear());
      }
      break;
      
    case SETTING_CHIME_TYPE:
      Serial.println(F("Adjusting chime type not implemented"));
      break;
      
    case SETTING_CHIME_INSTRUMENT:
      Serial.println(F("Adjusting chime instrument not implemented"));
      break;
      
    case SETTING_CHIME_FREQUENCY:
      Serial.println(F("Adjusting chime frequency not implemented"));
      break;
      
    default:
      Serial.println(F("Setting not implemented not implemented"));
      break;
  }
}

void checkWeatherAlerts() {
  // Don't trigger new alerts if we're in the cooldown period
  unsigned long currentMillis = millis();
  if (currentMillis - lastAlertTime < ALERT_COOLDOWN_MS) {
    return;  // Still in cooldown from previous alert
  }
  
  // Check for rapid weather changes and trigger alerts
  // Note: Priority order - pressure alerts override temperature alerts, 
  // which override rapid change alerts (only one alert active at a time)
  bool alertTriggered = false;
  
  if (dataLogger.checkPressureAlert()) {
    audioManager.playPressureAlert();
    displayManager.showAlert(ALERT_PRESSURE);
    alertTriggered = true;
  }
  else if (dataLogger.checkTemperatureAlert()) {
    audioManager.playTemperatureAlert();
    displayManager.showAlert(ALERT_TEMPERATURE);
    alertTriggered = true;
  }
  else if (dataLogger.checkRapidChange()) {
    audioManager.playWeatherAlert();
    displayManager.showAlert(ALERT_RAPID_CHANGE);
    alertTriggered = true;
  }
  
  // Update cooldown timer if an alert was triggered
  if (alertTriggered) {
    lastAlertTime = currentMillis;
  }
}
