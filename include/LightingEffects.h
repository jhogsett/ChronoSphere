#ifndef LIGHTING_EFFECTS_H
#define LIGHTING_EFFECTS_H

#include "Config.h"
#include "Sensors.h"

// ============================================================================
// LIGHTING EFFECTS - DEPRECATED
// ============================================================================
// NeoPixel LED control has been removed:
// - Future analog clock display will handle LED control
// - Preserving class structure for compatibility
//
// This class is kept as a stub that does nothing.
// ============================================================================

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
public:
  bool init() { return true; }  // Stub - always succeeds
  void update(SensorData sensorData) {}  // Stub
  
  // Mode control - all stubs
  void setMode(LightingMode mode) {}
  LightingMode getMode() { return LIGHTING_OFF; }
  
  // Brightness control - all stubs
  void setBrightness(uint8_t brightness) {}
  void adjustBrightnessForAmbientLight(float lightLevel) {}
  
  // Manual color control - all stubs
  void setSolidColor(uint8_t red, uint8_t green, uint8_t blue) {}
  void setPixelColor(uint8_t pixel, uint8_t red, uint8_t green, uint8_t blue) {}
  
  // Special effects - all stubs
  void flashAlert(Color color, uint8_t flashes) {}
  void startNonBlockingAlert(AlertType type, Color color, uint8_t flashes) {}
  bool isAlertActive() { return false; }
  AlertType getCurrentAlert() { return ALERT_NONE; }
  void showStartupSequence() {}
  void showErrorPattern() {}
  
  // Utility - all stubs
  void clear() {}
  void show() {}
  uint8_t getBrightness() { return 0; }
};

#endif
