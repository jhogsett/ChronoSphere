#ifndef LIGHTING_EFFECTS_H
#define LIGHTING_EFFECTS_H

#include <Adafruit_NeoPixel.h>
#include "Config.h"
#include "Sensors.h"

enum LightingMode {
  LIGHTING_OFF = 0,
  LIGHTING_SOLID_COLOR,
  LIGHTING_TEMPERATURE_GRADIENT,
  LIGHTING_WEATHER_INDICATOR,
  LIGHTING_RAINBOW,
  LIGHTING_BREATHING,
  LIGHTING_CLOCK_INDICATOR
};

struct Color {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

class LightingEffects {
private:
  Adafruit_NeoPixel strip;
  
  LightingMode currentMode;
  unsigned long lastUpdate;
  uint8_t brightness;
  uint16_t animationStep;
  
  // Color palettes
  static const Color temperatureColors[];
  static const Color weatherColors[];
  
  void updateSolidColor(Color color);
  void updateTemperatureGradient(float temperature);
  void updateWeatherIndicator(SensorData data);
  void updateRainbow();
  void updateBreathing();
  void updateClockIndicator(DateTime time);
  
  Color interpolateColor(Color color1, Color color2, float factor);
  Color temperatureToColor(float tempF);
  Color pressureToColor(float pressure);
  uint32_t colorToUint32(Color color);

public:
  bool init();
  void update(SensorData sensorData);
  
  // Mode control
  void setMode(LightingMode mode);
  LightingMode getMode();
  
  // Brightness control
  void setBrightness(uint8_t brightness);
  void adjustBrightnessForAmbientLight(float lightLevel);
  
  // Manual color control
  void setSolidColor(uint8_t red, uint8_t green, uint8_t blue);
  void setPixelColor(uint8_t pixel, uint8_t red, uint8_t green, uint8_t blue);
  
  // Special effects
  void flashAlert(Color color, uint8_t flashes);
  void showStartupSequence();
  void showErrorPattern();
  
  // Utility
  void clear();
  void show();
  uint8_t getBrightness();
};

#endif
