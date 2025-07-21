#include <Arduino.h>
#include "DisplayManager.h"
#include <stdio.h>
#include <string.h>  // For strcpy

bool DisplayManager::init() {
  // Initialize all three display modules
  for (int i = 0; i < 3; i++) {
    uint8_t address = DISPLAY_GREEN_ADDRESS + i;
    if (!displays[i].begin(address)) {
      Serial.print(F("Display "));
      Serial.print(i);
      Serial.println(F(" initialization failed"));
      return false;
    }
    displays[i].setBrightness(8); // Medium brightness
    displays[i].clear();
    displays[i].writeDisplay();
  }
  
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
  for (int i = 0; i < 3; i++) {
    displays[i].clear();
  }
}

void DisplayManager::displayOnModule(uint8_t module, const char* text) {
  if (module > 2) return;
  
  displays[module].clear();
  for (int i = 0; i < 4 && text[i] != '\0'; i++) {
    displays[module].writeDigitAscii(i, text[i]);
  }
  displays[module].writeDisplay();
}

void DisplayManager::displayTime(DateTime time) {
  char timeStr[5];
  char dateStr[5];
  
  formatTime(time, timeStr);
  formatDate(time, dateStr);
  
  // Display time on green, date on amber
  displayOnModule(0, timeStr);  // Green
  displayOnModule(1, dateStr);  // Amber
  displayOnModule(2, "    ");   // Red - blank
}

void DisplayManager::displayDate(DateTime time) {
  char buffer[5];
  sprintf(buffer, "%02d%02d", time.month(), time.day());
  displayOnModule(1, buffer);
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

void DisplayManager::formatTime(DateTime time, char* buffer) {
  int hour = time.hour();
  if (hour == 0) hour = 12;
  if (hour > 12) hour -= 12;
  
  sprintf(buffer, "%2d%02d", hour, time.minute());
}

void DisplayManager::formatDate(DateTime time, char* buffer) {
  sprintf(buffer, "%02d%02d", time.month(), time.day());
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

void DisplayManager::setBrightness(uint8_t brightness) {
  for (int i = 0; i < 3; i++) {
    displays[i].setBrightness(brightness);
  }
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
