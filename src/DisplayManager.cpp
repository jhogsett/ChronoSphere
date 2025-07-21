#include <Arduino.h>
#include "DisplayManager.h"
#include <HT16K33Disp.h>
#include <stdio.h>
#include <string.h>  // For strcpy

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
  int hour = time.hour();
  if (hour == 0) hour = 12;
  if (hour > 12) hour -= 12;
  
  // Format date: month in positions 6-7, day in positions 9-10
  int month = time.month();
  int day = time.day();
  
  // Create 12-character string: "HHMM  MM DD  "
  //                            0123456789AB
  if (hour < 10) {
    sprintf(displayText, " %d%02d  %2d %02d  ", hour, time.minute(), month, day);
  } else {
    sprintf(displayText, "%d%02d  %2d %02d  ", hour, time.minute(), month, day);
  }
  
  displayString(displayText);
}

void DisplayManager::displayTimeOnly(DateTime time) {
  char displayText[13];
  
  // Format time: "  HH MM SS  " - center-justified with colors
  // GREEN: HH, AMBER: MM, RED: SS
  int hour = time.hour();
  if (hour == 0) hour = 12;
  if (hour > 12) hour -= 12;
  
  // Create 12-character string: "  HH MM SS  "
  //                            0123456789AB
  sprintf(displayText, "  %2d %02d %02d  ", hour, time.minute(), time.second());
  
  displayString(displayText);
}

void DisplayManager::displayDateOnly(DateTime time) {
  char displayText[13];
  
  // Format date: "  MM DD YYYY" - using all three colors
  // GREEN: MM, AMBER: DD, RED: YYYY
  int month = time.month();
  int day = time.day();
  int year = time.year();
  
  // Create 12-character string: "  MM DD YYYY"
  //                            0123456789AB
  sprintf(displayText, "  %2d %02d %4d", month, day, year);
  
  displayString(displayText);
}

void DisplayManager::formatTime(DateTime time, char* buffer) {
  int hour = time.hour();
  if (hour == 0) hour = 12;
  if (hour > 12) hour -= 12;
  
  // Format as 4 digits with leading space for single digit hours
  if (hour < 10) {
    sprintf(buffer, " %d%02d", hour, time.minute());
  } else {
    sprintf(buffer, "%d%02d", hour, time.minute());
  }
}

void DisplayManager::formatDate(DateTime time, char* buffer) {
  sprintf(buffer, "%02d/%02d", time.month(), time.day());
}

void DisplayManager::displayTemperature(SensorData data) {
  char tempStr[5];
  char feelsStr[5];
  
  // Raw temperature on green
  sprintf(tempStr, "%3.1f", data.temperatureF);
  displayOnModule(0, tempStr);
  
  // Feels like on amber
  sprintf(feelsStr, "%3d", (int)data.feelsLikeF);
  displayOnModule(1, feelsStr);
  
  // Four-letter word on appropriate color display
  displayOnModule(data.displayColor, data.tempWord);
}

void DisplayManager::displayWeatherSummary(SensorData data) {
  char tempStr[5];
  char humStr[5];
  char pressStr[5];
  
  sprintf(tempStr, "%3d", (int)data.temperatureF);
  sprintf(humStr, "%3d%%", (int)data.humidity);
  sprintf(pressStr, "%4.0f", data.pressure);
  
  displayOnModule(0, tempStr);   // Green
  displayOnModule(1, humStr);    // Amber  
  displayOnModule(2, pressStr);  // Red
}

void DisplayManager::displayRollingCurrent(SensorData data) {
  unsigned long currentTime = millis();
  
  // Change display every 3 seconds
  if (currentTime - rollingTimer > 3000) {
    rollingTimer = currentTime;
    rollingIndex = (rollingIndex + 1) % 4;
  }
  
  char buffer1[5], buffer2[5], buffer3[5];
  
  switch (rollingIndex) {
    case 0: // Time and temperature
      formatTime(data.currentTime, buffer1);
      sprintf(buffer2, "%3d", (int)data.temperatureF);
      strcpy(buffer3, data.tempWord);
      break;
      
    case 1: // Date and humidity
      formatDate(data.currentTime, buffer1);
      sprintf(buffer2, "%3d%%", (int)data.humidity);
      strcpy(buffer3, "HUM ");
      break;
      
    case 2: // Pressure
      sprintf(buffer1, "%4.0f", data.pressure);
      strcpy(buffer2, "PRES");
      strcpy(buffer3, "HPA ");
      break;
      
    case 3: // Light level
      sprintf(buffer1, "%4.0f", data.lightLevel);
      strcpy(buffer2, "LITE");
      strcpy(buffer3, "LUX ");
      break;
  }
  
  displayOnModule(0, buffer1);
  displayOnModule(1, buffer2);
  displayOnModule(2, buffer3);
}

void DisplayManager::displayRollingHistorical() {
  // Placeholder - would display historical data
  displayOnModule(0, "HIST");
  displayOnModule(1, "ORICAL");
  displayOnModule(2, "DATA");
}

void DisplayManager::displayRollingTrends() {
  // Placeholder - would display trend data
  displayOnModule(0, "TREN");
  displayOnModule(1, "DING");
  displayOnModule(2, "DATA");
}

void DisplayManager::displaySettings() {
  displayOnModule(0, "SETT");
  displayOnModule(1, "INGS");
  displayOnModule(2, "MODE");
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
  displayOnModule(0, "WEAT");
  displayOnModule(1, "HER ");
  displayOnModule(2, "CLOK");
  
  delay(1000);
  
  displayOnModule(0, "INIT");
  displayOnModule(1, "IALIZ");
  displayOnModule(2, "ING ");
}

void DisplayManager::showError(const char* errorCode) {
  displayOnModule(0, "ERR ");
  displayOnModule(1, errorCode);
  displayOnModule(2, "FAIL");
}

void DisplayManager::showSetting(SettingItem setting, int value) {
  char valueStr[5];
  sprintf(valueStr, "%4d", value);
  
  displayOnModule(0, "SET ");
  displayOnModule(1, valueStr);
  displayOnModule(2, "TING");
}
