#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Adafruit_LEDBackpack.h>
#include "Config.h"
#include "Sensors.h"

class DisplayManager {
private:
  Adafruit_AlphaNum4 displays[3];  // Green, Amber, Red
  
  unsigned long lastUpdateTime;
  DisplayMode currentMode;
  uint8_t rollingIndex;
  unsigned long rollingTimer;
  
  void clearAllDisplays();
  void displayOnModule(uint8_t module, const char* text);
  void displayTime(DateTime time);
  void displayDate(DateTime time);
  void displayTemperature(SensorData data);
  void displayWeatherSummary(SensorData data);
  void displayRollingCurrent(SensorData data);
  void displayRollingHistorical();
  void displayRollingTrends();
  void displaySettings();
  
  // Utility functions
  void formatTime(DateTime time, char* buffer);
  void formatDate(DateTime time, char* buffer);
  void formatFloat(float value, char* buffer, uint8_t decimals);

public:
  bool init();
  void update(SensorData sensorData);
  void setMode(DisplayMode mode);
  DisplayMode getCurrentMode();
  bool isTimeToUpdate();
  
  // Brightness control
  void setBrightness(uint8_t brightness);
  void adjustBrightnessForAmbientLight(float lightLevel);
  
  // Special displays
  void showStartupMessage();
  void showError(const char* errorCode);
  void showSetting(SettingItem setting, int value);
};

#endif
