#include <Arduino.h>
#include "DisplayManager.h"
#include <HT16K33Disp.h>
#include <stdio.h>
#include <string.h>  // For strcpy

// Helper function for formatting floats for display (from reference\smart_thermo3.ino)
void float_to_fixed(float value, char *buffer, const char *pattern, byte decimals=1){
  if (decimals == 0) {
    // Handle zero decimals case - just format as integer
    int ivalue = int(value + 0.5);  // Round to nearest integer
    sprintf(buffer, pattern, ivalue);
  } else {
    int split = 10 * decimals;
    int ivalue = int(value * split);
    int valuei = ivalue / split;
    int valued = ivalue % split;
    sprintf(buffer, pattern, valuei, valued);
  }
}

bool DisplayManager::init() {
  // Initialize single display group managing all 3 displays
  // Different brightness levels needed due to LED color variations
  byte brightnessLevels[3] = {
    DISPLAY_GREEN_BRIGHTNESS, 
    DISPLAY_AMBER_BRIGHTNESS, 
    DISPLAY_RED_BRIGHTNESS
  }; // Green, Amber, Red
  
  // Create single display object managing 3 displays starting at 0x70
  displayGroup = new HT16K33Disp(DISPLAY_GREEN_ADDRESS, 3);
  displayGroup->init(brightnessLevels);
  displayGroup->clear();
  
  currentMode = MODE_CLOCK;
  lastUpdateTime = 0;
  rollingIndex = 0;
  rollingTimer = 0;
  
  Serial.println(F("Display manager initialized"));
  return true;
}

void DisplayManager::update(SensorData sensorData) {
  switch (currentMode) {
    case MODE_CLOCK:
      displayTime(sensorData.currentTime);
      break;
      
    case MODE_TEMPERATURE:
      displayTemperature(sensorData);
      break;
      
    case MODE_WEATHER_SUMMARY:
      displayWeatherSummary(sensorData);
      break;
      
    case MODE_ROLLING_CURRENT:
      displayRollingCurrent(sensorData);
      break;
      
    case MODE_ROLLING_HISTORICAL:
      displayRollingHistorical();
      break;
      
    case MODE_ROLLING_TRENDS:
      displayRollingTrends();
      break;
      
    case MODE_SETTINGS:
      displaySettings();
      break;
      
    default:
      // Debug: Show unexpected mode
      char modeText[13];
      sprintf(modeText, "MODE ERR %03d", currentMode);
      displayString(modeText);
      break;
  }
  
  lastUpdateTime = millis();
}

void DisplayManager::updateSettings(SensorData sensorData, bool settingsMode, SettingItem currentSetting, 
                                     int settingTimeComponent, int settingDateComponent, DateTime pendingDateTime,
                                     bool editingSettingValue) {
  if (settingsMode) {
    if (editingSettingValue) {
      displaySettingsInterface(currentSetting, settingTimeComponent, settingDateComponent, pendingDateTime);
    } else {
      displaySettingsMenu(currentSetting);
    }
  } else {
    // Normal display update
    update(sensorData);
  }
  lastUpdateTime = millis();
}

bool DisplayManager::isTimeToUpdate() {
  return (millis() - lastUpdateTime >= DISPLAY_UPDATE_INTERVAL);
}

void DisplayManager::clearAllDisplays() {
  displayGroup->clear();
}

void DisplayManager::displayOnModule(uint8_t module, const char* text) {
  // For backward compatibility: position text at specific 4-character segment
  // Module 0 = chars 0-3, Module 1 = chars 4-7, Module 2 = chars 8-11
  char displayText[13] = "            "; // 12 spaces
  
  if (module <= 2) {
    int startPos = module * 4;
    // Copy up to 4 characters to the appropriate position
    for (int i = 0; i < 4 && text[i] != '\0'; i++) {
      displayText[startPos + i] = text[i];
    }
  }
  
  displayGroup->show_string(displayText);
}

void DisplayManager::displayString(const char* text) {
  displayGroup->show_string(text);
}

void DisplayManager::displayScrollingString(const char* text, int showDelay, int scrollDelay) {
  displayGroup->scroll_string(text, showDelay, scrollDelay);
}

