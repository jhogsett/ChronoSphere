#include <Arduino.h>
#include "Sensors.h"
#include <math.h>

Sensors::Sensors() : bmp280(&Wire, BMP::eSdoLow) {
  // Constructor initializes BMP280 with I2C (SDA/SCL pins, SDO pulled low)
}

bool Sensors::init() {
  // Initialize DS3231 RTC
  Wire.begin(); // DS3231 requires Wire to be initialized
  
  // The DS3231 library doesn't have a begin() method like RTClib
  // We'll check if we can read from it instead
  bool century = false;
  bool h12Flag;
  bool pm;
  
  // Try to read the year as a test
  uint8_t year = rtc.getYear();
  if (year > 99) { // Invalid year suggests RTC not working
    Serial.println(F("RTC initialization failed"));
    return false;
  }
  
  Serial.println(F("DS3231 RTC initialized successfully"));
  
  // Initialize AHT21 temperature/humidity sensor
  if (!aht.begin()) {
    Serial.println(F("AHT21 initialization failed"));
    return false;
  }
  
  // Initialize BMP280 pressure sensor (using exact working API)
  bmp280.reset();
  Serial.println("BMP280 initialization test");
  
  while(bmp280.begin() != BMP::eStatusOK) {
    Serial.println("BMP280 begin failed");
    switch(bmp280.lastOperateStatus) {
      case BMP::eStatusOK: Serial.println("everything ok"); break;
      case BMP::eStatusErr: Serial.println("unknown error"); break;
      case BMP::eStatusErrDeviceNotDetected: Serial.println("device not detected"); break;
      case BMP::eStatusErrParameter: Serial.println("parameter error"); break;
      default: Serial.println("unknown status"); break;
    }
    Serial.println(F("DFRobot BMP280 IIC initialization failed"));
    return false;
  }
  
  Serial.println("BMP280 begin success");
  
  // Configure sensor exactly like working example
  bmp280.setConfigFilter(BMP::eConfigFilter_off);
  bmp280.setConfigTStandby(BMP::eConfigTStandby_125);
  bmp280.setCtrlMeasSamplingTemp(BMP::eSampling_X8);
  bmp280.setCtrlMeasSamplingPress(BMP::eSampling_X8);
  bmp280.setCtrlMeasMode(BMP::eCtrlMeasModeNormal);
  
  // Initialize BH1750 light sensor
  if (!lightMeter.begin()) {
    Serial.println(F("BH1750 initialization failed"));
    return false;
  }
  
  lastReadTime = 0;
  
  Serial.println(F("All sensors initialized successfully"));
  return true;
}

bool Sensors::readSensors() {
  // Read DS3231 RTC - get current time directly
  bool century = false;
  bool h12Flag;
  bool pm;
  
  // Create DateTime from individual components
  int year = 2000 + rtc.getYear(); // DS3231 returns 2-digit year
  int month = rtc.getMonth(century);
  int day = rtc.getDate();
  int hour = rtc.getHour(h12Flag, pm);
  int minute = rtc.getMinute();
  int second = rtc.getSecond();
  
  currentData.currentTime = DateTime(year, month, day, hour, minute, second);
  
  // Read AHT21
  sensors_event_t humidity, temp;
  if (!aht.getEvent(&humidity, &temp)) {
    Serial.println(F("Failed to read AHT21"));
    return false;
  }
  
  currentData.temperature = temp.temperature;
  currentData.humidity = humidity.relative_humidity;
  
  // Read BMP280
  currentData.pressure = bmp280.getPressure() / 100.0; // Convert Pa to hPa
  
  // Read BH1750
  currentData.lightLevel = lightMeter.readLightLevel();
  
  // Calculate derived values
  currentData.temperatureF = celsiusToFahrenheit(currentData.temperature);
  currentData.feelsLikeF = calculateFeelsLike(currentData.temperatureF, currentData.humidity);
  
  calculateTempWord(currentData.feelsLikeF);
  currentData.displayColor = getDisplayColor(currentData.feelsLikeF);
  
  lastReadTime = millis();
  return true;
}

bool Sensors::isTimeToRead() {
  return (millis() - lastReadTime >= SENSOR_READ_INTERVAL);
}

SensorData Sensors::getCurrentData() {
  return currentData;
}

