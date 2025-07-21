#include <Arduino.h>
#include "LightingEffects.h"
#include <math.h>  // For sin() function

// Temperature color palette (cold to hot)
const Color LightingEffects::temperatureColors[] = {
  {0, 0, 255},     // Blue (cold)
  {0, 128, 255},   // Light blue
  {0, 255, 255},   // Cyan
  {0, 255, 128},   // Green-cyan
  {0, 255, 0},     // Green (comfortable)
  {128, 255, 0},   // Yellow-green
  {255, 255, 0},   // Yellow
  {255, 128, 0},   // Orange
  {255, 0, 0},     // Red (hot)
  {255, 0, 128}    // Pink (very hot)
};

// Weather condition colors
const Color LightingEffects::weatherColors[] = {
  {0, 255, 0},     // Green (good weather)
  {255, 255, 0},   // Yellow (changing)
  {255, 128, 0},   // Orange (warning)
  {255, 0, 0},     // Red (severe)
  {128, 0, 255}    // Purple (storm)
};

bool LightingEffects::init() {
  strip = Adafruit_NeoPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
  strip.begin();
  strip.clear();
  strip.show();
  
  currentMode = LIGHTING_TEMPERATURE_GRADIENT;
  lastUpdate = 0;
  brightness = 128; // Medium brightness
  animationStep = 0;
  
  Serial.println(F("Lighting effects initialized"));
  return true;
}

void LightingEffects::update(SensorData sensorData) {
  unsigned long currentTime = millis();
  
  // Update at ~30 FPS
  if (currentTime - lastUpdate < 33) {
    return;
  }
  
  switch (currentMode) {
    case LIGHTING_OFF:
      clear();
      break;
      
    case LIGHTING_SOLID_COLOR:
      // Solid color mode - color set manually
      break;
      
    case LIGHTING_TEMPERATURE_GRADIENT:
      updateTemperatureGradient(sensorData.temperatureF);
      break;
      
    case LIGHTING_WEATHER_INDICATOR:
      updateWeatherIndicator(sensorData);
      break;
      
    case LIGHTING_RAINBOW:
      updateRainbow();
      break;
      
    case LIGHTING_BREATHING:
      updateBreathing();
      break;
      
    case LIGHTING_CLOCK_INDICATOR:
      updateClockIndicator(sensorData.currentTime);
      break;
  }
  
  show();
  lastUpdate = currentTime;
  animationStep++;
}

void LightingEffects::updateTemperatureGradient(float temperature) {
  Color tempColor = temperatureToColor(temperature);
  
  for (int i = 0; i < strip.numPixels(); i++) {
    // Create gradient effect with slight variation per pixel
    float factor = (float)i / strip.numPixels();
    Color pixelColor = interpolateColor(tempColor, {tempColor.red/2, tempColor.green/2, tempColor.blue/2}, factor * 0.3);
    
    strip.setPixelColor(i, colorToUint32(pixelColor));
  }
}

void LightingEffects::updateWeatherIndicator(SensorData data) {
  // Determine weather condition based on pressure trend and other factors
  Color weatherColor;
  
  if (data.pressure > 1020) {
    weatherColor = weatherColors[0]; // Good weather (high pressure)
  } else if (data.pressure > 1010) {
    weatherColor = weatherColors[1]; // Fair weather
  } else if (data.pressure > 1000) {
    weatherColor = weatherColors[2]; // Changing weather
  } else if (data.pressure > 990) {
    weatherColor = weatherColors[3]; // Poor weather
  } else {
    weatherColor = weatherColors[4]; // Storm conditions
  }
  
  // Set all pixels to weather color with breathing effect
  float breatheFactor = (sin(animationStep * 0.1) + 1) / 2; // 0 to 1
  Color breatheColor = interpolateColor({0, 0, 0}, weatherColor, breatheFactor);
  
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, colorToUint32(breatheColor));
  }
}

void LightingEffects::updateRainbow() {
  for (int i = 0; i < strip.numPixels(); i++) {
    // Calculate hue for rainbow effect
    int hue = (animationStep * 2 + i * 256 / strip.numPixels()) % 65536;
    
    // Convert HSV to RGB (simplified)
    uint8_t sector = hue >> 8;
    uint8_t offset = hue & 0xFF;
    
    Color color;
    switch (sector >> 5) {
      case 0: color = {255, offset, 0}; break;
      case 1: color = {255 - offset, 255, 0}; break;
      case 2: color = {0, 255, offset}; break;
      case 3: color = {0, 255 - offset, 255}; break;
      case 4: color = {offset, 0, 255}; break;
      default: color = {255, 0, 255 - offset}; break;
    }
    
    strip.setPixelColor(i, colorToUint32(color));
  }
}

void LightingEffects::updateBreathing() {
  float breatheFactor = (sin(animationStep * 0.05) + 1) / 2; // Slower breathing
  Color breatheColor = interpolateColor({0, 0, 0}, {255, 255, 255}, breatheFactor);
  
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, colorToUint32(breatheColor));
  }
}

