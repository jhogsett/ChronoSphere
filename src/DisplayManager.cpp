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

// Returns 0=Sunday, 1=Monday, ..., 6=Saturday using Sakamoto's algorithm
static uint8_t calcDayOfWeek(int year, int month, int day) {
  static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  if (month < 3) year--;
  return (year + year/4 - year/100 + year/400 + t[month-1] + day) % 7;
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
  
  // Initialize alert display state
  displayingAlert = false;
  currentAlertType = ALERT_NONE;
  alertDisplayStart = 0;
  
  // Serial.println(F("Display manager initialized"));
  return true;
}

void DisplayManager::update(SensorData sensorData) {
  // Check if alert display has timed out (show alert for 3 seconds)
  if (displayingAlert && (millis() - alertDisplayStart > 3000)) {
    clearAlert();
  }
  
  // If displaying alert, skip normal display updates
  if (displayingAlert) {
    return;
  }
  
  switch (currentMode) {
    case MODE_CLOCK:
      displayPanelTimeDate(sensorData);
      break;
      
    case MODE_TEMPERATURE:
      displayPanelTempHumidity(sensorData);
      break;
      
    case MODE_WEATHER_SUMMARY:
      displayPanelPressure(sensorData);
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

void DisplayManager::forceNextUpdate() {
  lastUpdateTime = 0;
}

void DisplayManager::clearAllDisplays() {
  displayGroup->clear();
}

void DisplayManager::displayString(const char* text) {
  displayGroup->show_string(text);
}

void DisplayManager::displayScrollingString(const char* text, int showDelay, int scrollDelay) {
  displayGroup->scroll_string(text, showDelay, scrollDelay);
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

void DisplayManager::displayPanelTimeDate(const SensorData& data) {
  static const char* dayNames[] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
  char displayText[13];
  int hour = data.currentTime.getHour();
  if (hour == 0) hour = 12;
  if (hour > 12) hour -= 12;
  int month = data.currentTime.getMonth();
  int day   = data.currentTime.getDay();
  int dow   = calcDayOfWeek(data.currentTime.getYear(), month, day);
  char timeStr[5];
  if (hour < 10)
    sprintf(timeStr, " %d%02d", hour, data.currentTime.getMinute());
  else
    sprintf(timeStr, "%d%02d", hour, data.currentTime.getMinute());
  sprintf(displayText, "%s%02d.%02d %s", timeStr, month, day, dayNames[dow]);
  displayString(displayText);
}

void DisplayManager::displayPanelFeelsLike(const SensorData& data) {
  char displayText[13];
  const char* word = data.tempWord;
  int fi = (int)(data.feelsLikeF * 10.0f + 0.5f);
  int fw = fi / 10;
  int fd = fi % 10;
  bool wordInGreen = (strcmp(word, "NICE") == 0 || strcmp(word, "WARM") == 0);
  bool wordInAmber = (strcmp(word, "COOL") == 0 || strcmp(word, "COZY") == 0);
  if (wordInGreen) {
    if (fw < 100)
      sprintf(displayText, "%-4s     %2d.%1d", word, fw, fd);
    else
      sprintf(displayText, "%-4s    %4d", word, fw);
  } else if (wordInAmber) {
    if (fw < 100)
      sprintf(displayText, " %2d.%1d%-4s    ", fw, fd, word);
    else
      sprintf(displayText, "%4d%-4s    ", fw, word);
  } else {
    if (fw < 100)
      sprintf(displayText, " %2d.%1d    %-4s", fw, fd, word);
    else
      sprintf(displayText, "%4d    %-4s", fw, word);
  }
  displayString(displayText);
}

void DisplayManager::displayPanelTempHumidity(const SensorData& data) {
  char displayText[13];
  int ti = (int)(data.temperatureF * 10.0f + 0.5f);
  int tw = ti / 10;
  int td = ti % 10;
  float tempC = (data.temperatureF - 32.0f) * 5.0f / 9.0f;
  bool cNeg = (tempC < 0);
  int ci = (int)((cNeg ? -tempC : tempC) * 10.0f + 0.5f);
  int cw = ci / 10;
  int cd = ci % 10;
  char celStr[6];
  sprintf(celStr, cNeg ? "-%2d.%d" : " %2d.%d", cw, cd);
  if (tw < 100)
    sprintf(displayText, " %2d.%1d%s%3d%%", tw, td, celStr, (int)(data.humidity + 0.5f));
  else
    sprintf(displayText, "%4d%s%3d%%", tw, celStr, (int)(data.humidity + 0.5f));
  displayString(displayText);
}

void DisplayManager::displayPanelPressure(const SensorData& data) {
  char displayText[13];
  int mb = (int)(data.pressure + 0.5f);
  int inHg_cents = (int)(data.pressure * 2.953f + 0.5f);
  sprintf(displayText, "Pres%4d%2d.%02d", mb, inHg_cents / 100, inHg_cents % 100);
  displayString(displayText);
}

void DisplayManager::displayPanelLight(const SensorData& data) {
  char displayText[13];
  sprintf(displayText, "Lux %8d", (int)(data.lightLevel + 0.5f));
  displayString(displayText);
}

void DisplayManager::displayRollingCurrent(SensorData data) {
  unsigned long currentTime = millis();
  if (currentTime - rollingTimer > 3000) {
    rollingTimer = currentTime;
    rollingIndex = (rollingIndex + 1) % 5;
  }
  switch (rollingIndex) {
    case 0: displayPanelTimeDate(data);     break;
    case 1: displayPanelFeelsLike(data);    break;
    case 2: displayPanelTempHumidity(data); break;
    case 3: displayPanelPressure(data);     break;
    case 4: displayPanelLight(data);        break;
  }
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
      sprintf(displayText, "Chime INSTR ");
      break;
    case SETTING_CHIME_FREQUENCY:
      sprintf(displayText, "Chime FREQ  ");
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

void DisplayManager::showInitFailure(const char* causes) {
  // Format: "F " (2 chars) + up to 10 chars of cause info = 12 chars total
  char text[13];
  snprintf(text, sizeof(text), "F %-10s", causes);
  displayString(text);
}

void DisplayManager::showSetting(SettingItem setting, int value) {
  char settingText[13];
  sprintf(settingText, "SET %4d TING", value);
  displayString(settingText);
}

void DisplayManager::showAlert(AlertType alertType) {
  char alertText[13];
  
  switch (alertType) {
    case ALERT_PRESSURE:
      sprintf(alertText, "PRESS ALERT ");
      break;
    case ALERT_TEMPERATURE:
      sprintf(alertText, "TEMP ALERT  ");
      break;
    case ALERT_RAPID_CHANGE:
      sprintf(alertText, "WTHR ALERT  ");
      break;
    default:
      sprintf(alertText, "ALERT       ");
      break;
  }
  
  displayString(alertText);
  displayingAlert = true;
  currentAlertType = alertType;
  alertDisplayStart = millis();
}

void DisplayManager::clearAlert() {
  displayingAlert = false;
  currentAlertType = ALERT_NONE;
  alertDisplayStart = 0;
}

bool DisplayManager::isDisplayingAlert() {
  return displayingAlert;
}
