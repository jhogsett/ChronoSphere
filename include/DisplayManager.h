#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "HT16K33Disp.h"
#include "Config.h"
#include "Sensors.h"

class DisplayManager {
private:
  HT16K33Disp* displayGroup;  // Single object managing all 3 displays
  
  unsigned long lastUpdateTime;
  DisplayMode currentMode;
  uint8_t rollingIndex;
  unsigned long rollingTimer;
  
  // Alert display state
  bool displayingAlert;
  AlertType currentAlertType;
  unsigned long alertDisplayStart;
  
  void clearAllDisplays();
  void displayString(const char* text);
  void displayScrollingString(const char* text, int showDelay = 100, int scrollDelay = 100);
  void displayTime(DateTime time);
  void displayDate(DateTime time);
  void displayTemperature(SensorData data);
  void displayWeatherSummary(SensorData data);
  void displayRollingCurrent(SensorData data);
  void displayRollingHistorical();
  void displayRollingTrends();
  void displaySettings();
  void displaySettingsMenu(SettingItem currentSetting);
  void displaySettingsInterface(SettingItem currentSetting, int settingTimeComponent, 
                                int settingDateComponent, DateTime pendingDateTime);
  
  // Utility functions
  void formatTime(DateTime time, char* buffer);
  void formatDate(DateTime time, char* buffer);
  void formatFloat(float value, char* buffer, uint8_t decimals);

public:
  bool init();
  void update(SensorData sensorData);
  void updateSettings(SensorData sensorData, bool settingsMode, SettingItem currentSetting, 
                      int settingTimeComponent, int settingDateComponent, DateTime pendingDateTime, 
                      bool editingSettingValue);
  void setMode(DisplayMode mode);
  DisplayMode getCurrentMode();
  bool isTimeToUpdate();
  
  // Display methods
  void displayTimeOnly(DateTime time);
  void displayDateOnly(DateTime time);
  
  // Brightness control
  void setBrightness(uint8_t brightness);
  void adjustBrightnessForAmbientLight(float lightLevel);
  
  // Special displays
  void showStartupMessage();
  void showError(const char* errorCode);
  void showSetting(SettingItem setting, int value);
  
  // Alert displays
  void showAlert(AlertType alertType);
  void clearAlert();
  bool isDisplayingAlert();
};

#endif