float Sensors::celsiusToFahrenheit(float celsius) {
  return celsius * 9.0 / 5.0 + 32.0;
}

float Sensors::calculateFeelsLike(float tempF, float humidity) {
  // NOAA Heat Index calculation for temperatures >= 80°F
  if (tempF >= 80.0) {
    float hi = -42.379 + 2.04901523 * tempF + 10.14333127 * humidity
               - 0.22475541 * tempF * humidity - 6.83783e-3 * tempF * tempF
               - 5.481717e-2 * humidity * humidity + 1.22874e-3 * tempF * tempF * humidity
               + 8.5282e-4 * tempF * humidity * humidity - 1.99e-6 * tempF * tempF * humidity * humidity;
    
    // Adjustments for low humidity
    if (humidity < 13 && tempF >= 80 && tempF <= 112) {
      hi -= ((13 - humidity) / 4) * sqrt((17 - abs(tempF - 95)) / 17);
    }
    
    // Adjustments for high humidity
    if (humidity > 85 && tempF >= 80 && tempF <= 87) {
      hi += ((humidity - 85) / 10) * ((87 - tempF) / 5);
    }
    
    return hi;
  }
  
  // For temperatures < 80°F, use wind chill approximation
  // Since we don't have wind sensor, assume light air (2 mph)
  if (tempF <= 50.0) {
    float windSpeed = 2.0; // mph
    return 35.74 + 0.6215 * tempF - 35.75 * pow(windSpeed, 0.16) + 0.4275 * tempF * pow(windSpeed, 0.16);
  }
  
  // For temperatures between 50-80°F, return actual temperature
  return tempF;
}

void Sensors::calculateTempWord(float feelsLikeF) {
  if (feelsLikeF <= TEMP_FROZ_MAX) {
    strcpy(currentData.tempWord, "FROZ");
  } else if (feelsLikeF <= TEMP_COLD_MAX) {
    strcpy(currentData.tempWord, "COLD");
  } else if (feelsLikeF <= TEMP_CHLY_MAX) {
    strcpy(currentData.tempWord, "CHLY");
  } else if (feelsLikeF <= TEMP_COOL_MAX) {
    strcpy(currentData.tempWord, "COOL");
  } else if (feelsLikeF <= TEMP_NICE_MAX) {
    strcpy(currentData.tempWord, "NICE");
  } else if (feelsLikeF <= TEMP_WARM_MAX) {
    strcpy(currentData.tempWord, "WARM");
  } else if (feelsLikeF <= TEMP_COZY_MAX) {
    strcpy(currentData.tempWord, "COZY");
  } else if (feelsLikeF <= TEMP_TOSY_MAX) {
    strcpy(currentData.tempWord, "TOSY");
  } else if (feelsLikeF <= TEMP_HOT_MAX) {
    strcpy(currentData.tempWord, "HOT ");
  } else {
    strcpy(currentData.tempWord, "SCOR");
  }
}

uint8_t Sensors::getDisplayColor(float feelsLikeF) {
  // Green for comfortable temperatures (NICE range)
  if (feelsLikeF >= COMFORT_GREEN_MIN && feelsLikeF <= COMFORT_GREEN_MAX) {
    return 0; // Green
  }
  
  // Red for very uncomfortable temperatures (COLD and HOT+ ranges)
  if (feelsLikeF <= COMFORT_RED_MAX || feelsLikeF >= TEMP_HOT_MAX) {
    return 2; // Red
  }
  
  // Amber for everything else
  return 1; // Amber
}

bool Sensors::setDateTime(DateTime newDateTime) {
  // Convert 4-digit year to 2-digit for DS3231
  int year2digit = newDateTime.getYear() - 2000;
  
  rtc.setYear(year2digit);
  rtc.setMonth(newDateTime.getMonth());
  rtc.setDate(newDateTime.getDay());
  rtc.setHour(newDateTime.getHour());
  rtc.setMinute(newDateTime.getMinute());
  rtc.setSecond(newDateTime.getSecond());
  return true;
}

DateTime Sensors::getCurrentTime() {
  return currentData.currentTime;
}

const char* Sensors::getTempWord() {
  return currentData.tempWord;
}

uint8_t Sensors::getTempDisplayColor() {
  return currentData.displayColor;
}
