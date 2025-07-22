#ifndef SENSORS_H
#define SENSORS_H

#include <DS3231-RTC.h>
#include <Adafruit_AHTX0.h>
#include <DFRobot_BMP280.h>
#include <BH1750.h>
#include "Config.h"

struct SensorData {
  DateTime currentTime;
  float temperature;      // Celsius
  float humidity;         // %
  float pressure;         // hPa
  float lightLevel;       // lux
  float temperatureF;     // Fahrenheit
  float feelsLikeF;       // Feels like temperature in Fahrenheit
  char tempWord[5];       // Four-letter temperature word
  uint8_t displayColor;   // 0=Green, 1=Amber, 2=Red
};

class Sensors {
private:
  DS3231 rtc;
  Adafruit_AHTX0 aht;
  DFRobot_BMP280_IIC bmp280;  // DFRobot BMP280 I2C library
  BH1750 lightMeter;
  
  unsigned long lastReadTime;
  SensorData currentData;
  
  float calculateFeelsLike(float tempF, float humidity);
  void calculateTempWord(float feelsLikeF);
  uint8_t getDisplayColor(float feelsLikeF);

public:
  Sensors();
  bool init();
  bool readSensors();
  SensorData getCurrentData();
  bool isTimeToRead();
  
  // Time management
  bool setDateTime(DateTime newDateTime);
  DateTime getCurrentTime();
  
  // Temperature calculations
  float celsiusToFahrenheit(float celsius);
  const char* getTempWord();
  uint8_t getTempDisplayColor();
};

#endif
