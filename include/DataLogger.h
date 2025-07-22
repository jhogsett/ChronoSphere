#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <EEPROM.h>
#include "Sensors.h"  // This now includes our DateTime struct
#include "Config.h"

struct HourlyRecord {
  DateTime timestamp;
  float avgTemperature;
  float avgHumidity;
  float avgPressure;
  float minTemperature;
  float maxTemperature;
  float minPressure;
  float maxPressure;
};

struct DailyRecord {
  DateTime date;
  float avgTemperature;
  float avgHumidity;
  float avgPressure;
  float minTemperature;
  float maxTemperature;
  float minPressure;
  float maxPressure;
};

struct TrendData {
  float temperatureTrend;    // °F per hour
  float pressureTrend;       // hPa per hour
  float humidityTrend;       // % per hour
  bool risingPressure;
  bool fallingPressure;
  bool rapidTempChange;
};

class DataLogger {
private:
  HourlyRecord hourlyData[24];     // Reduced to 24 hours instead of 168
  DailyRecord dailyData[7];        // Reduced to 7 days instead of 30
  
  uint8_t currentHourlyIndex;
  uint8_t currentDailyIndex;
  
  SensorData currentHourSamples[12];  // Reduced to 12 samples per hour instead of 60
  uint8_t currentSampleIndex;
  
  unsigned long lastLogTime;
  DateTime lastHourLogged;
  DateTime lastDayLogged;
  
  void logHourlyData();
  void logDailyData();
  void shiftArray();
  void saveToEEPROM();
  void loadFromEEPROM();

public:
  bool init();
  void update(SensorData currentData);
  
  // Data retrieval
  HourlyRecord getHourlyRecord(uint8_t hoursAgo);
  DailyRecord getDailyRecord(uint8_t daysAgo);
  
  // Statistics
  float getAverageTemperature(uint8_t hours);
  float getAveragePressure(uint8_t hours);
  float getAverageHumidity(uint8_t hours);
  
  float getMinTemperature(uint8_t hours);
  float getMaxTemperature(uint8_t hours);
  float getMinPressure(uint8_t hours);
  float getMaxPressure(uint8_t hours);
  
  // Trend analysis
  TrendData calculateTrends();
  bool detectWeatherChange();
  float predictTemperature(uint8_t hoursAhead);
  
  // Alerts
  bool checkPressureAlert();
  bool checkTemperatureAlert();
  bool checkRapidChange();
  
  // Data management
  void clearAllData();
  bool isDataValid();
  uint8_t getDataAge();  // Hours since oldest data
};

#endif