void LightingEffects::updateClockIndicator(DateTime time) {
  clear();
  
  // Light up pixels to indicate time
  int hourPixel = (time.hour() % 12) * strip.numPixels() / 12;
  int minutePixel = time.minute() * strip.numPixels() / 60;
  
  // Hour indicator (red)
  strip.setPixelColor(hourPixel, strip.Color(255, 0, 0));
  
  // Minute indicator (blue)  
  if (minutePixel != hourPixel) {
    strip.setPixelColor(minutePixel, strip.Color(0, 0, 255));
  } else {
    // Same position - mix colors
    strip.setPixelColor(hourPixel, strip.Color(255, 0, 255));
  }
}

Color LightingEffects::temperatureToColor(float tempF) {
  // Map temperature to color palette index
  float tempRange = 100.0; // 0-100°F range
  float normalizedTemp = constrain((tempF - 0) / tempRange, 0, 1);
  
  int paletteSize = sizeof(temperatureColors) / sizeof(temperatureColors[0]);
  float paletteIndex = normalizedTemp * (paletteSize - 1);
  
  int lowerIndex = (int)paletteIndex;
  int upperIndex = min(lowerIndex + 1, paletteSize - 1);
  float fraction = paletteIndex - lowerIndex;
  
  return interpolateColor(temperatureColors[lowerIndex], temperatureColors[upperIndex], fraction);
}

Color LightingEffects::pressureToColor(float pressure) {
  // Map pressure to color (blue=low, green=normal, red=high)
  if (pressure < 1000) {
    return {0, 0, 255}; // Blue (low pressure)
  } else if (pressure < 1013) {
    return {0, 255, 255}; // Cyan
  } else if (pressure < 1020) {
    return {0, 255, 0}; // Green (normal)
  } else if (pressure < 1030) {
    return {255, 255, 0}; // Yellow
  } else {
    return {255, 0, 0}; // Red (high pressure)
  }
}

Color LightingEffects::interpolateColor(Color color1, Color color2, float factor) {
  Color result;
  result.red = color1.red + (color2.red - color1.red) * factor;
  result.green = color1.green + (color2.green - color1.green) * factor;
  result.blue = color1.blue + (color2.blue - color1.blue) * factor;
  return result;
}

uint32_t LightingEffects::colorToUint32(Color color) {
  // Apply brightness scaling
  uint8_t r = (color.red * brightness) >> 8;
  uint8_t g = (color.green * brightness) >> 8;
  uint8_t b = (color.blue * brightness) >> 8;
  
  return strip.Color(r, g, b);
}

void LightingEffects::setMode(LightingMode mode) {
  currentMode = mode;
  animationStep = 0;
  
  if (mode == LIGHTING_OFF) {
    clear();
    show();
  }
}

LightingMode LightingEffects::getMode() {
  return currentMode;
}

void LightingEffects::setBrightness(uint8_t brightness) {
  this->brightness = brightness;
}

void LightingEffects::adjustBrightnessForAmbientLight(float lightLevel) {
  if (lightLevel < 1) {
    setBrightness(32);  // Very dim for darkness
  } else if (lightLevel < 10) {
    setBrightness(64);  // Dim for low light
  } else if (lightLevel < 100) {
    setBrightness(128); // Medium for normal light
  } else if (lightLevel < 1000) {
    setBrightness(192); // Bright for bright conditions
  } else {
    setBrightness(255); // Full brightness for very bright conditions
  }
}

void LightingEffects::setSolidColor(uint8_t red, uint8_t green, uint8_t blue) {
  Color color = {red, green, blue};
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, colorToUint32(color));
  }
  show();
}

void LightingEffects::setPixelColor(uint8_t pixel, uint8_t red, uint8_t green, uint8_t blue) {
  if (pixel < strip.numPixels()) {
    Color color = {red, green, blue};
    strip.setPixelColor(pixel, colorToUint32(color));
  }
}

void LightingEffects::flashAlert(Color color, uint8_t flashes) {
  for (uint8_t i = 0; i < flashes; i++) {
    // Flash on
    for (int j = 0; j < strip.numPixels(); j++) {
      strip.setPixelColor(j, colorToUint32(color));
    }
    show();
    delay(200);
    
    // Flash off
    clear();
    show();
    delay(200);
  }
}

void LightingEffects::showStartupSequence() {
  // Rainbow wipe effect
  for (int i = 0; i < strip.numPixels(); i++) {
    Color color = temperatureColors[i % (sizeof(temperatureColors) / sizeof(temperatureColors[0]))];
    strip.setPixelColor(i, colorToUint32(color));
    show();
    delay(100);
  }
  
  delay(500);
  clear();
  show();
}

void LightingEffects::showErrorPattern() {
  // Red flashing pattern
  for (int i = 0; i < 5; i++) {
    setSolidColor(255, 0, 0);
    delay(500);
    clear();
    show();
    delay(500);
  }
}

void LightingEffects::clear() {
  strip.clear();
}

void LightingEffects::show() {
  strip.show();
}

uint8_t LightingEffects::getBrightness() {
  return brightness;
}