void DisplayManager::displayTime(DateTime time) {
  char displayText[13];
  
  // Format time: 4 digits with leading space instead of zero (positions 0-3)
  int hour = time.getHour();
  if (hour == 0) hour = 12;
  if (hour > 12) hour -= 12;
  
  // Format date: month in positions 6-7, day in positions 9-10
  int month = time.getMonth();
  int day = time.getDay();
  
  // Create 12-character string: "HHMM  MM DD  "
  //                            0123456789AB
  if (hour < 10) {
    sprintf(displayText, " %d%02d  %2d %02d  ", hour, time.getMinute(), month, day);
  } else {
    sprintf(displayText, "%d%02d  %2d %02d  ", hour, time.getMinute(), month, day);
  }
  
  Serial.print(F("Clock display: "));
  Serial.println(displayText);
  
  displayString(displayText);
}

void DisplayManager::displayTimeOnly(DateTime time) {
  char displayText[13];
  
  // Format time: "  HH MM SS  " - center-justified with colors
  // GREEN: HH, AMBER: MM, RED: SS
  int hour = time.getHour();
  if (hour == 0) hour = 12;
  if (hour > 12) hour -= 12;
  
  // Create 12-character string: "  HH MM SS  "
  //                            0123456789AB
  sprintf(displayText, "  %2d %02d %02d  ", hour, time.getMinute(), time.getSecond());
  
  displayString(displayText);
}

void DisplayManager::displayDateOnly(DateTime time) {
  char displayText[13];
  
  // Format date: "  MM DD YYYY" - using all three colors
  // GREEN: MM, AMBER: DD, RED: YYYY
  int month = time.getMonth();
  int day = time.getDay();
  int year = time.getYear();
  
  // Create 12-character string: "  MM DD YYYY"
  //                            0123456789AB
  sprintf(displayText, "  %2d %02d %4d", month, day, year);
  
  displayString(displayText);
}

void DisplayManager::formatTime(DateTime time, char* buffer) {
  int hour = time.getHour();
  if (hour == 0) hour = 12;
  if (hour > 12) hour -= 12;
  
  // Format as 4 digits with leading space for single digit hours
  if (hour < 10) {
    sprintf(buffer, " %d%02d", hour, time.getMinute());
  } else {
    sprintf(buffer, "%d%02d", hour, time.getMinute());
  }
}

void DisplayManager::formatDate(DateTime time, char* buffer) {
  sprintf(buffer, "%02d/%02d", time.getMonth(), time.getDay());
}

void DisplayManager::displayTemperature(SensorData data) {
  char displayText[13];
  char tempStr[5];   // For temperature string like "75.0" 
  char feelsStr[5];  // For feels like string like "78.0"
  
  // Create 12-character string with 4-char segments: "TTTT FFFF WWWW"
  // Characters 0-3 (GREEN): Temperature (e.g. "75.0")  
  // Characters 4-7 (AMBER): Feels like (e.g. "78.0")
  // Characters 8-11 (RED): Temperature word (e.g. "WARM")
  
  // Format temperature using float_to_fixed like in reference code
  if(data.temperatureF < 100.0){
    float_to_fixed(data.temperatureF, tempStr, "%2d.%1d");
  } else {
    float_to_fixed(data.temperatureF, tempStr, "%3d");  // No decimal for 100+
  }
  
  // Format feels like temperature  
  if(data.feelsLikeF < 100.0){
    float_to_fixed(data.feelsLikeF, feelsStr, "%2d.%1d");
  } else {
    float_to_fixed(data.feelsLikeF, feelsStr, "%3d");  // No decimal for 100+
  }
  
  // Combine into 12-character display string
  sprintf(displayText, "%4s %4s%-4s", tempStr, feelsStr, data.tempWord);
  
  displayString(displayText);
}

void DisplayManager::displayWeatherSummary(SensorData data) {
  char displayText[13];
  char tempStr[5];     // For temperature like "79.0"
  char humidStr[5];    // For humidity like "45.0"
  char pressStr[5];    // For pressure like "1013"
  
  // Create 12-character string: "TTTT HHHH PPPP"
  // Characters 0-3 (GREEN): Temperature (e.g. "79.0")
  // Characters 4-7 (AMBER): Humidity (e.g. "45.0")
  // Characters 8-11 (RED): Pressure (e.g. "1013")
  
  // Format temperature 
  if(data.temperatureF < 100.0){
    float_to_fixed(data.temperatureF, tempStr, "%2d.%1d");
  } else {
    float_to_fixed(data.temperatureF, tempStr, "%3d");
  }
  
  // Format humidity (usually no decimals needed for humidity)
  sprintf(humidStr, "%3d%%", (int)data.humidity);
  
  // Format pressure as integer (avoid float formatting issues)
  sprintf(pressStr, "%4d", (int)data.pressure);
  
  sprintf(displayText, "%4s%4s%4s", tempStr, humidStr, pressStr);
  
  displayString(displayText);
}

void DisplayManager::displayRollingCurrent(SensorData data) {
  Serial.println(F("=== ROLLING DISPLAY START ==="));
  unsigned long currentTime = millis();
  
  Serial.print(F("Current time: "));
  Serial.println(currentTime);
  Serial.print(F("Rolling timer: "));
  Serial.println(rollingTimer);
  Serial.print(F("Time diff: "));
  Serial.println(currentTime - rollingTimer);
  
  // Change display every 3 seconds
  if (currentTime - rollingTimer > 3000) {
    rollingTimer = currentTime;
    rollingIndex = (rollingIndex + 1) % 5;
    Serial.print(F("Rolling index changed to: "));
    Serial.println(rollingIndex);
  }
  
  Serial.print(F("Current rolling index: "));
  Serial.println(rollingIndex);
  
  char buffer1[10], buffer2[10], buffer3[10];
  
  switch (rollingIndex) {
    case 0: // Time and actual temperature
      Serial.println(F("Case 0: Time and actual temperature"));
      formatTime(data.currentTime, buffer1);
      if(data.temperatureF < 100.0){
        float_to_fixed(data.temperatureF, buffer2, "%2d.%1d");
      } else {
        float_to_fixed(data.temperatureF, buffer2, "%3d");
      }
      strcpy(buffer3, "TEMP");
      Serial.print(F("Buffer1: ")); Serial.println(buffer1);
      Serial.print(F("Buffer2: ")); Serial.println(buffer2);
      Serial.print(F("Buffer3: ")); Serial.println(buffer3);
      break;
      
    case 1: // Date and feels-like temperature
      Serial.println(F("Case 1: Date and feels-like temperature"));
      formatDate(data.currentTime, buffer1);
      if(data.feelsLikeF < 100.0){
        float_to_fixed(data.feelsLikeF, buffer2, "%2d.%1d");
      } else {
        float_to_fixed(data.feelsLikeF, buffer2, "%3d");
      }
      strcpy(buffer3, "FEEL");
      Serial.print(F("Buffer1: ")); Serial.println(buffer1);
      Serial.print(F("Buffer2: ")); Serial.println(buffer2);
      Serial.print(F("Buffer3: ")); Serial.println(buffer3);
      break;
      
    case 2: // Temperature word and humidity
      Serial.println(F("Case 2: Temperature word and humidity"));
      strcpy(buffer1, data.tempWord);
      
      // DEBUG: Show humidity formatting process
      Serial.print(F("DEBUG: Raw humidity float: "));
      Serial.println(data.humidity);
      Serial.print(F("DEBUG: Cast to int: "));
      Serial.println((int)data.humidity);
      
      sprintf(buffer2, "%3d%%", (int)data.humidity);
      
      Serial.print(F("DEBUG: Formatted humidity string: '"));
      Serial.print(buffer2);
      Serial.print(F("' (length: "));
      Serial.print(strlen(buffer2));
      Serial.println(F(")"));
      
      strcpy(buffer3, "HUM ");
      Serial.print(F("Buffer1: ")); Serial.println(buffer1);
      Serial.print(F("Buffer2: ")); Serial.println(buffer2);
      Serial.print(F("Buffer3: ")); Serial.println(buffer3);
      break;
      
    case 3: // Pressure
      Serial.println(F("Case 3: Pressure"));
      sprintf(buffer1, "%4d", (int)data.pressure);
      strcpy(buffer2, "PRES");
      strcpy(buffer3, "HPA ");
      Serial.print(F("Buffer1: ")); Serial.println(buffer1);
      Serial.print(F("Buffer2: ")); Serial.println(buffer2);
      Serial.print(F("Buffer3: ")); Serial.println(buffer3);
      break;
      
    case 4: // Light level
      Serial.println(F("Case 4: Light level"));
      Serial.print(F("DEBUG: Raw lightLevel float: "));
      Serial.println(data.lightLevel);
      Serial.print(F("DEBUG: lightLevel is finite: "));
      Serial.println(isfinite(data.lightLevel));
      Serial.print(F("DEBUG: lightLevel is NaN: "));
      Serial.println(isnan(data.lightLevel));
      
      // Use float_to_fixed with 0 decimals (now fixed to handle this case)
      float_to_fixed(data.lightLevel, buffer1, "%4d", 0);  // 0 decimals for integer display
      
      Serial.print(F("DEBUG: Formatted lightLevel string: '"));
      Serial.print(buffer1);
      Serial.print(F("' (length: "));
      Serial.print(strlen(buffer1));
      Serial.println(F(")"));
      
      strcpy(buffer2, "LITE");
      strcpy(buffer3, "LUX ");
      Serial.print(F("Buffer1: ")); Serial.println(buffer1);
      Serial.print(F("Buffer2: ")); Serial.println(buffer2);
      Serial.print(F("Buffer3: ")); Serial.println(buffer3);
      break;
  }
  
  // Combine into single 12-character string for unified display

  Serial.println("\r\n\r\n\r\n\r\n\r\n");
  Serial.print("|");
  Serial.print(buffer1);
  Serial.println("|");
  Serial.print("|");
  Serial.print(buffer2);
  Serial.println("|");
  Serial.print("|");
  Serial.print(buffer3);
  Serial.println("|");
  Serial.println("\r\n\r\n\r\n\r\n\r\n");

  char displayText[20];
  sprintf(displayText, "%4s%4s%4s", buffer1, buffer2, buffer3);
  
  Serial.print(F("Final display text: '"));
  Serial.print(displayText);
  Serial.println(F("'"));
  
  displayString(displayText);
  Serial.println(F("=== ROLLING DISPLAY END ==="));
}

void DisplayManager::displayRollingHistorical() {
  // Placeholder - would display historical data using unified 12-character display
  displayString("History");
}

void DisplayManager::displayRollingTrends() {
  // Placeholder - would display trend data using unified 12-character display  
  displayString("Trends");
}

void DisplayManager::displaySettings() {
  displayString("Settings");
}

void DisplayManager::displaySettingsMenu(SettingItem currentSetting) {
  char displayText[13];
  
  switch (currentSetting) {
    case SETTING_TIME:
      sprintf(displayText, "Set TIME    ");
      break;
    case SETTING_DATE:
      sprintf(displayText, "Set DATE    ");
      break;
    case SETTING_CHIME_TYPE:
      sprintf(displayText, "Chime TYPE  ");
      break;
    case SETTING_CHIME_INSTRUMENT:
      sprintf(displayText, "Chime INSTRU.");
      break;
    case SETTING_CHIME_FREQUENCY:
      sprintf(displayText, "Chime FREQUE.");
      break;
    case SETTING_EXIT:
      sprintf(displayText, "EXIT        ");
      break;
    default:
      sprintf(displayText, "Setting %03d ", (int)currentSetting);
      break;
  }
  
  displayString(displayText);
}

void DisplayManager::displaySettingsInterface(SettingItem currentSetting, int settingTimeComponent, 
                                               int settingDateComponent, DateTime pendingDateTime) {
  char displayText[13];
  
  switch (currentSetting) {
    case SETTING_TIME:
      {
        char timeStr[9];  // "HH:MM:SS"
        sprintf(timeStr, "%02d:%02d:%02d", pendingDateTime.getHour(), 
                pendingDateTime.getMinute(), pendingDateTime.getSecond());
        
        // Show which component is being edited
        switch (settingTimeComponent) {
          case 0: // Hour
            sprintf(displayText, "H%s", timeStr);
            break;
          case 1: // Minute  
            sprintf(displayText, "M%s", timeStr);
            break;
          case 2: // Second
            sprintf(displayText, "S%s", timeStr);
            break;
        }
      }
      break;
      
    case SETTING_DATE:
      {
        char dateStr[9];  // "MM/DD/YY"
        sprintf(dateStr, "%02d/%02d/%02d", pendingDateTime.getMonth(), 
                pendingDateTime.getDay(), pendingDateTime.getYear() % 100);
        
        // Show which component is being edited
        switch (settingDateComponent) {
          case 0: // Month
            sprintf(displayText, "MO%s", dateStr);
            break;
          case 1: // Day
            sprintf(displayText, "DY%s", dateStr);
            break;
          case 2: // Year
            sprintf(displayText, "YR%s", dateStr);
            break;
        }
      }
      break;
      
    case SETTING_CHIME_TYPE:
      sprintf(displayText, "CHIME TYPE  ");
      break;
      
    case SETTING_CHIME_INSTRUMENT:
      sprintf(displayText, "CHIME INST  ");
      break;
      
    case SETTING_CHIME_FREQUENCY:
      sprintf(displayText, "CHIME FREQ  ");
      break;
      
    default:
      sprintf(displayText, "SETTING %03d ", (int)currentSetting);
      break;
  }
  
  displayString(displayText);
}

void DisplayManager::formatFloat(float value, char* buffer, uint8_t decimals) {
  if (decimals == 0) {
    sprintf(buffer, "%4.0f", value);
  } else if (decimals == 1) {
    sprintf(buffer, "%3.1f", value);
  } else {
    sprintf(buffer, "%2.2f", value);
  }
}

void DisplayManager::setMode(DisplayMode mode) {
  currentMode = mode;
  rollingIndex = 0;
  rollingTimer = millis();
  clearAllDisplays();
}

DisplayMode DisplayManager::getCurrentMode() {
  return currentMode;
}

void DisplayManager::setBrightness(uint8_t baseBrightness) {
  // Apply color compensation to maintain consistent apparent brightness
  byte compensatedBrightness[3];
  
  // Calculate compensated brightness maintaining the ratios: Green=1, Amber=9, Red=15
  // Base ratios: Green=0.067, Amber=0.6, Red=1.0 (relative to red)
  compensatedBrightness[0] = (baseBrightness * 1 + 7) / 15;  // Green: scale to 1/15 of red
  compensatedBrightness[1] = (baseBrightness * 9 + 7) / 15;  // Amber: scale to 9/15 of red  
  compensatedBrightness[2] = baseBrightness;                 // Red: unchanged
  
  // Ensure minimum brightness of 1
  if (compensatedBrightness[0] < 1) compensatedBrightness[0] = 1;
  if (compensatedBrightness[1] < 1) compensatedBrightness[1] = 1;
  if (compensatedBrightness[2] < 1) compensatedBrightness[2] = 1;
  
  // Re-initialize display group with new brightness
  displayGroup->init(compensatedBrightness);
}

void DisplayManager::adjustBrightnessForAmbientLight(float lightLevel) {
  uint8_t brightness;
  
  if (lightLevel < 10) {
    brightness = 2;  // Very dim
  } else if (lightLevel < 50) {
    brightness = 4;  // Dim
  } else if (lightLevel < 200) {
    brightness = 8;  // Medium
  } else if (lightLevel < 1000) {
    brightness = 12; // Bright
  } else {
    brightness = 15; // Very bright
  }
  
  setBrightness(brightness);
}

void DisplayManager::showStartupMessage() {
  displayString("ChronoSphere");
  delay(1000);
  // displayString("INITIALIZING");
}

void DisplayManager::showError(const char* errorCode) {
  char errorText[13];
  sprintf(errorText, "ERR %-4s FAIL", errorCode);
  displayString(errorText);
}

void DisplayManager::showSetting(SettingItem setting, int value) {
  char settingText[13];
  sprintf(settingText, "SET %4d TING", value);
  displayString(settingText);
}
